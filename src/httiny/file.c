#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/file.h>
#include <httiny/handler.h>
#include <httiny/header.h>
#include <httiny/http.h>
#include <httiny/mime.h>
#include <httiny/string.h>
#include <httiny/types.h>

static string *get_file_body(httiny_arena_t *arena, const string *file_path) {
  const char *file_path_cstr;
  FILE *file = NULL;
  string *body = NULL;
  u64 file_size = 0;

  file_path_cstr = string_get_cstr(arena, file_path);
  file = fopen(file_path_cstr, "rb");
  httiny_assert(file != NULL && "Failed to open file");

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  body = string_new(arena, NULL, file_size);
  httiny_assert(body != NULL && "Failed to allocate body");
  httiny_assert(fread(body->data, 1, file_size, file) == file_size &&
                "Failed to read file correctly");

  fclose(file);
  return body;
}

static string *sanitize_path(httiny_arena_t *arena, const string *path) {
  string *sanitized_path = NULL;
  u64 new_size = path->len;
  u64 copy_offset = 0;

  if (path->data[0] == '.' && path->data[1] == '.') {
    copy_offset += 2;
    new_size -= 2;
  }
  if (path->data[0] == '.' && path->data[1] == '/') {
    copy_offset += 1;
    new_size -= 1;
  }

  if (path->data[0] == '~' && path->data[1] == '/') {
    copy_offset += 1;
    new_size -= 1;
  }

  // TODO: maybe trim index.html

  sanitized_path =
      string_new(arena, (cstr *)path->data + copy_offset, new_size);
  httiny_assert(sanitized_path != NULL && "Failed to allocate sanitized path");
  return sanitized_path;
}

static void parse_dir(httiny_arena_t *arena, httiny_path_conf_t **path_conf,
                      const string *dir) {
  httiny_path_conf_t *path_conf_ptr = *path_conf;
  httiny_path_conf_t *new_path_conf = path_conf_new(arena);
  new_path_conf = *path_conf;

  const cstr *dir_cstr = string_get_cstr(arena, dir);
  DIR *dir_ptr = opendir(dir_cstr);
  struct dirent *entry = NULL;
  httiny_assert(dir_ptr != NULL && "Failed to open directory");

  char canonical_dir[1024];

  if (dir_cstr[dir->len - 1] == '/')
    snprintf(canonical_dir, 1024, "%s", dir_cstr);
  else
    snprintf(canonical_dir, 1024, "%s/", dir_cstr);

  while ((entry = readdir(dir_ptr)) != NULL) {
    if ((entry->d_name[0] == '.' && entry->d_name[1] == '\0') ||
        (entry->d_name[0] == '.' && entry->d_name[1] == '.' &&
         entry->d_name[2] == '\0'))
      continue;

    char file_path[1024];
    snprintf(file_path, 1024, "%s%s", canonical_dir, entry->d_name);

    if (entry->d_type == DT_DIR)
      parse_dir(arena, &new_path_conf,
                string_new(arena, file_path, strlen(file_path)));
    else if (entry->d_type == DT_REG) {
      if (new_path_conf->path_list->size + 1 >= path_conf_ptr->shared_capacity)
        __httiny_path_conf_grow(arena, &new_path_conf,
                                path_conf_ptr->shared_capacity * 2);

      httiny_file_t *new_file = arena_push(arena, sizeof(httiny_file_t));
      httiny_assert(new_file != NULL && "Failed to allocate");

      new_file->file_path = string_new(arena, file_path, strlen(file_path));
      const string *mime_type = get_mime_type(arena, new_file->file_path);
      new_file->mime_type = (string *)mime_type;
      new_path_conf->path_list->paths[new_path_conf->path_list->size++] =
          sanitize_path(arena, string_new(arena, file_path, strlen(file_path)));
    }
  }
  closedir(dir_ptr);
}

static int dummy_handler(void *state, httiny_http_req_t *req) {
  httiny_file_t *file = (httiny_file_t *)state;
  httiny_arena_t *arena = req->thread_arena;
  httiny_header_list_t *headers = req->resp->headers;

  string *file_path = file->file_path;
  httiny_assert(file_path != NULL && "Invalid file path");
  string *mime_type = file->mime_type;
  httiny_assert(mime_type != NULL && "Invalid mime type");

  string *file_body = get_file_body(arena, file_path);

  req->resp->status = 200;
  req->resp->reason = HTTINY_STR("OK");

  httiny_set_body(req, file_body);

  add_header(arena, &headers, HTTINY_CONTENT_TYPE, HTTINY_STR("Content-Type"),
             mime_type);

  httiny_send_resp(req);
  return 0;
}

// TODO: Scour passed directory for files and register them making index.html
// respond on */index.html and */
httiny_path_conf_t *serve_dir(httiny_path_conf_t **path_conf,
                              const string *dir_path,
                              const string_nullable *server_path_root) {
  httiny_path_conf_t *path_conf_ptr = *path_conf;
  httiny_arena_t *arena = path_conf_ptr->thread_arena;
}

httiny_path_conf_t *serve_file(httiny_path_conf_t **path_conf,
                               const string *file_path,
                               const string_nullable *server_path) {
  httiny_path_conf_t *path_conf_ptr = *path_conf;
  httiny_arena_t *arena = path_conf_ptr->thread_arena;
  httiny_path_list_t *path_list = path_conf_ptr->path_list;
  httiny_handler_list_t *handler_list = path_conf_ptr->handler_list;

  httiny_handler_t *new_handler = arena_push(arena, sizeof(*new_handler));
  httiny_assert(new_handler != NULL && "Failed to allocate handler");
  httiny_file_t *file_state = arena_push(arena, sizeof(*file_state));
  file_state->file_path = string_dup(arena, file_path);
  file_state->mime_type = (string *)get_mime_type(arena, (string *)file_path);

  new_handler->callback = dummy_handler;
  new_handler->state = file_state;

  if (path_list->size + 1 == path_conf_ptr->shared_capacity)
    __httiny_path_conf_grow(arena, path_conf,
                            path_conf_ptr->shared_capacity * 2);

  if (server_path)
    (*path_conf)->path_list->paths[path_list->size] =
        sanitize_path(arena, server_path);
  else
    (*path_conf)->path_list->paths[path_list->size] =
        sanitize_path(arena, file_path);

  (*path_conf)->path_list->size = path_list->size + 1;

  (*path_conf)->handler_list->handlers[handler_list->size] = new_handler;
  (*path_conf)->handler_list->size = handler_list->size + 1;

  return *path_conf;
}
