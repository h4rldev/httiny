#ifndef HTTINY_TYPES_H
#define HTTINY_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Unsigned types.
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

// Signed types.
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

/*
 * @brief The httiny string type, implemented for ease of use so that strings
 * aren't limited by null termination.
 *
 * @param data The data of the string.
 * @param len The length of the string.
 */
typedef struct {
  u8 *data;
  u64 len;
} string;

// @brief String aliases for readability
typedef string string_nullable;
typedef string httiny_path_t;
typedef string httiny_header_t;
typedef string httiny_header_name_t;
typedef string httiny_header_value_t;

// Type aliases of char to make it more explicit
typedef char cstr;
typedef cstr cstr_nullable;
typedef unsigned char ucstr;
typedef ucstr ucstr_nullable;

/*
 * @brief The httiny arena struct.
 *
 * @param reserved The amount of memory reserved for the arena.
 * @param committed The amount of memory committed for the arena.
 * @param position The current position in the arena.
 * @param commit_position The current position in the committed heap.
 */
typedef struct {
  u64 reserved;
  u64 committed;
  u64 position;
  u64 commit_position;
} httiny_arena_t;

/*
 * @brief The httiny scratch arena struct, for temporary heap allocations.
 *
 * @param start_pos The start position of the scratch arena.
 * @param arena The arena the scratch arena is made from.
 */
typedef struct {
  u64 start_pos;
  httiny_arena_t *arena;
} httiny_scratch_arena_t;

/*
 * @brief Typealiases for readability reasons.
 */

/*
 * @brief A list of headers.
 *
 * @param headers The headers to hold.
 * @param size The size of the list.
 * @param capacity The capacity of the list.
 */
typedef struct {
  httiny_header_t **headers;
  u64 size;
  u64 capacity;
} httiny_header_list_t;

/*
 * @brief An enum of all the headers for HTTP outside of Deprecated and
 * Non-Standard.
 *
 * @note Some of these are forbidden to be sent by the server to the client and
 * vise-versa
 */
typedef enum {
  HTTINY_ACCEPT,
  HTTINY_ACCEPT_CH,
  HTTINY_ACCEPT_ENCODING,
  HTTINY_ACCEPT_LANGUAGE,
  HTTINY_ACCEPT_PATCH,
  HTTINY_ACCEPT_POST,
  HTTINY_ACCEPT_RANGES,
  HTTINY_ACCESS_CONTROL_ALLOW_CREDS,
  HTTINY_ACCESS_CONTROL_ALLOW_HEADERS,
  HTTINY_ACCESS_CONTROL_ALLOW_METHODS,
  HTTINY_ACCESS_CONTROL_ALLOW_ORIGIN,
  HTTINY_ACCESS_CONTROL_EXPOSE_HEADERS,
  HTTINY_ACCESS_CONTROL_MAX_AGE,
  HTTINY_ACCESS_CONTROL_REQUEST_HEADERS,
  HTTINY_ACCESS_CONTROL_REQUEST_METHOD,
  HTTINY_ACTIVATE_STORAGE_ACCESS,
  HTTINY_AGE,
  HTTINY_ALLOW,
  HTTINY_ALT_SVC,
  HTTINY_ALT_USED,
  HTTINY_AUTHORIZATION,
  HTTINY_CACHE_CONTROL,
  HTTINY_CLEAR_SITE_DATA,
  HTTINY_CONNECTION,
  HTTINY_CONTENT_DIGEST,
  HTTINY_CONTENT_DISPOSITION,
  HTTINY_CONTENT_ENCODING,
  HTTINY_CONTENT_LANGUAGE,
  HTTINY_CONTENT_LENGTH,
  HTTINY_CONTENT_LOCATION,
  HTTINY_CONTENT_RANGE,
  HTTINY_CONTENT_SECURITY_POLICY,
  HTTINY_CONTENT_SECURITY_POLICY_REPORT_ONLY,
  HTTINY_CONTENT_TYPE,
  HTTINY_COOKIE,
  HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY,
  HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY,
  HTTINY_CROSS_ORIGIN_OPENER_POLICY,
  HTTINY_CROSS_ORIGIN_RESOURCE_POLICY,
  HTTINY_DATE,
  HTTINY_ETAG,
  HTTINY_EXPECT,
  HTTINY_EXPIRES,
  HTTINY_FORWARDED,
  HTTINY_FROM,
  HTTINY_HOST,
  HTTINY_IF_MATCH,
  HTTINY_IF_MODIFIED_SINCE,
  HTTINY_IF_NONE_MATCH,
  HTTINY_IF_RANGE,
  HTTINY_IF_UNMODIFIED_SINCE,
  HTTINY_INTEGRITY_POLICY,
  HTTINY_INTEGRITY_POLICY_REPORT_ONLY,
  HTTINY_KEEP_ALIVE,
  HTTINY_LAST_MODIFIED,
  HTTINY_LINK,
  HTTINY_LOCATION,
  HTTINY_MAX_FORWARDS,
  HTTINY_ORIGIN,
  HTTINY_ORIGIN_AGENT_CLUSTER,
  HTTINY_PREFER,
  HTTINY_PREFERENCE_APPLIED,
  HTTINY_PRIORITY,
  HTTINY_PROXY_AUTHENTICATE,
  HTTINY_PROXY_AUTHORIZATION,
  HTTINY_RANGE,
  HTTINY_REFERER,
  HTTINY_REFERRER_POLICY,
  HTTINY_REFRESH,
  HTTINY_REPORTING_ENDPOINTS,
  HTTINY_REPR_DIGEST,
  HTTINY_RETRY_AFTER,
  HTTINY_SEC_FETCH_DEST,
  HTTINY_SEC_FETCH_MODE,
  HTTINY_SEC_FETCH_SITE,
  HTTINY_SEC_FETCH_STORAGE_ACCESS,
  HTTINY_SEC_FETCH_USER,
  HTTINY_SEC_PURPOSE,
  HTTINY_SEC_WEBSOCKET_ACCEPT,
  HTTINY_SEC_WEBSOCKET_EXTENSIONS,
  HTTINY_SEC_WEBSOCKET_KEY,
  HTTINY_SEC_WEBSOCKET_PROTOCOL,
  HTTINY_SEC_WEBSOCKET_VERSION,
  HTTINY_SERVER,
  HTTINY_SERVER_TIMING,
  HTTINY_SERVICE_WORKER,
  HTTINY_SERVICE_WORKER_ALLOWED,
  HTTINY_SERVICE_WORKER_NAVIGATION_PRELOAD,
  HTTINY_SET_COOKIE,
  HTTINY_SET_LOGIN,
  HTTINY_SOURCEMAP,
  HTTINY_STRICT_TRANSPORT_SECURITY,
  HTTINY_TE,
  HTTINY_TIMING_ALLOW_ORIGIN,
  HTTINY_TRAILER,
  HTTINY_TRANSFER_ENCODING,
  HTTINY_UPGRADE,
  HTTINY_UPGRADE_INSECURE_REQUESTS,
  HTTINY_USER_AGENT,
  HTTINY_VARY,
  HTTINY_VIA,
  HTTINY_WANT_CONTENT_DIGEST,
  HTTINY_WANT_REPR_DIGEST,
  HTTINY_WWW_AUTHENTICATE,
  HTTINY_X_CONTENT_TYPE_OPTIONS,
  HTTINY_X_FRAME_OPTIONS,
} HTTINY_HEADER_KEY;

