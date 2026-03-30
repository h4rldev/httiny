#ifndef HTTINY_HTTP_H
#define HTTINY_HTTP_H

#include <httiny/arena.h>
#include <httiny/header.h>
#include <httiny/types.h>

/*
 * @brief A http response.
 *
 * @param headers The headers to send.
 * @param body The body to send.
 * @param status The status code to send.
 * @param reason The status reason to send.
 */
typedef struct {
  httiny_header_list_t *headers;
  string *body;
  u16 status;
  string *reason;
} httiny_http_resp_t;

/*
 * @brief A http request.
 *
 * @param thread_arena The arena for handler use.
 * @param headers The headers gotten from the client.
 * @param body The body gotten from the client.
 * @param method The method gotten from the client.
 * @param path The path the request was made on.
 * @param res The response to send.
 *
 * @param conn The connection information, not for direct use.
 */
typedef struct {
  httiny_arena_t *thread_arena;
  httiny_header_list_t *headers;
  string *body;
  string *method;
  string *path;
  httiny_http_resp_t *resp;
  struct {
    int client_sockfd;
  } conn;
} httiny_http_req_t;

/*
 * @brief Creates a new http request body from a string for processing.
 *
 * @param arena The arena to allocate the request from (is the thread arena for
 * the current registered handler).
 * @param sockfd The socket from the request thread.
 * @param req_message The request message body.
 *
 * @return The new http request to populate a handler.
 */
httiny_http_req_t *http_req_new(httiny_arena_t *arena, int sockfd,
                                string *req_message);

/*
 * @brief Stringifies the HTTP header for a response.
 * Taking the headers, status code and reason and gives out a valid HTTP header.
 *
 * @param arena The arena to allocate the header from.
 * @param resp The response to stringify from.
 *
 * @return The stringified HTTP header.
 */
string *stringify_http_header(httiny_arena_t *arena, httiny_http_resp_t *resp);

/*
 * @brief Sends a response to the client.
 *
 * @param req The request to send the response for.
 *
 * @returns nothing, if something goes wrong, it will lead to an assert failure.
 */
void httiny_send_resp(httiny_http_req_t *req);

/*
 * @brief Sets the body of the request.
 *
 * @param req The request to set the body for.
 * @param body The body to set.
 *
 * @returns nothing, if something goes wrong, it will lead to an assert failure.
 */
void httiny_set_body(httiny_http_req_t *req, string *body);

#endif // !HTTINY_HTTP_H
