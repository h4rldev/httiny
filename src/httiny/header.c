#include <string.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/header.h>
#include <httiny/string.h>
#include <httiny/types.h>

httiny_header_list_t *header_list_new(httiny_arena_t *arena, u64 capacity,
                                      string_nullable *first) {
  httiny_header_list_t *header_list =
      arena_push(arena, sizeof(httiny_header_list_t));
  header_list->size = 0;
  header_list->capacity = capacity;
  header_list->headers = arena_push(arena, sizeof(string *) * capacity);

  if (first != NULL) {
    header_list->headers[0] = first;
    header_list->size++;
  }

  return header_list;
}

void header_list_grow(httiny_arena_t *arena, httiny_header_list_t **header_list,
                      u64 new_capacity) {
  // Why do we abort and not let the arena destroy?
  // Cause the kernel will free the map on process exit anyway.
  httiny_assert((*header_list) != NULL && "Invalid header list");
  httiny_header_list_t *header_list_ptr = *header_list;

  httiny_assert(new_capacity > header_list_ptr->capacity &&
                "Invalid capacity size, can't be smaller than the previous");

  httiny_header_list_t *new_headers =
      header_list_new(arena, new_capacity, header_list_ptr->headers[0]);

  for (u64 i = 1; i < header_list_ptr->size; i++)
    if (header_list_ptr->headers[i] != NULL)
      new_headers->headers[i] = header_list_ptr->headers[i];

  new_headers->size = header_list_ptr->size;

  *header_list = new_headers;
}

void header_append(httiny_arena_t *arena, httiny_header_list_t **header_list,
                   httiny_header_t *header) {
  httiny_header_list_t *header_list_ptr = *header_list;

  if ((header_list_ptr->size + 1) == header_list_ptr->capacity)
    header_list_grow(arena, header_list, header_list_ptr->capacity * 2);

  (*header_list)->headers[header_list_ptr->size] = header;
  (*header_list)->size = header_list_ptr->size + 1;
}

httiny_header_t *header_list_get(httiny_header_list_t *header_list, u64 index) {
  httiny_assert(index > header_list->size && "Invalid index");
  return header_list->headers[index];
}

