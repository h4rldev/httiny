#include <string.h>
#include <strings.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/header.h>
#include <httiny/string.h>
#include <httiny/types.h>

static void __header_list_grow(httiny_arena_t *arena,
                               httiny_header_list_t **header_list,
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

static void __header_append(httiny_arena_t *arena,
                            httiny_header_list_t **header_list,
                            httiny_header_t *header) {
  httiny_header_list_t *header_list_ptr = *header_list;

  if ((header_list_ptr->size + 1) == header_list_ptr->capacity)
    __header_list_grow(arena, header_list, header_list_ptr->capacity * 2);

  (*header_list)->headers[header_list_ptr->size] = header;
  (*header_list)->size = header_list_ptr->size + 1;
}

static httiny_header_t *__create_header(httiny_arena_t *arena,
                                        HTTINY_HEADER_KEY header_key,
                                        string_nullable *original_key_name,
                                        httiny_header_value_t *val) {
  httiny_header_name_t *header_name = get_header_name(arena, header_key);
  if (!header_name)
    httiny_assert(original_key_name != NULL &&
                  "Invalid usage of __create_header");

  if (original_key_name != NULL && header_name != NULL)
    httiny_assert(stringncase_compare(arena, header_name, original_key_name,
                                      header_name->len) == true &&
                  "Invalid header name");

  // Assume we're using the original key name if we don't have a header name
  if (!header_name)
    header_name = original_key_name;

  httiny_header_t *header =
      string_new(arena, NULL, header_name->len + val->len + 2);

  stringcat(header, header_name);
  memcpy(header->data + header_name->len, ": ", 2);
  memcpy(header->data + header_name->len + 2, val->data, val->len);

  return header;
}

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

HTTINY_HEADER_KEY get_header_key(httiny_header_t *header) {
  cstr *header_cstr = (cstr *)header->data;

  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept-C")) == 0)
    return HTTINY_ACCEPT_CH;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept-E")) == 0)
    return HTTINY_ACCEPT_ENCODING;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept-L")) == 0)
    return HTTINY_ACCEPT_LANGUAGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept-P")) == 0)
    return HTTINY_ACCEPT_PATCH;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept-P")) == 0)
    return HTTINY_ACCEPT_POST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept-R")) == 0)
    return HTTINY_ACCEPT_RANGES;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Accept")) == 0)
    return HTTINY_ACCEPT;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-Allow-C")) == 0)
    return HTTINY_ACCESS_CONTROL_ALLOW_CREDS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-Allow-H")) == 0)
    return HTTINY_ACCESS_CONTROL_ALLOW_HEADERS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-Allow-M")) == 0)
    return HTTINY_ACCESS_CONTROL_ALLOW_METHODS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-Allow-O")) == 0)
    return HTTINY_ACCESS_CONTROL_ALLOW_ORIGIN;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-E")) == 0)
    return HTTINY_ACCESS_CONTROL_EXPOSE_HEADERS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-M")) == 0)
    return HTTINY_ACCESS_CONTROL_MAX_AGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-Request-H")) == 0)
    return HTTINY_ACCESS_CONTROL_REQUEST_HEADERS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Access-Control-Request-M")) == 0)
    return HTTINY_ACCESS_CONTROL_REQUEST_METHOD;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Activate-S")) == 0)
    return HTTINY_ACTIVATE_STORAGE_ACCESS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Age")) == 0)
    return HTTINY_AGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Allow")) == 0)
    return HTTINY_ALLOW;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Alt-S")) == 0)
    return HTTINY_ALT_SVC;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Alt-U")) == 0)
    return HTTINY_ALT_USED;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Au")) == 0)
    return HTTINY_AUTHORIZATION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Ca")) == 0)
    return HTTINY_CACHE_CONTROL;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Cl")) == 0)
    return HTTINY_CLEAR_SITE_DATA;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Conn")) == 0)
    return HTTINY_CONNECTION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-Dig")) == 0)
    return HTTINY_CONTENT_DIGEST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-Dis")) == 0)
    return HTTINY_CONTENT_DISPOSITION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-En")) == 0)
    return HTTINY_CONTENT_ENCODING;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-La")) == 0)
    return HTTINY_CONTENT_LANGUAGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-Le")) == 0)
    return HTTINY_CONTENT_LENGTH;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-Lo")) == 0)
    return HTTINY_CONTENT_LOCATION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-R")) == 0)
    return HTTINY_CONTENT_RANGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-Security-Policy-R")) ==
      0)
    return HTTINY_CONTENT_SECURITY_POLICY_REPORT_ONLY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-Security-P")) == 0)
    return HTTINY_CONTENT_SECURITY_POLICY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Content-T")) == 0)
    return HTTINY_CONTENT_TYPE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Coo")) == 0)
    return HTTINY_COOKIE;
  if (strncasecmp(header_cstr,
                  HTTINY_STR_LIT("Cross-Origin-Embedder-Policy-R")) == 0)
    return HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Cross-Origin-Embedder-P")) == 0)
    return HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Cross-Origin-O")) == 0)
    return HTTINY_CROSS_ORIGIN_OPENER_POLICY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Cross-Origin-R")) == 0)
    return HTTINY_CROSS_ORIGIN_RESOURCE_POLICY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("D")) == 0)
    return HTTINY_DATE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("ET")) == 0)
    return HTTINY_ETAG;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Expe")) == 0)
    return HTTINY_EXPECT;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Expi")) == 0)
    return HTTINY_EXPIRES;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Fo")) == 0)
    return HTTINY_FORWARDED;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Fr")) == 0)
    return HTTINY_FROM;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("H")) == 0)
    return HTTINY_HOST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("If-Ma")) == 0)
    return HTTINY_IF_MATCH;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("If-Mo")) == 0)
    return HTTINY_IF_MODIFIED_SINCE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("If-N")) == 0)
    return HTTINY_IF_NONE_MATCH;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("If-R")) == 0)
    return HTTINY_IF_RANGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("If-Un")) == 0)
    return HTTINY_IF_UNMODIFIED_SINCE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Integrity-Policy-R")) == 0)
    return HTTINY_INTEGRITY_POLICY_REPORT_ONLY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Integrity-P")) == 0)
    return HTTINY_INTEGRITY_POLICY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("K")) == 0)
    return HTTINY_KEEP_ALIVE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("La")) == 0)
    return HTTINY_LAST_MODIFIED;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Li")) == 0)
    return HTTINY_LINK;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Lo")) == 0)
    return HTTINY_LOCATION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("M")) == 0)
    return HTTINY_MAX_FORWARDS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Origin-A")) == 0)
    return HTTINY_ORIGIN_AGENT_CLUSTER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("O")) == 0)
    return HTTINY_ORIGIN;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Preference-A")) == 0)
    return HTTINY_PREFERENCE_APPLIED;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Pre")) == 0)
    return HTTINY_PREFER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Pri")) == 0)
    return HTTINY_PRIORITY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Proxy-Authe")) == 0)
    return HTTINY_PROXY_AUTHENTICATE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Pro")) == 0)
    return HTTINY_PROXY_AUTHORIZATION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Ra")) == 0)
    return HTTINY_RANGE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Referr")) == 0)
    return HTTINY_REFERRER_POLICY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Refe")) == 0)
    return HTTINY_REFERER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Ref")) == 0)
    return HTTINY_REFRESH;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Repor")) == 0)
    return HTTINY_REPORTING_ENDPOINTS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Repr")) == 0)
    return HTTINY_REPR_DIGEST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Ret")) == 0)
    return HTTINY_RETRY_AFTER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-Fetch-Des")) == 0)
    return HTTINY_SEC_FETCH_DEST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-Fetch-M")) == 0)
    return HTTINY_SEC_FETCH_MODE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-Fetch-Si")) == 0)
    return HTTINY_SEC_FETCH_SITE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-Fetch-St")) == 0)
    return HTTINY_SEC_FETCH_STORAGE_ACCESS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-Fetch-Us")) == 0)
    return HTTINY_SEC_FETCH_USER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-P")) == 0)
    return HTTINY_SEC_PURPOSE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-WebSocket-A")) == 0)
    return HTTINY_SEC_WEBSOCKET_ACCEPT;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-WebSocket-E")) == 0)
    return HTTINY_SEC_WEBSOCKET_EXTENSIONS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-WebSocket-K")) == 0)
    return HTTINY_SEC_WEBSOCKET_KEY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-WebSocket-P")) == 0)
    return HTTINY_SEC_WEBSOCKET_PROTOCOL;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Sec-WebSocket-V")) == 0)
    return HTTINY_SEC_WEBSOCKET_VERSION;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Server-T")) == 0)
    return HTTINY_SERVER_TIMING;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Serve")) == 0)
    return HTTINY_SERVER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Service-Worker-A")) == 0)
    return HTTINY_SERVICE_WORKER_ALLOWED;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Service-Worker-N")) == 0)
    return HTTINY_SERVICE_WORKER_NAVIGATION_PRELOAD;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Servi")) == 0)
    return HTTINY_SERVICE_WORKER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Set-C")) == 0)
    return HTTINY_SET_COOKIE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Set")) == 0)
    return HTTINY_SET_LOGIN;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("So")) == 0)
    return HTTINY_SOURCEMAP;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("S")) == 0)
    return HTTINY_STRICT_TRANSPORT_SECURITY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Te")) == 0)
    return HTTINY_TE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Ti")) == 0)
    return HTTINY_TIMING_ALLOW_ORIGIN;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Trai")) == 0)
    return HTTINY_TRAILER;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Tran")) == 0)
    return HTTINY_TRANSFER_ENCODING;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Upgrade-I")) == 0)
    return HTTINY_UPGRADE_INSECURE_REQUESTS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Up")) == 0)
    return HTTINY_UPGRADE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("U")) == 0)
    return HTTINY_USER_AGENT;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Va")) == 0)
    return HTTINY_VARY;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Vi")) == 0)
    return HTTINY_VIA;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Want-C")) == 0)
    return HTTINY_WANT_CONTENT_DIGEST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Wa")) == 0)
    return HTTINY_WANT_REPR_DIGEST;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("Ww")) == 0)
    return HTTINY_WWW_AUTHENTICATE;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("X-C")) == 0)
    return HTTINY_X_CONTENT_TYPE_OPTIONS;
  if (strncasecmp(header_cstr, HTTINY_STR_LIT("X-F")) == 0)
    return HTTINY_X_FRAME_OPTIONS;
  httiny_assert(false && "Invalid header");
}
httiny_header_name_t *get_header_name(httiny_arena_t *arena,
                                      HTTINY_HEADER_KEY key) {
  if (HTTINY_X_FRAME_OPTIONS < key || key < HTTINY_ACCEPT)
    return NULL;

  switch (key) {
  case HTTINY_ACCEPT:
    return HTTINY_STR("Accept");
  case HTTINY_ACCEPT_CH:
    return HTTINY_STR("Accept-CH");
  case HTTINY_ACCEPT_ENCODING:
    return HTTINY_STR("Accept-Encoding");
  case HTTINY_ACCEPT_LANGUAGE:
    return HTTINY_STR("Accept-Language");
  case HTTINY_ACCEPT_PATCH:
    return HTTINY_STR("Accept-Patch");
  case HTTINY_ACCEPT_POST:
    return HTTINY_STR("Accept-Post");
  case HTTINY_ACCEPT_RANGES:
    return HTTINY_STR("Accept-Ranges");
  case HTTINY_ACCESS_CONTROL_ALLOW_CREDS:
    return HTTINY_STR("Access-Control-Allow-Credentials");
  case HTTINY_ACCESS_CONTROL_ALLOW_HEADERS:
    return HTTINY_STR("Access-Control-Allow-Headers");
  case HTTINY_ACCESS_CONTROL_ALLOW_METHODS:
    return HTTINY_STR("Access-Control-Allow-Methods");
  case HTTINY_ACCESS_CONTROL_ALLOW_ORIGIN:
    return HTTINY_STR("Access-Control-Allow-Origin");
  case HTTINY_ACCESS_CONTROL_EXPOSE_HEADERS:
    return HTTINY_STR("Access-Control-Expose-Headers");
  case HTTINY_ACCESS_CONTROL_MAX_AGE:
    return HTTINY_STR("Access-Control-Max-Age");
  case HTTINY_ACCESS_CONTROL_REQUEST_HEADERS:
    return HTTINY_STR("Access-Control-Request-Headers");
  case HTTINY_ACCESS_CONTROL_REQUEST_METHOD:
    return HTTINY_STR("Access-Control-Request-Method");
  case HTTINY_ACTIVATE_STORAGE_ACCESS:
    return HTTINY_STR("Activate-Storage-Access");
  case HTTINY_AGE:
    return HTTINY_STR("Age");
  case HTTINY_ALLOW:
    return HTTINY_STR("Allow");
  case HTTINY_ALT_SVC:
    return HTTINY_STR("Alt-Svc");
  case HTTINY_ALT_USED:
    return HTTINY_STR("Alt-Used");
  case HTTINY_AUTHORIZATION:
    return HTTINY_STR("Authorization");
  case HTTINY_CACHE_CONTROL:
    return HTTINY_STR("Cache-Control");
  case HTTINY_CLEAR_SITE_DATA:
    return HTTINY_STR("Clear-Site-Data");
  case HTTINY_CONNECTION:
    return HTTINY_STR("Connection");
  case HTTINY_CONTENT_DISPOSITION:
    return HTTINY_STR("Content-Disposition");
  case HTTINY_CONTENT_ENCODING:
    return HTTINY_STR("Content-Encoding");
  case HTTINY_CONTENT_LANGUAGE:
    return HTTINY_STR("Content-Language");
  case HTTINY_CONTENT_LENGTH:
    return HTTINY_STR("Content-Length");
  case HTTINY_CONTENT_LOCATION:
    return HTTINY_STR("Content-Location");
  case HTTINY_CONTENT_RANGE:
    return HTTINY_STR("Content-Range");
  case HTTINY_CONTENT_SECURITY_POLICY:
    return HTTINY_STR("Content-Security-Policy");
  case HTTINY_CONTENT_SECURITY_POLICY_REPORT_ONLY:
    return HTTINY_STR("Content-Security-Policy-Report-Only");
  case HTTINY_CONTENT_TYPE:
    return HTTINY_STR("Content-Type");
  case HTTINY_COOKIE:
    return HTTINY_STR("Cookie");
  case HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY:
    return HTTINY_STR("Cross-Origin-Embedder-Policy");
  case HTTINY_CROSS_ORIGIN_EMBEDDER_POLICY_REPORT_ONLY:
    return HTTINY_STR("Cross-Origin-Embedder-Policy-Report-Only");
  case HTTINY_CROSS_ORIGIN_OPENER_POLICY:
    return HTTINY_STR("Cross-Origin-Opener-Policy");
  case HTTINY_CROSS_ORIGIN_RESOURCE_POLICY:
    return HTTINY_STR("Cross-Origin-Resource-Policy");
  case HTTINY_DATE:
    return HTTINY_STR("Date");
  case HTTINY_ETAG:
    return HTTINY_STR("ETag");
  case HTTINY_EXPECT:
    return HTTINY_STR("Expect");
  case HTTINY_EXPIRES:
    return HTTINY_STR("Expires");
  case HTTINY_FROM:
    return HTTINY_STR("From");
  case HTTINY_HOST:
    return HTTINY_STR("Host");
  case HTTINY_IF_MATCH:
    return HTTINY_STR("If-Match");
  case HTTINY_IF_MODIFIED_SINCE:
    return HTTINY_STR("If-Modified-Since");
  case HTTINY_IF_NONE_MATCH:
    return HTTINY_STR("If-None-Match");
  case HTTINY_IF_RANGE:
    return HTTINY_STR("If-Range");
  case HTTINY_IF_UNMODIFIED_SINCE:
    return HTTINY_STR("If-Unmodified-Since");
  case HTTINY_LAST_MODIFIED:
    return HTTINY_STR("Last-Modified");
  case HTTINY_LINK:
    return HTTINY_STR("Link");
  case HTTINY_LOCATION:
    return HTTINY_STR("Location");
  case HTTINY_MAX_FORWARDS:
    return HTTINY_STR("Max-Forwards");
  case HTTINY_ORIGIN:
    return HTTINY_STR("Origin");
  case HTTINY_ORIGIN_AGENT_CLUSTER:
    return HTTINY_STR("Origin-Agent-Cluster");
  case HTTINY_PREFER:
    return HTTINY_STR("Prefer");
  case HTTINY_PREFERENCE_APPLIED:
    return HTTINY_STR("Preference-Applied");
  case HTTINY_PRIORITY:
    return HTTINY_STR("Priority");
  case HTTINY_PROXY_AUTHENTICATE:
    return HTTINY_STR("Proxy-Authenticate");
  case HTTINY_PROXY_AUTHORIZATION:
    return HTTINY_STR("Proxy-Authorization");
  case HTTINY_RANGE:
    return HTTINY_STR("Range");
  case HTTINY_REFERER:
    return HTTINY_STR("Referer");
  case HTTINY_REFERRER_POLICY:
    return HTTINY_STR("Referrer-Policy");
  case HTTINY_REFRESH:
    return HTTINY_STR("Refresh");
  case HTTINY_REPORTING_ENDPOINTS:
    return HTTINY_STR("Reporting-Endpoints");
  case HTTINY_REPR_DIGEST:
    return HTTINY_STR("Repr-Digest");
  case HTTINY_RETRY_AFTER:
    return HTTINY_STR("Retry-After");
  case HTTINY_SEC_FETCH_DEST:
    return HTTINY_STR("Sec-Fetch-Dest");
  case HTTINY_SEC_FETCH_MODE:
    return HTTINY_STR("Sec-Fetch-Mode");
  case HTTINY_SEC_FETCH_SITE:
    return HTTINY_STR("Sec-Fetch-Site");
  case HTTINY_SEC_FETCH_STORAGE_ACCESS:
    return HTTINY_STR("Sec-Fetch-Storage-Access");
  case HTTINY_SEC_FETCH_USER:
    return HTTINY_STR("Sec-Fetch-User");
  case HTTINY_SEC_PURPOSE:
    return HTTINY_STR("Sec-Purpose");
  case HTTINY_SEC_WEBSOCKET_ACCEPT:
    return HTTINY_STR("Sec-WebSocket-Accept");
  case HTTINY_SEC_WEBSOCKET_EXTENSIONS:
    return HTTINY_STR("Sec-WebSocket-Extensions");
  case HTTINY_SEC_WEBSOCKET_KEY:
    return HTTINY_STR("Sec-WebSocket-Key");
  case HTTINY_SEC_WEBSOCKET_PROTOCOL:
    return HTTINY_STR("Sec-WebSocket-Protocol");
  case HTTINY_SEC_WEBSOCKET_VERSION:
    return HTTINY_STR("Sec-WebSocket-Version");
  case HTTINY_SERVER:
    return HTTINY_STR("Server");
  case HTTINY_SERVER_TIMING:
    return HTTINY_STR("Server-Timing");
  case HTTINY_SERVICE_WORKER:
    return HTTINY_STR("Service-Worker");
  case HTTINY_SERVICE_WORKER_ALLOWED:
    return HTTINY_STR("Service-Worker-Allowed");
  case HTTINY_SERVICE_WORKER_NAVIGATION_PRELOAD:
    return HTTINY_STR("Service-Worker-Navigation-Preload");
  case HTTINY_SET_COOKIE:
    return HTTINY_STR("Set-Cookie");
  case HTTINY_SET_LOGIN:
    return HTTINY_STR("Set-Login");
  case HTTINY_SOURCEMAP:
    return HTTINY_STR("SourceMap");
  case HTTINY_STRICT_TRANSPORT_SECURITY:
    return HTTINY_STR("Strict-Transport-Security");
  case HTTINY_TE:
    return HTTINY_STR("TE");
  case HTTINY_TIMING_ALLOW_ORIGIN:
    return HTTINY_STR("Timing-Allow-Origin");
  case HTTINY_TRAILER:
    return HTTINY_STR("Trailer");
  case HTTINY_TRANSFER_ENCODING:
    return HTTINY_STR("Transfer-Encoding");
  case HTTINY_UPGRADE:
    return HTTINY_STR("Upgrade");
  case HTTINY_UPGRADE_INSECURE_REQUESTS:
    return HTTINY_STR("Upgrade-Insecure-Requests");
  case HTTINY_USER_AGENT:
    return HTTINY_STR("User-Agent");
  case HTTINY_VARY:
    return HTTINY_STR("Vary");
  case HTTINY_VIA:
    return HTTINY_STR("Via");
  case HTTINY_WANT_CONTENT_DIGEST:
    return HTTINY_STR("Want-Content-Digest");
  case HTTINY_WANT_REPR_DIGEST:
    return HTTINY_STR("Want-Repr-Digest");
  case HTTINY_WWW_AUTHENTICATE:
    return HTTINY_STR("WWW-Authenticate");
  case HTTINY_X_CONTENT_TYPE_OPTIONS:
    return HTTINY_STR("X-Content-Type-Options");
  case HTTINY_X_FRAME_OPTIONS:
    return HTTINY_STR("X-Frame-Options");
  default:
    httiny_assert(false && "Invalid header");
  }
}

