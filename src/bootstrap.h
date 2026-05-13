#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "util/util.h"
#include "runtimes/hostfxr_types.h"

int hook_hostfxr_get_runtime_delegate(
    hostfxr_handle host_context_handle,
    enum hostfxr_delegate_type type,
    /*out*/ void **delegate);

int hook_coreclr_create_delegate(void *hostHandle,
    unsigned int domainId,
    const char *entryPointAssemblyName,
    const char *entryPointTypeName,
    const char *entryPointMethodName,
    void **delegate);

#endif