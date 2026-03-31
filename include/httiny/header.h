#ifndef HTTINY_HEADER_H
#define HTTINY_HEADER_H

#include <httiny/types.h>

/*
 * @brief Creates a new header list with the given length and first header.
 *
 * @param arena The arena to allocate the header list from.
 * @param capacity The initial capacity of the header list.
 * @param first The first header in the list (NULL if none).
 */
httiny_header_list_t *header_list_new(httiny_arena_t *arena, u64 capacity,
                                      string_nullable *first);

/*
 * @brief Gets the header key for a given header.
 *
 * @param header The header to get the key for.
 *
 * @return The header key.
 */
HTTINY_HEADER_KEY get_header_key(httiny_header_t *header);

/*
 * @brief Gets the header name for a given header key.
 *
 * @param arena The arena to allocate the header name from.
 * @param key The header key to get the name for.
 *
 * @return The header name.
 */
httiny_header_name_t *get_header_name(httiny_arena_t *arena,
                                      HTTINY_HEADER_KEY key);

/*
 * @brief Gets the header name for a given header.
 *
 * @param arena The arena to allocate the header name from.
 * @param header The header to get the name for.
 *
 * @return The header name.
 */
httiny_header_name_t *get_header_name_from_header(httiny_arena_t *arena,
                                                  httiny_header_t *header);

/*
 * @brief Gets the header from the given header list.
 *
 * @param arena The arena to allocate when finding the header.
 * @param header_list The header list to get the header from.
 * @param key The header key to get.
 * @param cursor The cursor to use. (set to -1 if you wanna search from the
 * beginning)
 *
 * @return The header.
 */
httiny_header_t *get_header_from_list(httiny_arena_t *arena,
                                      httiny_header_list_t *header_list,
                                      HTTINY_HEADER_KEY key, i64 *cursor);

/*
 * @brief Adds a header to the given header list.
 *
 * @param arena The arena to allocate the header from.
 * @param header_list The header list to add to.
 * @param header_key The header key to add.
 * @param original_key_name The original header name.
 * @param val The header value.
 *
 * @return The index of the header in the list.
 */
u64 add_header(httiny_arena_t *arena, httiny_header_list_t **header_list,
               HTTINY_HEADER_KEY header_key, string_nullable *original_key_name,
               httiny_header_value_t *val);

#endif // !HTTINY_HEADER_H
