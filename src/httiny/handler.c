#include <string.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/handler.h>

static void __httiny_path_conf_grow(httiny_arena_t *arena,
                                    httiny_path_conf_t **path_conf,
                                    u64 new_capacity) {
  httiny_assert((*path_conf) != NULL && "Invalid path list");

  httiny_handler_list_t *handler_list_ptr = (*path_conf)->handler_list;
  httiny_path_list_t *path_list_ptr = (*path_conf)->path_list;

  httiny_assert((*path_conf)->shared_capacity < new_capacity &&
                "Can't shrink a path conf");

  httiny_path_conf_t *new_path_conf =
      arena_push(arena, sizeof(httiny_path_conf_t));

  httiny_handler_list_t *new_handler_list =
      arena_push(arena, sizeof(httiny_handler_list_t));
  new_handler_list->size = 0;
  new_handler_list->handlers =
      arena_push(arena, sizeof(httiny_handler_t) * new_capacity);

  httiny_path_list_t *new_path_list =
      arena_push(arena, sizeof(httiny_path_list_t));
  new_path_list->size = 0;
  new_path_list->paths =
      arena_push(arena, sizeof(httiny_path_t) * new_capacity);

  // No need to iterate for both, since they share size.
  for (u64 i = 0; i < handler_list_ptr->size; i++)
    if (handler_list_ptr->handlers[i] != NULL &&
        path_list_ptr->paths[i] != NULL) {
      new_handler_list->handlers[i] = handler_list_ptr->handlers[i];
      new_path_list->paths[i] = path_list_ptr->paths[i];
    }

  new_path_conf->handler_list = new_handler_list;
  new_path_conf->path_list = new_path_list;
  new_path_conf->shared_capacity = new_capacity;

  *path_conf = new_path_conf;
}

httiny_path_conf_t *path_conf_new(httiny_arena_t *thread_arena) {
  httiny_path_conf_t *path_conf =
      arena_push(thread_arena, sizeof(httiny_path_conf_t));

  u64 initial_capacity = 2;

  httiny_handler_list_t *handler_list =
      arena_push(thread_arena, sizeof(httiny_handler_list_t));
  httiny_path_list_t *path_list =
      arena_push(thread_arena, sizeof(httiny_path_list_t));

  handler_list->size = 0;
  path_list->size = 0;

  handler_list->handlers =
      arena_push(thread_arena, sizeof(httiny_handler_t) * initial_capacity);
  path_list->paths =
      arena_push(thread_arena, sizeof(httiny_path_t) * initial_capacity);

  path_conf->handler_list = handler_list;
  path_conf->path_list = path_list;
  path_conf->shared_capacity = initial_capacity;
  path_conf->thread_arena = get_thread_arena(MiB(16), MiB(8));

  return path_conf;
}

httiny_path_conf_t *handler_register(httiny_path_conf_t **path_conf,
                                     const httiny_path_t *path,
                                     void *state_nullable,
                                     httiny_callback_t callback) {
  httiny_arena_t *arena = (*path_conf)->thread_arena;
  httiny_handler_list_t *handler_list_ptr = (*path_conf)->handler_list;
  httiny_path_list_t *path_list_ptr = (*path_conf)->path_list;
  httiny_path_conf_t *path_conf_ptr = *path_conf;
  httiny_handler_t *new_handler = arena_push(arena, sizeof(httiny_handler_t));
  httiny_assert(new_handler != NULL && "Failed to allocate handler");

  new_handler->callback = callback;
  new_handler->state = arena_push(arena, sizeof(state_nullable));
  memcpy(new_handler->state, state_nullable, sizeof(state_nullable));

  if (path_list_ptr->size + 1 == path_conf_ptr->shared_capacity)
    __httiny_path_conf_grow((*path_conf)->thread_arena, path_conf,
                            path_conf_ptr->shared_capacity * 2);

  (*path_conf)->path_list->paths[path_list_ptr->size] = (httiny_path_t *)path;
  (*path_conf)->handler_list->handlers[path_list_ptr->size] = new_handler;

  (*path_conf)->path_list->size = path_list_ptr->size + 1;
  (*path_conf)->handler_list->size = handler_list_ptr->size + 1;

  return *path_conf;
}
