#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include <httiny/arena.h>
#include <httiny/header.h>
#include <httiny/http.h>
#include <httiny/string.h>
#include <httiny/types.h>

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
  const char *chunk_cstr = string_get_cstr(arena, chunk);
  u64 chunk_len = chunk->len;

  const char *ptr = chunk_cstr;
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
  printf("Chunk len: %lu\n", chunk->len);
  u64 size = snprintf(header_buf, 32, "%zx\r\n", chunk->len);
  memcpy(header->data, header_buf, size - 1);
  header->len = size - 1;

  printf("Sending headers: %.*s\n", (int)header->len, header->data);
  printf("Sending chunk: %.*s\n", (int)chunk->len, chunk->data);
  printf("Chunk size: %lu\n", chunk->len);

  assert(stream_chunk(arena, sockfd, header) == 0 &&
         "Failed to send chunk size header chunk");
  assert(stream_chunk(arena, sockfd, chunk) == 0 &&
         "Failed to send chunk data");
  assert(stream_chunk(arena, sockfd, HTTINY_STR("\r\n")) == 0 &&
         "Failed to send chunk trailer");

  return 0;
}

static string *generate_date(httiny_arena_t *arena) {
  time_t t = time(NULL);
  struct tm *tm = gmtime(&t);

  cstr *date = arena_push(arena, sizeof(cstr) * 36);
  strftime(date, 36, "Date: %a, %d %b %Y %H:%M:%S GMT", tm);
  return string_new(arena, date, 35);
}

static string *get_reason(httiny_arena_t *arena, u16 status) {
  for (int i = 0; status_codes_table[i].status != 0; i++)
    if (status_codes_table[i].status == status)
      return string_new(arena, status_codes_table[i].reason,
                        status_codes_table[i].reason_len);
  assert(false && "Invalid or unrecognized status code");
}
httiny_http_resp *http_resp_new(httiny_arena_t *arena, string *body,
                                u16 status) {
  httiny_http_resp *resp = arena_push(arena, sizeof(httiny_http_resp));
  resp->body = body;
  resp->status = status;
  resp->reason = get_reason(arena, status);

  httiny_header_list_t *headers = header_list_new(arena, 2, NULL);
  header_append(arena, headers, HTTINY_STR("Server: HTTiny"));

  if (status >= 200 && (status != 204 && status != 304)) {
    printf("Adding date\n");
    header_append(arena, headers, generate_date(arena));
    printf("Adding transfer encoding\n");
    header_append(arena, headers, HTTINY_STR("Transfer-Encoding: chunked"));
  }

  header_append(arena, headers, HTTINY_STR("Connection: keep-alive"));

  resp->headers = headers;
  return resp;
}

string *stringify_http_header(httiny_arena_t *arena, httiny_http_resp *resp) {
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
    printf("Adding header: %.*s\n", (int)headers->headers[i]->len,
           headers->headers[i]->data);
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
  printf("http_message: %.*s\n", (int)http_message->len, http_message->data);
  return http_message;
}

int send_http_resp(httiny_arena_t *arena, int sockfd, httiny_http_resp *resp) {
  string *http_header = stringify_http_header(arena, resp);
  send_chunk(arena, sockfd, http_header);

  httiny_http_msg_chunks_t *chunks = split_to_chunks(arena, resp->body, 1024);

  for (u64 i = 0; i < chunks->size; i++)
    send_chunk(arena, sockfd, chunks->chunks[i]);

  return 0;
}