httiny_header_name_t *get_header_name_from_header(httiny_arena_t *arena,
                                                  httiny_header_t *header) {
  const cstr *header_cstr = string_get_cstr(arena, header);
  httiny_assert(header_cstr != NULL && "Invalid header");
  httiny_assert(strchr(header_cstr, ':') != NULL && "Invalid header");

  cstr *buf = NULL;
  while (*header_cstr || *header_cstr != ':') {
    *buf = *header_cstr;

    buf++;
    header_cstr++;
  }

  u64 len = 0;

  httiny_assert((len = buf - header_cstr) < header->len && "Invalid header");
  return string_new(arena, header_cstr, len);
}

httiny_header_t *get_header_from_list(httiny_arena_t *arena,
                                      httiny_header_list_t *header_list,
                                      HTTINY_HEADER_KEY key, i64 *cursor) {
  httiny_header_name_t *wanted_header_name = get_header_name(arena, key);
  httiny_header_t *wanted_header = NULL;

  httiny_assert(cursor != NULL && "Invalid cursor, pass cursor to be -1");

  u64 local_cursor = *cursor;
  if (*cursor < 0)
    local_cursor = 0;

  for (; local_cursor < header_list->size; local_cursor++) {
    httiny_header_t *header = header_list->headers[local_cursor];
    if (stringcase_compare(arena, wanted_header_name,
                           get_header_name_from_header(arena, header))) {
      wanted_header = header;
      break;
    }
  }

  if (*cursor < 0)
    *cursor = local_cursor;

  return wanted_header;
}

u64 add_header(httiny_arena_t *arena, httiny_header_list_t **header_list,
               HTTINY_HEADER_KEY header_key, string_nullable *original_key_name,
               httiny_header_value_t *val) {
  httiny_assert(header_list && "Invalid header list");
  httiny_assert(*header_list && "Invalid header list");

  httiny_header_t *header =
      __create_header(arena, header_key, original_key_name, val);
  httiny_assert(header != NULL && "Failed to create header");

  __header_append(arena, header_list, header);
  return (*header_list)->size - 1;
}
