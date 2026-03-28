#ifndef HTTINY_STRING_H
#define HTTINY_STRING_H

#include <httiny/arena.h>
#include <httiny/types.h>

/*
 * @brief Creates a new string
 *
 * @param arena The arena to allocate the string in.
 * @data The string to add to the string, if NULL, it just allocates an empty
 * string.
 * @param len The length of the string.
 *
 * @return The new empty string.
 */
string *string_new(httiny_arena_t *arena, const cstr_nullable *data, u64 len);

/*
 * @brief Converts a string to cstring.
 *
 * @param arena The arena to allocate the string in.
 * @param string The string to convert.
 *
 * @return The cstring.
 */
const char *string_get_cstr(httiny_arena_t *arena, string *string);

/*
 * @brief Duplicates a string.
 *
 * @param arena The arena to allocate the string in.
 * @param src The string to duplicate.
 *
 * @return The duplicated string.
 */
string *string_dup(httiny_arena_t *arena, string *src);

/*
 * @brief Compares two strings.
 *
 * @param first The first string to compare.
 * @param second The second string to compare.
 *
 * @return true if the strings are equal, false otherwise.
 */
bool string_compare(string *first, string *second);

/*
 * @brief Compare n bytes of second with the first string.
 *
 * @param first The first string to compare.
 * @param second The second string to compare.
 * @param nbytes The number of bytes to compare.
 *
 * @return true if the strings are equal, false otherwise.
 */
bool stringn_compare(string *first, string *second, u64 nbytes);

/*
 * @brief Compares two strings case insensitive.
 *
 * @param first The first string to compare.
 * @param second The second string to compare.
 *
 * @return true if the strings are equal, false otherwise.
 */
bool stringcase_compare(httiny_arena_t *arena, string *first, string *second);

/*
 * @brief Compare n bytes of second with the first string case insensitive.
 *
 * @param arena The arena to dupe the strings in.
 * @param first The first string to compare.
 * @param second The second string to compare.
 * @param nbytes The number of bytes to compare.
 */
bool stringncase_compare(httiny_arena_t *arena, string *first, string *second,
                         u64 nbytes);

/*
 * @brief Concatenates two strings, essentially appending `src` to `dest`.
 *
 * @param dest The string to append to.
 * @param src The string to append.
 *
 * @return The new string.
 */
string *stringcat(string *dest, string *src);

/*
 * @brief Concatenates nbytes from `src` to `dest`.
 *
 * @param dest The string to append to.
 * @param src The string to append.
 * @param nbytes The number of bytes to append.
 *
 * @return The new string.
 */
string *stringncat(string *dest, string *src, u64 nbytes);

/*
 * @brief Prints a string.
 *
 * @param str The string to print.
 *
 * @return The number of bytes printed.
 */
ssize_t print_string(string *str);

/*
 * @brief Prints a string in hex.
 *
 * @param str The string to print.
 */
void print_string_hex(string *str);

#define HTTINY_STR(str) string_new(arena, str, sizeof(str) - 1)
#define HTTINY_STR_LIT(str) str, sizeof(str) - 1

#endif // !HTTINY_STRING_H
