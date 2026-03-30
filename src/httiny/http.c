#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/header.h>
#include <httiny/http.h>
#include <httiny/string.h>
#include <httiny/types.h>
#include <unistd.h>

typedef string httiny_path_t;

typedef struct {
  u16 status;
  const char *reason;
  u64 reason_len;
} status_table_entry;

static status_table_entry status_codes_table[] = {
    {100, HTTINY_STR_LIT("Continue")},
    {101, HTTINY_STR_LIT("Switching Protocols")},
    {103, HTTINY_STR_LIT("Early Hints")},
    {200, HTTINY_STR_LIT("OK")},
    {201, HTTINY_STR_LIT("Created")},
    {202, HTTINY_STR_LIT("Accepted")},
    {203, HTTINY_STR_LIT("Non-Authoritative Information")},
    {204, HTTINY_STR_LIT("No Content")},
    {205, HTTINY_STR_LIT("Reset Content")},
    {206, HTTINY_STR_LIT("Partial Content")},
    {300, HTTINY_STR_LIT("Multiple Choices")},
    {301, HTTINY_STR_LIT("Moved Permanently")},
    {302, HTTINY_STR_LIT("Found")},
    {303, HTTINY_STR_LIT("See Other")},
    {304, HTTINY_STR_LIT("Not Modified")},
    {307, HTTINY_STR_LIT("Temporary Redirect")},
    {308, HTTINY_STR_LIT("Permanent Redirect")},
    {400, HTTINY_STR_LIT("Bad Request")},
    {401, HTTINY_STR_LIT("Unauthorized")},
    {402, HTTINY_STR_LIT("Payment Required")},
    {403, HTTINY_STR_LIT("Forbidden")},
    {404, HTTINY_STR_LIT("Not Found")},
    {405, HTTINY_STR_LIT("Method Not Allowed")},
    {406, HTTINY_STR_LIT("Not Acceptable")},
    {407, HTTINY_STR_LIT("Proxy Authentication Required")},
    {408, HTTINY_STR_LIT("Request Timeout")},
    {409, HTTINY_STR_LIT("Conflict")},
    {410, HTTINY_STR_LIT("Gone")},
    {411, HTTINY_STR_LIT("Length Required")},
    {412, HTTINY_STR_LIT("Precondition Failed")},
    {413, HTTINY_STR_LIT("Content Too Large")},
    {414, HTTINY_STR_LIT("URI Too Long")},
    {415, HTTINY_STR_LIT("Unsupported Media Type")},
    {416, HTTINY_STR_LIT("Range Not Satisfiable")},
    {417, HTTINY_STR_LIT("Expectation Failed")},
    {418, HTTINY_STR_LIT("I'm a teapot")},
    {421, HTTINY_STR_LIT("Misdirected Request")},
    {422, HTTINY_STR_LIT("Unprocessable Content")},
    {423, HTTINY_STR_LIT("Locked")},
    {424, HTTINY_STR_LIT("Failed Dependency")},
    {425, HTTINY_STR_LIT("Too Early")},
    {426, HTTINY_STR_LIT("Upgrade Required")},
    {428, HTTINY_STR_LIT("Precondition Required")},
    {429, HTTINY_STR_LIT("Too Many Requests")},
    {431, HTTINY_STR_LIT("Request Header Fields Too Large")},
    {500, HTTINY_STR_LIT("Internal Server Error")},
    {501, HTTINY_STR_LIT("Not Implemented")},
    {502, HTTINY_STR_LIT("Bad Gateway")},
    {503, HTTINY_STR_LIT("Service Unavailable")},
    {504, HTTINY_STR_LIT("Gateway Timeout")},
    {505, HTTINY_STR_LIT("HTTP Version Not Supported")},
    {506, HTTINY_STR_LIT("Variant Also Negotiates")},
    {507, HTTINY_STR_LIT("Insufficient Storage")},
    {508, HTTINY_STR_LIT("Loop Detected")},
    {510, HTTINY_STR_LIT("Not Extended")},
    {511, HTTINY_STR_LIT("Network Authentication Required")},
    {0, 0, 0},
};

typedef struct {
  string **chunks;
  u64 size;
} httiny_http_msg_chunks_t;

