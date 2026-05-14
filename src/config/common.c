#include "../crt.h"
#include "config.h"

Config config;

void cleanup_config() {
#define FREE_NON_NULL(val)                                                     \
    if (val != NULL) {                                                         \
        free(val);                                                             \
        val = NULL;                                                            \
    }

    FREE_NON_NULL(config.target_assembly);

#undef FREE_NON_NULL
}

void init_config_defaults() {
    config.enabled = FALSE;
    config.target_assembly = NULL;
}