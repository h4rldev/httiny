#ifndef HTTINY_HANDLER_H
#define HTTINY_HANDLER_H

#include <httiny/http.h>
#include <httiny/types.h>

typedef string httiny_path_t;

// TODO: Implement handler registration logic, and handling of paths.

/*
 * @brief A function pointer type to handle a request.
 *
 * @param req The request to handle.
 *
 * @return The status code to send.
 */
typedef int (*httiny_callback_t)(void *, httiny_http_req_t *);

/*
 * @brief A handler to handle a request.
 *
 * @param callback The callback to handle the request.
 * @param state The state to pass to the callback.
 */
typedef struct {
  httiny_callback_t callback;
  void *state;
} httiny_handler_t;

/*
 * @brief A list of handlers to handle requests.
 *
 * @param handlers The handlers to handle requests.
 * @param size The size of the list.
 */
typedef struct {
  httiny_handler_t **handlers;
  u64 size;
} httiny_handler_list_t;

/*
 * @brief A list of paths to handle, used to register a handler for a specific
 * path.
 *
 * @param paths The paths to handle.
 * @param size The size of the list.
 */
typedef struct {
  httiny_path_t **paths;
  u64 size;
} httiny_path_list_t;

/*
 * @brief The global path configuration for handling what path is for each
 * handler.
 *
 * @param path_list The list of paths to handle.
 * @param handler_list The list of handlers to handle requests.
 * @param shared_capacity The capacity of the path list and handler list.
 *
 * @note path_list[index] corresponds to handler_list[index]
 */
typedef struct {
  httiny_arena_t *thread_arena;
  httiny_path_list_t *path_list;
  httiny_handler_list_t *handler_list;
  u64 shared_capacity;
} httiny_path_conf_t;

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
httiny_path_conf_t *handler_register(httiny_path_conf_t **oath_conf,
                                     const httiny_path_t *path,
                                     void *state_nullable,
                                     httiny_callback_t callback);

#endif // !HTTINY_HANDLER_H
