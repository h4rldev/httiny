#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include <httiny/arena.h>
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
  assert(send_http_resp(arena, client_sockfd, resp) == 0 &&
         "Failed to send normal response");
  printf("Sent resp\n");

Close:
  close(client_sockfd);
  return NULL;
}

int start_server(httiny_arena_t *arena) {
  struct sockaddr_in serv_addr = make_address(HTTINY_STR("127.0.0.1"), 8080);
  struct sockaddr_in cli_addr;

  int sockfd = make_socket();
  bind_socket(sockfd, serv_addr);
  listen_socket(sockfd, 10);

  int *client_sockfd = arena_push(arena, sizeof(int));
  while (true) {
    if ((*client_sockfd = accept_socket(sockfd, &cli_addr)) < 0) {
      perror("Accept failed");
      continue;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_connection, (void *)client_sockfd);
    pthread_detach(thread_id);
  }
}

int main(void) {
  httiny_arena_t *arena = arena_new(MiB(128), MiB(64));

  start_server(arena);

  arena_destroy(arena);
  return 0;
}
