#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <httiny/arena.h>
#include <httiny/string.h>
#include <httiny/types.h>

string *string_new(httiny_arena_t *arena, const cstr_nullable *data, u64 len) {
  string *new_str = arena_push(arena, sizeof(string));
  new_str->data = arena_push(arena, sizeof(u8) * len);
  new_str->len = len;

  if (data)
    memcpy(new_str->data, data, len);

  return new_str;
}

const char *string_get_cstr(httiny_arena_t *arena, const string *string) {
  char *cstr = arena_push(arena, sizeof(const char *) * string->len + 1);

  memcpy(cstr, string->data, string->len);
  memset(cstr + string->len, 0, 1);

  return (const char *)cstr;
}

string *string_dup(httiny_arena_t *arena, const string *src) {
  string *new_str = string_new(arena, NULL, src->len);

  memcpy(new_str->data, src->data, src->len);

  return new_str;
}

bool string_compare(const string *first, const string *second) {
  if (!first || !second) {
    fprintf(stderr, "String is NULL\n");
    return false;
  }

  if (first->len != second->len)
    return false;

  return memcmp(first->data, second->data, first->len) == 0;
}

bool stringncompare(const string *first, const string *second, u64 nbytes) {
  return memcmp(first->data, second->data, nbytes) == 0;
}

static void to_lower(string *str) {
  for (u64 i = 0; i < str->len; i++) {
    str->data[i] = tolower(str->data[i]);
  }
}

bool stringcase_compare(httiny_arena_t *arena, const string *first,
                        const string *second) {
  httiny_scratch_arena_t scratch = arena_scratch_new(arena);

  string *first_dup = string_dup(scratch.arena, first);
  string *second_dup = string_dup(scratch.arena, second);

  to_lower(first_dup);
  to_lower(second_dup);

  bool match = memcmp(first_dup->data, second_dup->data, first_dup->len) == 0;

  arena_scratch_destroy(scratch);
  return match;
}

bool stringncase_compare(httiny_arena_t *arena, const string *first,
                         const string *second, u64 nbytes) {
  httiny_scratch_arena_t scratch = arena_scratch_new(arena);

  string *first_dup = string_dup(scratch.arena, first);
  string *second_dup = string_dup(scratch.arena, second);

  to_lower(first_dup);
  to_lower(second_dup);

  bool match = memcmp(first_dup->data, second_dup->data, nbytes) == 0;

  arena_scratch_destroy(scratch);
  return match;
}

string *stringcat(string *dest, const string *src) {
  if (dest->data[0] != 0)
    return memcpy(dest->data + src->len, src->data, src->len);

  return memcpy(dest->data, src->data, src->len);
}

string *stringncat(string *dest, const string *src, u64 nbytes) {
  return memcpy(dest->data + nbytes, src->data, nbytes);
}

ssize_t print_string(const string *str) {
  return printf("%.*s\n", (int)str->len, (const char *)str->data);
}

void print_string_hex(const string *str) {
  for (u64 i = 0; i < str->len - 1; i++)
    printf("%02x ", str->data[i]);
  printf("%02x\n", str->data[str->len - 1]);
}
