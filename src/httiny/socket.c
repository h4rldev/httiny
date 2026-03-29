#include <arpa/inet.h>

#include <httiny/assert.h>
#include <httiny/types.h>

struct sockaddr_in make_address(string *ip, u16 port) {
  static struct sockaddr_in addr = {0};

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr((char *)ip->data);

  return addr;
}

int make_socket(void) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  httiny_assert(sock != -1 && "Failed to create socket");

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