httiny_header_name_t *get_header_name(httiny_arena_t *arena,
                                      HTTINY_HEADER_KEY key) {
  switch (key) {
  case HTTINY_ACCEPT:
    return string_new(arena, "Accept", 6);
  case HTTINY_ACCEPT_CH:
    return string_new(arena, "Accept-CH", 9);
  case HTTINY_ACCEPT_ENCODING:
    return string_new(arena, "Accept-Encoding", 15);
  case HTTINY_ACCEPT_LANGUAGE:
    return string_new(arena, "Accept-Language", 15);
  case HTTINY_ACCEPT_PATCH:
    return string_new(arena, "Accept-Patch", 12);
  case HTTINY_ACCEPT_POST:
    return string_new(arena, "Accept-Post", 11);
  case HTTINY_ACCEPT_RANGES:
    return string_new(arena, "Accept-Ranges", 13);
  case HTTINY_ACCESS_CONTROL_ALLOW_CREDS:
    return string_new(arena, "Access-Control-Allow-Credentials", 32);
  case HTTINY_ACCESS_CONTROL_ALLOW_HEADERS:
    return string_new(arena, "Access-Control-Allow-Headers", 28);
  case HTTINY_ACCESS_CONTROL_ALLOW_METHODS:
    return string_new(arena, "Access-Control-Allow-Methods", 28);
  case HTTINY_ACCESS_CONTROL_ALLOW_ORIGIN:
    return string_new(arena, "Access-Control-Allow-Origin", 27);
  case HTTINY_ACCESS_CONTROL_EXPOSE_HEADERS:
    return string_new(arena, "Access-Control-Expose-Headers", 29);
  case HTTINY_ACCESS_CONTROL_MAX_AGE:
    return string_new(arena, "Access-Control-Max-Age", 22);
  case HTTINY_ACCESS_CONTROL_REQUEST_HEADERS:
    return string_new(arena, "Access-Control-Request-Headers", 30);
  case HTTINY_ACCESS_CONTROL_REQUEST_METHOD:
    return string_new(arena, "Access-Control-Request-Method", 29);
  case HTTINY_ACTIVATE_STORAGE_ACCESS:
    return string_new(arena, "Activate-Storage-Access", 23);
  case HTTINY_AGE:
    return string_new(arena, "Age", 3);
  case HTTINY_ALLOW:
    return string_new(arena, "Allow", 5);
  case HTTINY_ALT_SVC:
    return string_new(arena, "Alt-Svc", 7);
  case HTTINY_ALT_USED:
    return string_new(arena, "Alt-Used", 8);
  case HTTINY_AUTHORIZATION:
    return string_new(arena, "Authorization", 13);
  case HTTINY_CACHE_CONTROL:
    return string_new(arena, "Cache-Control", 13);
  case HTTINY_CLEAR_SITE_DATA:
    return string_new(arena, "Clear-Site-Data", 15);
  case HTTINY_CONNECTION:
    return string_new(arena, "Connection", 10);
  case HTTINY_CONTENT_DISPOSITION:
    return string_new(arena, "Content-Disposition", 19);
  case HTTINY_CONTENT_ENCODING:
    return string_new(arena, "Content-Encoding", 16);
  case HTTINY_CONTENT_LANGUAGE:
    return string_new(arena, "Content-Language", 16);
  case HTTINY_CONTENT_LENGTH:
    return string_new(arena, "Content-Length", 14);
  case HTTINY_CONTENT_LOCATION:
    return string_new(arena, "Content-Location", 16);
  case HTTINY_CONTENT_RANGE:
    return string_new(arena, "Content-Range", 13);
  case HTTINY_CONTENT_SECURITY_POLICY:
    return string_new(arena, "Content-Security-Policy", 23);
  case HTTINY_CONTENT_SECURITY_POLICY_REPORT_ONLY:
    return string_new(arena, "Content-Security-Policy-Report-Only", 35);
  case HTTINY_CONTENT_TYPE:
    return string_new(arena, "Content-Type", 12);
  case HTTINY_COOKIE:
    return string_new(arena, "Cookie", 5);
  case HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY:
    return string_new(arena, "Cross-Origin-Embedder-Policy", 28);
  case HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY:
    return string_new(arena, "Cross-Origin-Embedder-Policy-Report-Only", 40);
  case HTTINY_CROSS_ORIGIN_OPENER_POLICY:
    return string_new(arena, "Cross-Origin-Opener-Policy", 26);
  case HTTINY_CROSS_ORIGIN_RESOURCE_POLICY:
    return string_new(arena, "Cross-Origin-Resource-Policy", 28);
  case HTTINY_DATE:
    return string_new(arena, "Date", 4);
  case HTTINY_ETAG:
    return string_new(arena, "ETag", 4);
  case HTTINY_EXPECT:
    return string_new(arena, "Expect", 6);
  case HTTINY_EXPIRES:
    return string_new(arena, "Expires", 7);
  case HTTINY_FROM:
    return string_new(arena, "From", 4);
  case HTTINY_HOST:
    return string_new(arena, "Host", 4);
  case HTTINY_IF_MATCH:
    return string_new(arena, "If-Match", 8);
  case HTTINY_IF_MODIFIED_SINCE:
    return string_new(arena, "If-Modified-Since", 17);
  case HTTINY_IF_NONE_MATCH:
    return string_new(arena, "If-None-Match", 13);
  case HTTINY_IF_RANGE:
    return string_new(arena, "If-Range", 8);
  case HTTINY_IF_UNMODIFIED_SINCE:
    return string_new(arena, "If-Unmodified-Since", 19);
  case HTTINY_LAST_MODIFIED:
    return string_new(arena, "Last-Modified", 13);
  case HTTINY_LINK:
    return string_new(arena, "Link", 4);
  case HTTINY_LOCATION:
    return string_new(arena, "Location", 8);
  case HTTINY_MAX_FORWARDS:
    return string_new(arena, "Max-Forwards", 12);
  case HTTINY_ORIGIN:
    return string_new(arena, "Origin", 6);
  case HTTINY_ORIGIN_AGENT_CLUSTER:
    return string_new(arena, "Origin-Agent-Cluster", 20);
  case HTTINY_PREFER:
    return string_new(arena, "Prefer", 6);
  case HTTINY_PREFERENCE_APPLIED:
    return string_new(arena, "Preference-Applied", 18);
  case HTTINY_PRIORITY:
    return string_new(arena, "Priority", 8);
  case HTTINY_PROXY_AUTHENTICATE:
    return string_new(arena, "Proxy-Authenticate", 18);
  case HTTINY_PROXY_AUTHORIZATION:
    return string_new(arena, "Proxy-Authorization", 19);
  case HTTINY_RANGE:
    return string_new(arena, "Range", 5);
  case HTTINY_REFERER:
    return string_new(arena, "Referer", 7);
  case HTTINY_REFERRER_POLICY:
    return string_new(arena, "Referrer-Policy", 15);
  case HTTINY_REFRESH:
    return string_new(arena, "Refresh", 7);
  case HTTINY_REPORTING_ENDPOINTS:
    return string_new(arena, "Reporting-Endpoints", 19);
  case HTTINY_REPR_DIGEST:
    return string_new(arena, "Repr-Digest", 11);
  case HTTINY_RETRY_AFTER:
    return string_new(arena, "Retry-After", 11);
  case HTTINY_SEC_FETCH_DEST:
    return string_new(arena, "Sec-Fetch-Dest", 14);
  case HTTINY_SEC_FETCH_MODE:
    return string_new(arena, "Sec-Fetch-Mode", 14);
  case HTTINY_SEC_FETCH_SITE:
    return string_new(arena, "Sec-Fetch-Site", 14);
  case HTTINY_SEC_FETCH_STORAGE_ACCESS:
    return string_new(arena, "Sec-Fetch-Storage-Access", 24);
  case HTTINY_SEC_FETCH_USER:
    return string_new(arena, "Sec-Fetch-User", 14);
  case HTTINY_SEC_PURPOSE:
    return string_new(arena, "Sec-Purpose", 11);
  case HTTINY_SEC_WEBSOCKET_ACCEPT:
    return string_new(arena, "Sec-WebSocket-Accept", 20);
  case HTTINY_SEC_WEBSOCKET_EXTENSIONS:
    return string_new(arena, "Sec-WebSocket-Extensions", 24);
  case HTTINY_SEC_WEBSOCKET_KEY:
    return string_new(arena, "Sec-WebSocket-Key", 17);
  case HTTINY_SEC_WEBSOCKET_PROTOCOL:
    return string_new(arena, "Sec-WebSocket-Protocol", 22);
  case HTTINY_SEC_WEBSOCKET_VERSION:
    return string_new(arena, "Sec-WebSocket-Version", 21);
  case HTTINY_SERVER:
    return string_new(arena, "Server", 6);
  case HTTINY_SERVER_TIMING:
    return string_new(arena, "Server-Timing", 13);
  case HTTINY_SERVICE_WORKER:
    return string_new(arena, "Service-Worker", 14);
  case HTTINY_SERVICE_WORKER_ALLOWED:
    return string_new(arena, "Service-Worker-Allowed", 22);
  case HTTINY_SERVICE_WORKER_NAVIGATION_PRELOAD:
    return string_new(arena, "Service-Worker-Navigation-Preload", 33);
  case HTTINY_SET_COOKIE:
    return string_new(arena, "Set-Cookie", 10);
  case HTTINY_SET_LOGIN:
    return string_new(arena, "Set-Login", 9);
  case HTTINY_SOURCEMAP:
    return string_new(arena, "SourceMap", 9);
  case HTTINY_STRICT_TRANSPORT_SECURITY:
    return string_new(arena, "Strict-Transport-Security", 25);
  case HTTINY_TE:
    return string_new(arena, "TE", 2);
  case HTTINY_TIMING_ALLOW_ORIGIN:
    return string_new(arena, "Timing-Allow-Origin", 19);
  case HTTINY_TRAILER:
    return string_new(arena, "Trailer", 7);
  case HTTINY_TRANSFER_ENCODING:
    return string_new(arena, "Transfer-Encoding", 17);
  case HTTINY_UPGRADE:
    return string_new(arena, "Upgrade", 7);
  case HTTINY_UPGRADE_INSECURE_REQUESTS:
    return string_new(arena, "Upgrade-Insecure-Requests", 25);
  case HTTINY_USER_AGENT:
    return string_new(arena, "User-Agent", 10);
  case HTTINY_VARY:
    return string_new(arena, "Vary", 4);
  case HTTINY_VIA:
    return string_new(arena, "Via", 3);
  case HTTINY_WANT_CONTENT_DIGEST:
    return string_new(arena, "Want-Content-Digest", 19);
  case HTTINY_WANT_REPR_DIGEST:
    return string_new(arena, "Want-Repr-Digest", 16);
  case HTTINY_WWW_AUTHENTICATE:
    return string_new(arena, "WWW-Authenticate", 16);
  case HTTINY_X_CONTENT_TYPE_OPTIONS:
    return string_new(arena, "X-Content-Type-Options", 22);
  case HTTINY_X_FRAME_OPTIONS:
    return string_new(arena, "X-Frame-Options", 15);
  default:
    httiny_assert(false && "Invalid header");
  }
}

httiny_header_t *create_header(httiny_arena_t *arena,
                               HTTINY_HEADER_KEY header_key,
                               string_nullable *original_key_name,
                               httiny_header_value_t *val) {
  httiny_header_name_t *header_name = get_header_name(arena, header_key);
  httiny_header_t *header =
      string_new(arena, NULL, header_name->len + val->len + 2);

  stringcat(header, header_name);
  memcpy(header->data + header_name->len, ": ", 2);
  memcpy(header->data + header_name->len + 2, val->data, val->len);

  return header;
}
