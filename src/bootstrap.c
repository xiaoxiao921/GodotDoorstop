#include "bootstrap.h"
#include "config/config.h"
#include "crt.h"
#include "runtimes/hostfxr.h"
#include "util/logging.h"
#include "util/paths.h"
#include "util/util.h"

#if defined(_WIN32)
    #define CORECLR_DELEGATE_CALLTYPE __stdcall
    #ifdef _WCHAR_T_DEFINED
        typedef wchar_t char_t;
    #else
        typedef unsigned short char_t;
    #endif
#else
    #define CORECLR_DELEGATE_CALLTYPE
    typedef char char_t;
#endif

#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)

// Signature of delegate returned by coreclr_delegate_type::load_assembly_and_get_function_pointer
typedef int (CORECLR_DELEGATE_CALLTYPE *load_assembly_and_get_function_pointer_fn)(
    const char_t *assembly_path      /* Fully qualified path to assembly */,
    const char_t *type_name          /* Assembly qualified type name */,
    const char_t *method_name        /* Public static method name compatible with delegateType */,
    const char_t *delegate_type_name /* Assembly qualified delegate type name or null
                                        or UNMANAGEDCALLERSONLY_METHOD if the method is marked with
                                        the UnmanagedCallersOnlyAttribute. */,
    void         *reserved           /* Extensibility parameter (currently unused and must be 0) */,
    /*out*/ void **delegate          /* Pointer where to store the function pointer result */);

load_assembly_and_get_function_pointer_fn g_load_assembly_and_get_function_pointer_orig = NULL;

int CORECLR_DELEGATE_CALLTYPE hook_load_assembly_and_get_function_pointer(
    const char_t *assembly_path,
    const char_t *type_name_UNUSED,
    const char_t *method_name,
    const char_t *delegate_type_name,
    void         *reserved,
    /*out*/ void **delegate) {

    char_t *target_name = get_file_name(config.target_assembly, FALSE);

    const char_t *prefix = TEXT("GodotPlugins.Game.Main, ");
    char_t *doorstop_type_name = calloc(strlen(prefix) + strlen(target_name) + 1, sizeof(char_t));
    strcat(doorstop_type_name, prefix);
    strcat(doorstop_type_name, target_name);

    LOG("  assembly: %s", config.target_assembly);
    LOG("  type:     %s", doorstop_type_name);
    LOG("  method:   %s", method_name);

    int res = g_load_assembly_and_get_function_pointer_orig(
        config.target_assembly,
        doorstop_type_name,
        method_name,
        delegate_type_name,
        reserved,
        delegate);

    LOG("load_assembly_and_get_function_pointer res: 0x%08x", res);

    return res;
}

bool_t runtime_props_patched = FALSE;

int hook_hostfxr_get_runtime_delegate(
    hostfxr_handle host_context_handle,
    enum hostfxr_delegate_type type,
    /*out*/ void **delegate) {
        int res = hostfxr.get_runtime_delegate(host_context_handle, type, delegate);

        g_load_assembly_and_get_function_pointer_orig = *delegate;

        *delegate = hook_load_assembly_and_get_function_pointer;

        return res;
}