/*
 * @brief A table for status codes and their reasons.
 *
 * @param status The status code.
 * @param reason The reason for the status code.
 * @param reason_len The length of the reason.
 */
typedef struct {
  u16 status;
  const char *reason;
  u64 reason_len;
} status_table_entry;

/*
 * @brief A list of HTTP message chunks
 *
 * @note Why does this not have a capacity field? Cause this list never needs to
 * grow as it's initialized and used once per handler.
 *
 * @param chunks The chunks to hold.
 * @param size The size of the list.
 */
typedef struct {
  string **chunks;
  u64 size;
} httiny_http_msg_chunks_t;

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
  string *reason;
  u16 status;
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
 * @brief A function pointer type to handle a request.
 *
 * @param req The request to handle.
 *
 * @return The status code to send.
 */
typedef int (*httiny_callback_t)(void *, httiny_http_req_t *);

/*
 * @brief A handler to handle a request.
 *
 * @param callback The callback to handle the request.
 * @param state The state to pass to the callback.
 */
typedef struct {
  httiny_callback_t callback;
  void *state;
} httiny_handler_t;
/*
 * @brief A list of handlers to handle requests.
 *
 * @param handlers The handlers to handle requests.
 * @param size The size of the list.
 */
typedef struct {
  httiny_handler_t **handlers;
  u64 size;
} httiny_handler_list_t;

/*
 * @brief A list of paths to handle, used to register a handler for a specific
 * path.
 *
 * @param paths The paths to handle.
 * @param size The size of the list.
 */
typedef struct {
  httiny_path_t **paths;
  u64 size;
} httiny_path_list_t;

/*
 * @brief The http version to use.
 *
 * @param HTTP_1_0 HTTP/1.0
 * @param HTTP_1_1 HTTP/1.1
 */
typedef enum {
  HTTP_1_0,
  HTTP_1_1,
} httiny_http_ver_t;

/*
 * @brief A file to serve.
 *
 * @param file_data The data of the file.
 * @param mime_type The mime type of the file.
 */
typedef struct {
  string *file_path;
  string *mime_type;
} httiny_file_t;

/*
 * @brief A list of files to serve.
 *
 * @param file The file to serve.
 * @param size The size of the list.
 */
typedef struct {
  httiny_file_t **files;
  u64 size;
} httiny_file_list_t;

/*
 * @brief The global path configuration for handling what path is for each
 * handler.
 *
 * @param thread_arena The arena for path_conf use.
 * @param handler_conf The configuration for handlers.
 * @param file_conf The configuration for files.
 *
 * @note x_list[index] corresponds to other list in same struct
 */
typedef struct {
  httiny_arena_t *thread_arena;
  httiny_path_list_t *path_list;
  httiny_handler_list_t *handler_list;
  httiny_handler_list_t *not_found_handlers;
  u64 not_found_handlers_capacity;
  u64 shared_capacity;
} httiny_path_conf_t;

#endif // !HTTINY_TYPES_H
