#ifndef ARENA_H
#define ARENA_H

#include <httiny/types.h>

#define KiB(num) ((u64)(num) << 10)
#define MiB(num) ((u64)(num) << 20)
#define GiB(num) ((u64)(num) << 30)
#define ALIGN_POW2(num, pow)                                                   \
  (((uint64_t)(num) + ((uint64_t)(pow) - 1)) & (~((uint64_t)(pow) - 1)))

#define MIN(num1, num2) ((num1 < num2) ? num1 : num2)
#define MAX(num1, num2) ((num1 > num2) ? num1 : num2)

#define HTTINY_ARENA_BASE_POS (sizeof(httiny_arena_t))
#define HTTINY_ARENA_ALIGNMENT (sizeof(void *))

/*
 * @brief Creates a new arena with the given reserve and commit sizes.
 *
 * @param reserve_size The size of the heap to reserve for the process.
 * @param commit_size The size of the heap to commit for the process.
 *
 * @return A pointer to the new arena.
 */
httiny_arena_t *arena_new(u64 reserve_size, u64 commit_size);

/*
 * @brief Destroys the given arena through freeing the mmap.
 *
 * @param arena The arena to destroy.
 */
void arena_destroy(httiny_arena_t *arena);

/*
 * @brief Pushes a new chunk of memory onto the given arena.
 *
 * @param arena The arena to push the memory onto.
 * @param size The size of the memory to push.
 *
 * @return A pointer to the newly pushed memory.
 */
void *arena_push(httiny_arena_t *arena, u64 size);

/*
 * @brief Pops the given chunk of memory from the given arena.
 *
 * @param arena The arena to pop the memory from.
 * @param size The size of the memory to pop.
 */
void arena_pop(httiny_arena_t *arena, u64 size);

/*
 * @brief Pops to the given position in the arena.
 *
 * @param arena The arena to pop the memory from.
 * @param pos The position to pop to.
 */
void arena_pop_to(httiny_arena_t *arena, u64 pos);

/*
 * @brief Clear the committed heap for the arena.
 *
 * @param arena The arena to clear.
 */
void arena_clear(httiny_arena_t *arena);

/*
 * @brief Makes a small scratch arena for temporary heap allocations.
 *
 * @param arena The arena to make the scratch arena for.
 *
 * @return The scratch arena.
 */
httiny_scratch_arena_t arena_scratch_new(httiny_arena_t *arena);

/*
 * @breif Destroys the scratch arena, by just popping to the start position
 *
 * @param arena_scratch The scratch arena to destroy.
 */
void arena_scratch_destroy(httiny_scratch_arena_t arena_scratch);

/*
 * @brief Gets the thread arena, creating it if it doesn't exist.
 *
 * @param reserve_size The size of the heap to reserve for the process.
 * @param commit_size The size of the heap to commit for the process.
 *
 * return A pointer to the thread arena.
 */
httiny_arena_t *get_thread_arena(u64 reserve_size, u64 commit_size);

#endif // !ARENA_H
