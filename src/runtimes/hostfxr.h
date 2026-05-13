#ifdef DEFINE_CALLS
#include "hostfxr_types.h"

DEF_CALL(int, get_runtime_delegate,
    const hostfxr_handle host_context_handle,
    enum hostfxr_delegate_type type,
    /*out*/ void **delegate)

DEF_CALL(int, set_runtime_property_value,
const hostfxr_handle host_context_handle,
const char_t *name,
const char_t *value)
DEF_CALL(int, get_runtime_property_value,
    const hostfxr_handle host_context_handle,
    const char_t *name,
    const char_t **value)

#else

#ifndef HOSTFXR_H
#define HOSTFXR_H

#if defined(_WIN32)
#define HOSTFXR_CALLTYPE __cdecl
#else
#define HOSTFXR_CALLTYPE
#endif

#define IMPORT_PREFIX hostfxr
#if _WIN32
#define IMPORT_CONV HOSTFXR_CALLTYPE
#else
#define IMPORT_CONV __attribute__((cdecl))
#endif

#include "func_import.h"

#undef IMPORT_PREFIX
#undef IMPORT_CONV

#endif  // HOSTFXR_H
#endif  // DEFINE_CALLS