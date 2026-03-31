#ifndef HTTINY_SERVE_H
#define HTTINY_SERVE_H

#include <httiny/types.h>

/*
 * @brief Initializes and listens the server
 *
 * @note Why split up the accept loop and the server listening? To allow the
 * user to more easily make an accept loop.
 *
 * @param path_conf The path conf to use.
 * @param ip The IP to listen on.
 * @param port The port to listen on.
 * @param HTTP_ver The HTTP version to use, (HTTP_1_0 or HTTP_1_1, HTTP2+ wont
 * be implemented)
 *
 * @return 0 on success, -1 on failure.
 */
void httiny_init_server(httiny_path_conf_t *path_conf, const string *ip,
                        u16 port, httiny_http_ver_t HTTP_ver);

/*
 * @brief Starts the accept event loop.
 *
 * @note Uses the path conf that's exposed by httiny_init_server and whatever.
 */
void httiny_event_loop(void);

#endif // !HTTINY_SERVE_H
