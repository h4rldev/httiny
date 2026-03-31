#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/socket.h>
#include <httiny/string.h>
#include <httiny/types.h>

struct sockaddr_in make_address(const string *ip, const u16 port) {
  static struct sockaddr_in addr = {0};
  cstr *ip_cstr = (cstr *)ip->data;
  ip_cstr[ip->len] = '\0';

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip_cstr);

  return addr;
}

int make_socket(void) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  httiny_assert(sock != -1 && "Failed to create socket");

  int enabled = 1;
  httiny_assert(
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) != -1 &&
      "Failed to set SO_REUSEADDR");

  return sock;
}

void bind_socket(int sockfd, struct sockaddr_in addr) {
  httiny_assert(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != -1 &&
                "Failed to bind socket");
}

void listen_socket(int sockfd, int backlog) {
  httiny_assert(listen(sockfd, backlog) != -1 && "Failed to listen on socket");
}

int accept_socket(int sockfd, struct sockaddr_in *addr) {
  socklen_t addrlen = sizeof(*addr);
  int new_sock = accept(sockfd, (struct sockaddr *)addr, &addrlen);
  httiny_assert(new_sock >= 0 && "Failed to accept socket");

  return new_sock;
}

int stream_chunk(httiny_arena_t *arena, int sockfd, string *chunk) {
  const unsigned char *chunk_cstr = chunk->data;
  u64 chunk_len = chunk->len;

  const unsigned char *ptr = chunk_cstr;
  while (chunk_len > 0) {
    ssize_t sent = send(sockfd, ptr, chunk_len, 0);
    if (sent <= 0) {
      return errno;
    }
    ptr += sent;
    chunk_len -= sent;
  }
  return 0;
}

void send_chunk(httiny_arena_t *arena, int sockfd, string *chunk) {
  string *header = string_new(arena, NULL, 32);
  char header_buf[32];
  u64 size = snprintf(header_buf, 32, "%zx\r\n", chunk->len);
  memcpy(header->data, header_buf, size);
  header->len = size;

  httiny_assert(stream_chunk(arena, sockfd, header) == 0 &&
                "Failed to send chunk size header chunk");
  httiny_assert(stream_chunk(arena, sockfd, chunk) == 0 &&
                "Failed to send chunk data");
  httiny_assert(stream_chunk(arena, sockfd, HTTINY_STR("\r\n")) == 0 &&
                "Failed to send chunk trailer");
  httiny_assert(stream_chunk(arena, sockfd, HTTINY_STR("0\r\n")) == 0 &&
                "Failed to send terminator");
  httiny_assert(stream_chunk(arena, sockfd, HTTINY_STR("\r\n")) == 0 &&
                "Failed to send terminator terminator");
}
