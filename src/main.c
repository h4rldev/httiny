#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/select.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/handler.h>
#include <httiny/header.h>
#include <httiny/http.h>
#include <httiny/socket.h>
#include <httiny/types.h>

#include <api/test.h>

#define BUFFER_SIZE MiB(1)
static httiny_path_conf_t *path_conf;

int not_found_handler(void *state, httiny_http_req_t *req) {
  httiny_arena_t *arena = req->thread_arena;

  req->resp->status = 404;
  req->resp->reason =
      HTTINY_STR("Not Found"); // Can be NULL and HTTINY will generate the
                               // appropriate reason

  httiny_set_body(req, HTTINY_STR("Not Found"));
  add_header(arena, &req->resp->headers, HTTINY_CONTENT_TYPE,
             HTTINY_STR("Content-Type"), HTTINY_STR("text/plain"));

  httiny_send_resp(req); // Sends the response

  return 0; // If you return non-zero it will respond with a set body or the set
            // 404 file (TODO)
}

void *handle_connection(void *arg) {
  int client_sockfd = *(int *)arg;
  httiny_arena_t *arena = get_thread_arena(MiB(128), MiB(64));
  string *request = string_new(arena, NULL, BUFFER_SIZE);

  ssize_t received = recv(client_sockfd, request->data, BUFFER_SIZE, 0);
  if (received <= 0) {
    printf("Client closed connection or connection is empty?\n");
    goto Close;
  }

  printf("Bytes received: %lu\n", received);
  httiny_http_req_t *req = http_req_new(arena, client_sockfd, request);
  string *path = req->path;

  for (u64 i = 0; i < path_conf->handler_list->size; i++) {
    // TODO: Return basic 404 if unpopulated.
    httiny_assert((path_conf->handler_list->handlers[i] != NULL ||
                   path_conf->path_list->paths[i] != NULL) &&
                  "Path conf is unpopulated");
    printf("Got request on path %.*s\n", (int)path->len, path->data);

    printf("Comparing %.*s with %.*s\n", (int)path->len, path->data,
           (int)path_conf->path_list->paths[i]->len,
           path_conf->path_list->paths[i]->data);
    if (string_compare(path, path_conf->path_list->paths[i])) {
      printf("Handling request for %.*s\n", (int)path->len, path->data);
      int ret = path_conf->handler_list->handlers[i]->callback(
          path_conf->handler_list->handlers[i]->state, req);
      if (ret != 0)
        not_found_handler(NULL, req);
    }
  }
Close:
  close(client_sockfd);
  return NULL;
}

static volatile int running = 1;

void close_socket(int dummy) {
  printf("\nClosing socket\n");
  running = 0;
}

int start_server(httiny_arena_t *arena) {
  struct sockaddr_in serv_addr = make_address(HTTINY_STR("127.0.0.1"), 8081);
  struct sockaddr_in cli_addr;

  int server_sockfd = make_socket();
  bind_socket(server_sockfd, serv_addr);
  listen_socket(server_sockfd, 10);

  int flags = fcntl(server_sockfd, F_GETFL, 0);
  fcntl(server_sockfd, F_SETFL, flags | O_NONBLOCK);

  signal(SIGINT, close_socket);

  printf("Listening on %s:%u\n", inet_ntoa(serv_addr.sin_addr),
         ntohs(serv_addr.sin_port));

  int *client_sockfd = arena_push(arena, sizeof(int));
  while (running) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_sockfd, &read_fds);
    struct timeval timeout = {0, 500000};

    int ret = select(server_sockfd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1 && errno == EINTR)
      continue;
    if (ret > 0 && FD_ISSET(server_sockfd, &read_fds)) {
      if ((*client_sockfd = accept_socket(server_sockfd, &cli_addr)) < 0) {
        perror("Accept failed");
        continue;
      }

      pthread_t thread_id;
      pthread_create(&thread_id, NULL, handle_connection,
                     (void *)client_sockfd);
      pthread_detach(thread_id);
    }
  }

  return 0;
}

int main(void) {
  httiny_arena_t *arena = arena_new(MiB(128), MiB(64));

  path_conf = path_conf_new(arena);

  path_conf = handler_register(&path_conf, HTTINY_STR("/"), NULL, test_handler);

  start_server(arena);

  arena_destroy(arena);
  return 0;
}
