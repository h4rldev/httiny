#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <threads.h>

#include <httiny/arena.h>
#include <httiny/assert.h>
#include <httiny/types.h>

// TODO: Make this arena allocator multi-platform.
static u32 get_page_size(void) { return (u32)sysconf(_SC_PAGESIZE); }

static void *mem_reserve(u64 size) {
  void *ret = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  httiny_assert(ret != MAP_FAILED && "Failed to reserve memory, mmap failed");
  return ret;
}

static bool mem_commit(void *ptr, u64 size) {
  return (i32)mprotect(ptr, size, PROT_READ | PROT_WRITE) == 0;
}

static bool mem_decommit(void *ptr, u64 size) {
  i32 ret = mprotect(ptr, size, PROT_NONE);
  httiny_assert(ret == 0 && "Failed to remove protection");
  return (i32)madvise(ptr, size, MADV_DONTNEED) == 0;
}

static bool mem_release(void *ptr, u64 size) {
  return (i32)munmap(ptr, size) == 0;
}

httiny_arena_t *arena_new(u64 reserve_size, u64 commit_size) {
  u32 page_size = get_page_size();

  reserve_size = ALIGN_POW2(reserve_size, page_size);
  commit_size = ALIGN_POW2(commit_size, page_size);

  httiny_assert(reserve_size > commit_size &&
                "Reserve size must be larger than commit size");

  httiny_arena_t *arena = mem_reserve(reserve_size);
  httiny_assert(mem_commit(arena, commit_size) == true &&
                "Failed to commit memory");

  arena->reserved = reserve_size;
  arena->committed = commit_size;
  arena->position = HTTINY_ARENA_BASE_POS;
  arena->commit_position = commit_size;

  return arena;
}

void arena_destroy(httiny_arena_t *arena) {
  mem_release(arena, arena->reserved);
}

void *arena_push(httiny_arena_t *arena, u64 size) {
  u64 pos_aligned = ALIGN_POW2(arena->position, HTTINY_ARENA_ALIGNMENT);
  u64 new_pos = pos_aligned + size;

  httiny_assert(new_pos < arena->reserved && "Arena full");

  if (new_pos > arena->commit_position) {
    u64 new_commit_pos = new_pos;
    new_commit_pos += arena->committed - 1;
    new_commit_pos -= new_commit_pos % arena->committed;
    new_commit_pos = MIN(new_commit_pos, arena->reserved);

    u8 *mem = (u8 *)arena + arena->commit_position;
    u64 commit_size = new_commit_pos - arena->commit_position;

    httiny_assert(mem_commit(mem, commit_size) == true &&
                  "Failed to commit memory");

    arena->commit_position = new_commit_pos;
  }

  arena->position = new_pos;
  u8 *out = (u8 *)arena + pos_aligned;
  memset(out, 0, size);

  return (void *)out;
}

void arena_pop(httiny_arena_t *arena, u64 size) {
  size = ALIGN_POW2(size, arena->position - HTTINY_ARENA_ALIGNMENT);
  arena->position -= size;
}

void arena_pop_to(httiny_arena_t *arena, u64 pos) {
  u64 size = pos < arena->position ? arena->position - pos : 0;
  arena_pop(arena, size);
}

void arena_clear(httiny_arena_t *arena) {
  arena_pop_to(arena, HTTINY_ARENA_BASE_POS);
  httiny_assert(mem_decommit(arena, arena->committed) == true &&
                "Failed to decommit");
  httiny_assert(mem_commit(arena, arena->committed) == true &&
                "Failed to recommit");
}

httiny_scratch_arena_t arena_scratch_new(httiny_arena_t *arena) {
  return (httiny_scratch_arena_t){
      .arena = arena,
      .start_pos = arena->position,
  };
}

void arena_scratch_destroy(httiny_scratch_arena_t arena_scratch) {
  arena_pop_to(arena_scratch.arena, arena_scratch.start_pos);
}

static pthread_key_t arena_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

static void destroy_thread_arena(void *ptr) {
  httiny_arena_t *arena = ptr;
  if (arena)
    arena_destroy(arena);
}

static void make_key(void) {
  pthread_key_create(&arena_key, destroy_thread_arena);
}

httiny_arena_t *get_thread_arena(u64 reserve_size, u64 commit_size) {
  pthread_once(&key_once, make_key);
  httiny_arena_t *arena = pthread_getspecific(arena_key);

  if (arena == NULL) {
    arena = arena_new(reserve_size, commit_size);
    pthread_setspecific(arena_key, arena);
  }

  return arena;
}
