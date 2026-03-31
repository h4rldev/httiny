#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/http.h>
#include <httiny/serve.h>
#include <httiny/socket.h>
#include <httiny/string.h>
#include <httiny/types.h>

#define BUFFER_SIZE MiB(1)

static httiny_path_conf_t *static_path_conf;
static httiny_http_ver_t static_http_ver;
static int static_server_sockfd;

static int default_not_found_handler(void *state, httiny_http_req_t *req) {
  httiny_arena_t *arena = req->thread_arena;
  httiny_header_list_t *headers = req->resp->headers;

  req->resp->status = 404;
  req->resp->reason =
      HTTINY_STR("Not Found"); // Can be NULL and HTTINY will generate the
                               // appropriate reason

  httiny_set_body(req, HTTINY_STR("Not Found"));
  add_header(arena, &headers, HTTINY_CONTENT_TYPE, HTTINY_STR("Content-Type"),
             HTTINY_STR("text/plain"));

  httiny_send_resp(req); // Sends the response
  return 0; // If you return non-zero it will respond with a set body or the set
            // 404 file (TODO)
}

static bool path_exist(httiny_path_conf_t *path_conf, string *path,
                       u64 *index) {
  for (u64 i = 0; i < path_conf->handler_list->size; i++)
    if (string_compare(path, path_conf->path_list->paths[i])) {
      *index = i;
      return true;
    }

  return false;
}

static void *handle_connection(void *arg) {
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
  u64 idx = 0;

  if (path_exist(static_path_conf, path, &idx)) {
    int ret = static_path_conf->handler_list->handlers[idx]->callback(
        static_path_conf->handler_list->handlers[idx]->state, req);
    if (ret != 0)
      default_not_found_handler(NULL, req);
  } else
    default_not_found_handler(NULL, req);

Close:
  close(client_sockfd);
  return NULL;
}

static volatile int running = 1;

static void close_socket(int dummy) {
  printf("\nClosing socket\n");
  running = 0;
}

void httiny_event_loop(void) {
  httiny_arena_t *arena = static_path_conf->thread_arena;
  int *client_sock_fd = arena_push(arena, sizeof(int));

  while (running) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(static_server_sockfd, &read_fds);
    struct timeval timeout = {0, 500000};

    int ret = select(static_server_sockfd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == -1 && errno == EINTR)
      continue;

    if (ret > 0 && FD_ISSET(static_server_sockfd, &read_fds)) {
      if ((*client_sock_fd = accept_socket(static_server_sockfd, NULL)) < 0) {
        perror("Accept failed");
        continue;
      }

      pthread_t thread_id;
      pthread_create(&thread_id, NULL, handle_connection,
                     (void *)client_sock_fd);
      pthread_detach(thread_id);
    }
  }

  close(static_server_sockfd);
}

void httiny_init_server(httiny_path_conf_t *path_conf, const string *ip,
                        const u16 port, httiny_http_ver_t HTTP_ver) {
  switch (HTTP_ver) {
  case HTTP_1_0:
  case HTTP_1_1:
    static_http_ver = HTTP_ver;
    break;
  default:
    httiny_assert(false && "Invalid HTTP version");
  }

  static_path_conf = path_conf;

  httiny_assert(path_conf && "Invalid path conf");
  httiny_assert(path_conf->path_list->size > 0 && "No paths registered");
  httiny_assert(path_conf->handler_list->size > 0 && "No handlers registered");

  struct sockaddr_in serv_addr = make_address(ip, port);

  int server_sockfd = make_socket();
  bind_socket(server_sockfd, serv_addr);
  listen_socket(server_sockfd, 10);

  int flags = fcntl(server_sockfd, F_GETFL, 0);
  fcntl(server_sockfd, F_SETFL, flags | O_NONBLOCK);

  static_server_sockfd = server_sockfd;
  signal(SIGINT, close_socket);
}
