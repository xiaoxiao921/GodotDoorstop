#include "entrypoint.h"
#include "../bootstrap.h"
#include "../config/config.h"
#include "../crt.h"
#include "../runtimes/hostfxr.h"
#include "../util/logging.h"
#include "../util/paths.h"
#include "hook.h"
#include "proxy/proxy.h"

/**
 * @brief Ensures current working directory is the game folder
 *
 * In some cases (e.g. custom launchers), the CWD (current working directory)
 * is not the same as Unity default. This can break invokable DLLs and even
 * Unity itself. This fix ensures current working directory is the same as
 * application directory and fixes it if not.
 *
 * @return bool_t Whether CWD was changed to match program directory.
 */
bool_t fix_cwd() {
    char_t *app_path = program_path();
    char_t *app_dir = get_folder_name(app_path);
    bool_t fixed_cwd = FALSE;
    char_t *working_dir = get_working_dir();

    if (strcmpi(app_dir, working_dir) != 0) {
        fixed_cwd = TRUE;
        SetCurrentDirectory(app_dir);
    }

    free(app_path);
    free(app_dir);
    free(working_dir);
    return fixed_cwd;
}

#define LOG_FILE_CMD_START L" -logFile \""
#define LOG_FILE_CMD_START_LEN STR_LEN(LOG_FILE_CMD_START)
#define LOG_FILE_CMD_EXTRA 1024

#define LOG_FILE_CMD_END L"\\output_log.txt\""
#define LOG_FILE_CMD_END_LEN STR_LEN(LOG_FILE_CMD_END)

char_t *default_boot_config_path = NULL;

char_t *new_cmdline_args = NULL;
char *new_cmdline_args_narrow = NULL;

LPWSTR WINAPI get_command_line_hook() {
    if (new_cmdline_args)
        return new_cmdline_args;
    return GetCommandLineW();
}

LPSTR WINAPI get_command_line_hook_narrow() {
    if (new_cmdline_args_narrow)
        return new_cmdline_args_narrow;
    return GetCommandLineA();
}

HANDLE stdout_handle;
bool_t WINAPI close_handle_hook(void *handle) {
    if (stdout_handle && handle == stdout_handle)
        return TRUE;
    return CloseHandle(handle);
}

bool_t initialized = FALSE;
void *WINAPI get_proc_address_detour(void *module, char *name) {
    // If the lpProcName pointer contains an ordinal rather than a string,
    // high-word value of the pointer is zero (see PR #66)
#define REDIRECT_INIT(init_name, init_func, target, extra_init)                \
    if (HIWORD(name) && lstrcmpA(name, init_name) == 0) {                      \
        if (!initialized) {                                                    \
            initialized = TRUE;                                                \
            LOG("Got %S at %p", init_name, module);                            \
            extra_init;                                                        \
            init_func(module);                                                 \
            LOG("Loaded all runtime functions\n")                              \
        }                                                                      \
        return (void *)(target);                                               \
    }

    if (HIWORD(name)) {
        char_t *name_w = widen(name);
        LOG("proc detour \"%s\"", name_w);
        free(name_w);
    }

    REDIRECT_INIT("hostfxr_get_runtime_delegate", load_hostfxr_funcs, hook_hostfxr_get_runtime_delegate, {});

    return (void *)GetProcAddress(module, name);
#undef REDIRECT_INIT
}

void inject(DoorstopPaths const *paths) {

    if (!config.enabled) {
        LOG("Doorstop disabled!");
        free_logger();
        return;
    }

    LOG("Doorstop enabled!");
    HMODULE app_module = GetModuleHandle(NULL);

        LOG("Installing IAT hooks");
    bool_t ok = TRUE;

#define HOOK_SYS(mod, from, to) ok &= iat_hook(mod, "kernel32.dll", &from, &to)

    HOOK_SYS(app_module, GetProcAddress, get_proc_address_detour);

#undef HOOK_SYS

    if (!ok) {
        LOG("Failed to install IAT hook!");
        free_logger();
    } else {
        LOG("Hooks installed");
    }
}

BOOL WINAPI DllEntry(HINSTANCE hInstDll, DWORD reasonForDllLoad,
                     LPVOID reserved) {
    if (reasonForDllLoad == DLL_PROCESS_DETACH)
        SetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL);
    if (reasonForDllLoad != DLL_PROCESS_ATTACH)
        return TRUE;

    init_crt();
    bool_t fixed_cwd = fix_cwd();
    init_logger();
    DoorstopPaths *paths = paths_init(hInstDll, fixed_cwd);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

// TODO: Some MinGW distributons don't seem to have GetFinalPathNameByHandle
// properly defined
#if VERBOSE && !defined(__MINGW32__)
    LOG("Standard output handle at %p", stdout_handle);
    char_t handle_path[MAX_PATH] = TEXT("\0");
    GetFinalPathNameByHandle(stdout_handle, handle_path, MAX_PATH, 0);
    LOG("Standard output handle path: %s", handle_path);
#endif

    load_proxy(paths->doorstop_filename);
    LOG("Proxy loaded");

    load_config();
    LOG("Config loaded");

    if (!file_exists(config.target_assembly)) {
        LOG("Could not find target assembly!");
        config.enabled = FALSE;
    }

    inject(paths);

    paths_free(paths);

    return TRUE;
}
