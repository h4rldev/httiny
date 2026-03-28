#ifndef HTTINY_HTTP_H
#define HTTINY_HTTP_H

#include <httiny/arena.h>
#include <httiny/header.h>
#include <httiny/types.h>

typedef struct {
  httiny_header_list_t *headers;
  string *body;
  u16 status;
  string *reason;
} httiny_http_resp;

typedef struct {
  httiny_header_list_t *headers;
  string *body;
  string *method;
} httiny_http_req;

httiny_http_resp *http_resp_new(httiny_arena_t *arena, string *body,
                                u16 status);

string *stringify_http_header(httiny_arena_t *arena, httiny_http_resp *resp);
int send_http_resp(httiny_arena_t *arena, int sockfd, httiny_http_resp *resp);

#endif // !HTTINY_HTTP_H