static httiny_http_msg_chunks_t *split_to_chunks(httiny_arena_t *arena,
                                                 string *str, u64 chunk_size) {
  u64 full_chunks = str->len / chunk_size;
  u64 remainder = str->len % chunk_size;
  u64 chunk_count = full_chunks + (remainder ? 1 : 0);

  httiny_http_msg_chunks_t *chunks =
      arena_push(arena, sizeof(httiny_http_msg_chunks_t));
  string **chunks_arr = arena_push(arena, sizeof(string) * chunk_count);

  u64 offset = 0;
  for (u64 i = 0; i < full_chunks; i++) {
    string *chunk = string_new(arena, NULL, chunk_size);
    memcpy(chunk->data, str->data + offset, chunk_size);
    offset += chunk_size;
    chunks_arr[i] = chunk;
  }

  if (remainder) {
    string *chunk = string_new(arena, NULL, remainder);
    memcpy(chunk->data, str->data + offset, remainder);
    chunks_arr[full_chunks] = chunk;
  }

  chunks->size = chunk_count;
  chunks->chunks = chunks_arr;

  return chunks;
}

static int stream_chunk(httiny_arena_t *arena, int sockfd, string *chunk) {
  const unsigned char *chunk_cstr = chunk->data;
  u64 chunk_len = chunk->len;

  const unsigned char *ptr = chunk_cstr;
  while (chunk_len > 0) {
    ssize_t sent = send(sockfd, ptr, chunk_len, 0);
    if (sent <= 0) {
      printf("Failed to send chunk: %s\n", strerror(errno));
      return -1;
    }
    ptr += sent;
    chunk_len -= sent;
  }

  return 0;
}

static int send_chunk(httiny_arena_t *arena, int sockfd, string *chunk) {
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

  return 0;
}

static string *generate_date(httiny_arena_t *arena) {
  time_t t = time(NULL);
  struct tm *tm = gmtime(&t);

  cstr *date = arena_push(arena, sizeof(cstr) * 30);
  strftime(date, 30, "%a, %d %b %Y %H:%M:%S GMT", tm);
  return string_new(arena, date, 29);
}

static string *get_reason(httiny_arena_t *arena, u16 status) {
  for (int i = 0; status_codes_table[i].status != 0; i++)
    if (status_codes_table[i].status == status)
      return string_new(arena, status_codes_table[i].reason,
                        status_codes_table[i].reason_len);
  httiny_assert(false && "Invalid or unrecognized status code");
}

static httiny_header_list_t *parse_headers_from_body(httiny_arena_t *arena,
                                                     string *body) {
  httiny_assert(body != NULL && "Invalid body");

  string *message_dup = string_dup(arena, body);

  httiny_header_list_t *gotten_headers = header_list_new(arena, 2, NULL);

  const cstr *message_cstr = string_get_cstr(arena, message_dup);
  const cstr *message_start = message_cstr;
  const cstr *message_end = strstr(message_start, "\r\n");

  const cstr *headers_start = message_end + 1;
  const cstr *headers_end = strstr(headers_start, "\r\n\r\n");

  const string *header_clump =
      string_new(arena, headers_start, headers_end - headers_start);
  const cstr *headers_cstr = string_get_cstr(arena, (string *)header_clump);

  const cstr *header_clump_start = headers_cstr;
  const cstr *header_clump_end;

  while (*header_clump_start) {
    httiny_header_name_t *header_name = NULL;
    httiny_header_value_t *header_value = NULL;

    header_clump_end = strstr(header_clump_start, "\r\n");
    if (!header_clump_end)
      header_clump_end = strchr(header_clump_start, '\0');

    if (header_clump_end == header_clump_start)
      break;

    const cstr *colon =
        memchr(header_clump_start, ':', header_clump_end - header_clump_start);
    httiny_assert(colon != NULL && "Invalid header, missing colon");

    header_name =
        string_new(arena, header_clump_start, colon - header_clump_start);

    const cstr *header_value_cstr = colon + 1;
    if (*header_value_cstr == ' ')
      header_value_cstr++;

    header_value = string_new(arena, header_value_cstr,
                              header_clump_end - header_value_cstr);

    add_header(arena, &gotten_headers, -1, header_name, header_value);

    if (*header_clump_end == '\r')
      header_clump_start = header_clump_end + 2;
    else
      header_clump_start = header_clump_end;
  }

  return gotten_headers;
}

