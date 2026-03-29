#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/select.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/header.h>
#include <httiny/http.h>
#include <httiny/socket.h>
#include <httiny/types.h>

#define BUFFER_SIZE MiB(1)

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
  httiny_http_resp *resp = http_resp_new(arena, HTTINY_STR("Hello World"), 200);
  printf("Sending resp\n");
  httiny_assert(send_http_resp(arena, client_sockfd, resp) == 0 &&
                "Failed to send normal response");
  printf("Sent resp\n");

Close:
  close(client_sockfd);
  return NULL;
}

static volatile int running = 1;

void close_socket(int dummy) {
  printf("Closing socket\n");
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

  start_server(arena);

  arena_destroy(arena);
  return 0;
}
