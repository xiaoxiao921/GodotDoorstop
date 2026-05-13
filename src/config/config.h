#ifndef CONFIG_H
#define CONFIG_H

#include "../util/util.h"

/**
 * @brief Doorstop configuration
 */
typedef struct {
    /**
     * @brief Whether Doorstop is enabled (enables hooking methods and executing
     * target assembly).
     */
    bool_t enabled;

    /**
     * @brief Whether to ignore DOORSTOP_DISABLE.
     *
     * If enabled, Doorstop will ignore DOORSTOP_DISABLE environment variable.
     * This is sometimes useful with Steam games that break env var isolation.
     */
    bool_t ignore_disabled_env;

    /**
     * @brief Path to a managed assembly to invoke.
     */
    char_t *target_assembly;
} Config;

extern Config config;

/**
 * @brief Load configuration.
 */
extern void load_config();

/**
 * @brief Initialize default values for configuration.
 */
extern void init_config_defaults();

/**
 * @brief Clean up configuration.
 */
extern void cleanup_config();
#endif