httiny_http_req_t *http_req_new(httiny_arena_t *arena, int sockfd,
                                string *req_message) {
  httiny_assert(req_message != NULL && "Invalid request message");
  httiny_assert(req_message->len > 0 && "Empty request message");

  httiny_http_resp_t *resp = NULL;
  httiny_http_req_t *req = NULL;
  httiny_header_list_t *gotten_headers = NULL;

  string *method = NULL;
  string *path = NULL;
  string *body = NULL;

  const cstr *req_cstr = string_get_cstr(arena, req_message);
  const cstr *start = req_cstr;
  const cstr *end;

  end = strchr(start, ' ');
  httiny_assert(end != NULL && "Failed to begin parsing HTTP request");
  method = string_new(arena, start, end - start);

  start = end + 1;
  end = strchr(start, ' ');
  httiny_assert(end != NULL && "Failed parsing HTTP request");
  path = string_new(arena, start, end - start);

  gotten_headers =
      parse_headers_from_body(arena, string_dup(arena, req_message));

  httiny_header_list_t *initial_headers = header_list_new(arena, 2, NULL);

  add_header(arena, &initial_headers, HTTINY_SERVER, NULL,
             HTTINY_STR("httiny"));
  add_header(arena, &initial_headers, HTTINY_DATE, NULL, generate_date(arena));
  add_header(arena, &initial_headers, HTTINY_CONTENT_TYPE, NULL,
             HTTINY_STR("text/plain"));
  add_header(arena, &initial_headers, HTTINY_TRANSFER_ENCODING, NULL,
             HTTINY_STR("chunked"));
  add_header(arena, &initial_headers, HTTINY_CONNECTION, NULL,
             HTTINY_STR("keep-alive"));

  resp = arena_push(arena, sizeof(httiny_http_resp_t));
  resp->headers = initial_headers;
  resp->body = NULL;
  resp->status = 200;
  resp->reason = HTTINY_STR("OK");

  req = arena_push(arena, sizeof(httiny_http_req_t));
  req->thread_arena = get_thread_arena(MiB(128), MiB(64));
  req->headers = gotten_headers;
  req->body = body;
  req->method = method;
  req->path = path;
  req->resp = resp;
  req->conn.client_sockfd = sockfd;

  return req;
}

string *stringify_http_header(httiny_arena_t *arena, httiny_http_resp_t *resp) {
  httiny_header_list_t *headers = resp->headers;

  // 256 for padding and spaces and stuff.
  u64 size = (headers->size * (headers->headers[0]->len * 2)) + 256;
  string *http_message = string_new(arena, NULL, size);

  u64 actual_size = 0;

  memcpy(http_message->data, "HTTP/1.1 ", 9);
  actual_size += 9;

  cstr *status = arena_push(arena, 7);
  snprintf(status, 7, "%u ", resp->status);

  memcpy(http_message->data + actual_size, status, 4);
  actual_size += 4;

  memcpy(http_message->data + actual_size, resp->reason->data,
         resp->reason->len);
  actual_size += resp->reason->len;

  memcpy(http_message->data + actual_size, "\r\n", 2);
  actual_size += 2;

  for (u64 i = 0; i < headers->size; i++) {
    memcpy(http_message->data + actual_size, headers->headers[i]->data,
           headers->headers[i]->len);
    actual_size += headers->headers[i]->len;
    memcpy(http_message->data + actual_size, "\r\n", 2);
    actual_size += 2;
  }
  memcpy(http_message->data + actual_size, "\r\n",
         2); // The space between headers and body
  actual_size += 2;

  http_message->len = actual_size;
  return http_message;
}

void httiny_send_resp(httiny_http_req_t *req) {
  int client_sockfd = req->conn.client_sockfd;
  httiny_arena_t *arena = req->thread_arena;
  httiny_http_resp_t *resp = req->resp;
  if (resp->reason == NULL) // Fill reason if it's NULL
    resp->reason = get_reason(arena, resp->status);

  string *http_header = stringify_http_header(arena, resp);
  stream_chunk(arena, req->conn.client_sockfd, http_header);

  httiny_http_msg_chunks_t *chunks = split_to_chunks(arena, resp->body, 1024);

  for (u64 i = 0; i < chunks->size; i++)
    send_chunk(arena, client_sockfd, chunks->chunks[i]);
}

void httiny_set_body(httiny_http_req_t *req, string *body) {
  httiny_arena_t *arena = req->thread_arena;
  req->resp->body = string_dup(arena, body);
}
