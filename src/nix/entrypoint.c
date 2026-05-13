#include "../bootstrap.h"
#include "../config/config.h"
#include "../crt.h"
#include "../util/logging.h"
#include "../util/paths.h"
#include "../util/util.h"
#include "./plthook/plthook.h"

#if defined(__APPLE__)
#define PLTHOOK_OPEN_BY_HANDLE_OR_ADDRESS plthook_open_by_handle
#else
#define PLTHOOK_OPEN_BY_HANDLE_OR_ADDRESS plthook_open_by_address
#endif

static bool_t initialized = FALSE;
void *dlsym_hook(void *handle, const char *name) {
#define REDIRECT_INIT(init_name, init_func, target, extra_init)                \
    if (!strcmp(name, init_name)) {                                            \
        if (!initialized) {                                                    \
            initialized = TRUE;                                                \
            init_func(handle);                                                 \
            extra_init;                                                        \
        }                                                                      \
        return (void *)target;                                                 \
    }

    // Resolve dnsym always so that it can be passed to capture_mono_path.
    // On Unix, we use dladdr which allows to use arbitrary symbols for
    // resolving their location.
    // However, using handle seems to cause issues on some distros, so we pass
    // the resolved symbol instead.
    void *res = dlsym(handle, name);
    REDIRECT_INIT("hostfxr_get_runtime_delegate", load_hostfxr_funcs, hook_hostfxr_get_runtime_delegate, {});

#undef REDIRECT_INIT
    return res;
}

__attribute__((constructor)) void doorstop_ctor() {
    init_logger();
    load_config();

    if (!config.enabled) {
        LOG("Doorstop not enabled! Skipping!");
        return;
    }

    plthook_t *hook;

    if (plthook_open(&hook, NULL) != 0) {
        LOG("Failed to open current process PLT! Cannot run Doorstop! "
            "Error: "
            "%s\n",
            plthook_error());
        return;
    }

    if (plthook_replace(hook, "dlsym", &dlsym_hook, NULL) != 0)
        LOG("Failed to hook dlsym, ignoring it. Error: %s",
               plthook_error());

    plthook_close(hook);
}