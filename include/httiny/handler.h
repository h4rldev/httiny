#ifndef HTTINY_HANDLER_H
#define HTTINY_HANDLER_H

#include <httiny/types.h>

/*
 * @brief Creates a new path conf with the given arena.
 *
 * @param thread_arena The arena to allocate the path conf from.
 *
 * @return The new path conf.
 */
httiny_path_conf_t *path_conf_new(httiny_arena_t *thread_arena);

/*
 * @brief Registers a handler for a given path by append
 *
 * @param path The path to register the handler for.
 * @param state_nullable The state to pass to the handler.
 * @param handler The handler to register.
 *
 * @return The updated path conf.
 */
httiny_path_conf_t *handler_register(httiny_path_conf_t **path_conf,
                                     const httiny_path_t *path,
                                     void *state_nullable,
                                     httiny_callback_t callback);

#endif // !HTTINY_HANDLER_H
