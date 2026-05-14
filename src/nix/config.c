#include "../config/config.h"
#include "../util/logging.h"
#include "../crt.h"

void get_env_bool(const char_t *name, bool_t *target) {
    char_t *value = getenv(name);
    if (value != NULL && strcmp(value, "1") == 0) {
        *target = TRUE;
    } else if (value == NULL || strcmp(value, "0") == 0) {
        *target = FALSE;
    }
}

void try_get_env(const char_t *name, char_t *def, char_t **target) {
    char_t *value = getenv(name);
    if (value != NULL && strlen(value) > 0) {
        *target = strdup(value);
    } else {
        *target = def;
    }
}

void get_env_path(const char_t *name, char_t **target) {
    char_t *value = getenv(name);
    if (value != NULL && strlen(value) > 0) {
        *target = get_full_path(value);
    } else {
        *target = NULL;
    }
}

void load_config() {
    get_env_bool("DOORSTOP_ENABLED", &config.enabled);
    get_env_path("DOORSTOP_TARGET_ASSEMBLY", &config.target_assembly);

    //Print out all the relevant configuration settings using LOG()
    LOG("DOORSTOP_ENABLED: %d", config.enabled);
    LOG("DOORSTOP_TARGET_ASSEMBLY: %s", config.target_assembly);
}