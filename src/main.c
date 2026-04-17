#include <stdio.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/file.h>
#include <httiny/handler.h>
#include <httiny/header.h>
#include <httiny/http.h>
#include <httiny/serve.h>
#include <httiny/socket.h>
#include <httiny/types.h>

#include <api/test.h>

int main(void) {
  httiny_arena_t *arena = arena_new(MiB(128), MiB(64));

  httiny_path_conf_t *path_conf = path_conf_new(arena);

  path_conf =
      serve_file(&path_conf, HTTINY_STR("index.html"), HTTINY_STR("/test"));

  httiny_init_server(path_conf, HTTINY_STR("127.0.0.1"), 8081, HTTP_1_1);
  printf("Listening on %s:%u\n", "127.0.0.1", 8081);
  httiny_event_loop();

  arena_destroy(arena);
  return 0;
}
