#ifndef HTTINY_SOCKET_H
#define HTTINY_SOCKET_H

#include <netinet/in.h>

#include <httiny/types.h>

/*
 * @brief Creates a new sockaddr_in
 *
 * @param ip The IP address to create the sockaddr_in for.
 * @param port The port to create the sockaddr_in for.
 */
struct sockaddr_in make_address(string *ip, u16 port);

/*
 * @brief Creates a new socket (currently only AF_INET)
 *
 * @return The new socket fd.
 */
int make_socket(void);

/*
 * @brief Binds a socket to an address
 *
 * @param sockfd The socket to bind.
 * @param addr The address to bind to.
 */
void bind_socket(int sockfd, struct sockaddr_in addr);

/*
 * @brief Listens on a socket
 *
 * @param sockfd The socket to listen on.
 * @param backlog The backlog to listen on.
 */
void listen_socket(int sockfd, int backlog);

/*
 * @brief Accepts a new socket connection.
 *
 * @param sockfd The socket to accept on.
 * @param addr The address to accept the connection on.
 *
 * @return The new socket fd.
 */
int accept_socket(int sockfd, struct sockaddr_in *addr);

#endif // !HTTINY_SOCKET_H
