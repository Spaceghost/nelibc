/* Standalone allocator replacement: this program defines malloc, realloc and
   free (and, with -DREPLACE_ALIGNED, aligned_alloc) the way an application
   linked statically against the standalone archive may. Every pointer the
   library hands back for the caller to free must come from this arena, the
   library must free its own FILE objects through this free, and it must never
   pass this free a pointer the arena did not produce. calloc is left to the
   library so its replaced-malloc path is exercised. */
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wordexp.h>

#define ARENA_SIZE (4u << 20)
#define BLOCK_MAGIC 0x6d616c6cu

struct block {
  size_t size;
  unsigned magic;
  unsigned freed;
};

static _Alignas(64) unsigned char arena[ARENA_SIZE];
static size_t arena_used;
static unsigned long allocations, frees;
static int failures;

static void die(const char *message) {
  write(2, "malloc-replace: ", 16);
  write(2, message, strlen(message));
  write(2, "\n", 1);
  _exit(1);
}

static void *arena_take(size_t alignment, size_t size) {
  size_t start = (arena_used + sizeof(struct block) + alignment - 1) & ~(alignment - 1);
  if (size > ARENA_SIZE || start > ARENA_SIZE - size) die("arena exhausted");
  struct block *header = (struct block *)(arena + start - sizeof(struct block));
  header->size = size;
  header->magic = BLOCK_MAGIC;
  header->freed = 0;
  arena_used = start + size;
  memset(arena + start, 0xa5, size); /* calloc must clear it itself */
  allocations++;
  return arena + start;
}

static int in_arena(const void *pointer) {
  uintptr_t address = (uintptr_t)pointer;
  return address >= (uintptr_t)arena + sizeof(struct block) && address < (uintptr_t)arena + ARENA_SIZE;
}

static struct block *header_of(void *pointer) {
  if (!in_arena(pointer)) die("free/realloc of a pointer this allocator did not return");
  struct block *header = (struct block *)((unsigned char *)pointer - sizeof(struct block));
  if (header->magic != BLOCK_MAGIC) die("free/realloc of a pointer without a block header");
  if (header->freed) die("double free");
  return header;
}

void *malloc(size_t size) { return arena_take(16, size ? size : 1); }

void free(void *pointer) {
  if (pointer == 0) return;
  header_of(pointer)->freed = 1;
  frees++;
}

void *realloc(void *pointer, size_t size) {
  if (pointer == 0) return malloc(size);
  struct block *header = header_of(pointer);
  void *replacement = malloc(size);
  memcpy(replacement, pointer, header->size < size ? header->size : size);
  free(pointer);
  return replacement;
}

#ifdef REPLACE_ALIGNED
static unsigned long aligned_calls;
void *aligned_alloc(size_t alignment, size_t size) {
  aligned_calls++;
  return arena_take(alignment < 16 ? 16 : alignment, size ? size : 1);
}
#endif

static void check(int condition, const char *what) {
  if (!condition) {
    fputs("malloc-replace: FAIL ", stderr);
    fputs(what, stderr);
    fputs("\n", stderr);
    failures++;
  }
}

static void check_owned(const void *pointer, const char *what) {
  check(pointer != 0 && in_arena(pointer), what);
}

int main(void) {
  char *text = strdup("replaced");
  check_owned(text, "strdup result comes from the application malloc");
  check(text && strcmp(text, "replaced") == 0, "strdup copies");
  free(text);
  text = strndup("replacement", 7);
  check_owned(text, "strndup result comes from the application malloc");
  free(text);

  unsigned char *zeroed = calloc(16, 4);
  check_owned(zeroed, "library calloc uses the application malloc");
  int all_zero = zeroed != 0;
  for (int i = 0; zeroed && i < 64; i++) all_zero &= zeroed[i] == 0;
  check(all_zero, "library calloc clears memory from the application malloc");
  free(zeroed);
  errno = 0;
  check(calloc(SIZE_MAX / 2, 4) == 0 && errno == ENOMEM, "calloc overflow fails with ENOMEM");

  int *array = reallocarray(0, 8, sizeof(int));
  check_owned(array, "reallocarray uses the application realloc");
  free(array);

  char *formatted = 0;
  check(asprintf(&formatted, "%s-%d", "fmt", 42) == 6, "asprintf length");
  check_owned(formatted, "asprintf result comes from the application malloc");
  check(formatted && strcmp(formatted, "fmt-42") == 0, "asprintf text");
  free(formatted);

  char *scanned = 0;
  check(sscanf("token rest", "%ms", &scanned) == 1, "sscanf %ms converts");
  check_owned(scanned, "sscanf %ms result comes from the application malloc");
  check(scanned && strcmp(scanned, "token") == 0, "sscanf %ms text");
  free(scanned);

  FILE *file = tmpfile();
  check_owned(file, "tmpfile FILE comes from the application malloc");
  if (file) {
    for (int i = 0; i < 300; i++) fputc('x', file);
    fputs("\nshort\n", file);
    rewind(file);
    char *line = 0;
    size_t capacity = 0;
    check(getline(&line, &capacity, file) == 301, "getline reads a long line");
    check_owned(line, "getline buffer comes from the application realloc");
    check(getdelim(&line, &capacity, '\n', file) == 6, "getdelim reuses the buffer");
    free(line);
    struct block *file_block = (struct block *)((unsigned char *)file - sizeof(struct block));
    unsigned long frees_before = frees;
    check(fclose(file) == 0, "fclose");
    check(file_block->freed && frees == frees_before + 1,
          "fclose returns the FILE to the application free");
  }

  char *stream_buffer = 0;
  size_t stream_size = 0;
  FILE *stream = open_memstream(&stream_buffer, &stream_size);
  check_owned(stream, "open_memstream FILE comes from the application malloc");
  if (stream) {
    for (int i = 0; i < 1000; i++) fputc('a' + i % 26, stream);
    check(fclose(stream) == 0, "open_memstream fclose");
    check(stream_size == 1000, "open_memstream size");
    check_owned(stream_buffer, "open_memstream buffer comes from the application allocator");
    free(stream_buffer);
  }

  wchar_t *wide_buffer = 0;
  size_t wide_size = 0;
  FILE *wide = open_wmemstream(&wide_buffer, &wide_size);
  check_owned(wide, "open_wmemstream FILE comes from the application malloc");
  if (wide) {
    for (int i = 0; i < 100; i++) fputwc(L'w', wide);
    check(fclose(wide) == 0, "open_wmemstream fclose");
    check(wide_size == 100, "open_wmemstream size");
    check_owned(wide_buffer, "open_wmemstream buffer comes from the application allocator");
    free(wide_buffer);
  }

  char *resolved = realpath(".", 0);
  check_owned(resolved, "realpath(NULL) result comes from the application malloc");
  free(resolved);
  /* getcwd(NULL, ...) is not supported (POSIX leaves it unspecified);
     get_current_dir_name is the allocating form. */
  char *directory = get_current_dir_name();
  check_owned(directory, "get_current_dir_name result comes from the application malloc");
  free(directory);

  struct dirent **entries = 0;
  int count = scandir("/", &entries, 0, alphasort);
  check(count > 0, "scandir lists /");
  check_owned(entries, "scandir list comes from the application malloc");
  for (int i = 0; i < count; i++) {
    check_owned(entries[i], "scandir entry comes from the application malloc");
    free(entries[i]);
  }
  free(entries);

  wordexp_t words;
  check(wordexp("one two", &words, 0) == 0 && words.we_wordc == 2, "wordexp splits");
  check_owned(words.we_wordv, "wordexp vector comes from the application malloc");
  wordfree(&words);

  void *aligned = 0;
#ifdef REPLACE_ALIGNED
  aligned = aligned_alloc(64, 64);
  check_owned(aligned, "application aligned_alloc");
  free(aligned);
  aligned = 0;
  check(posix_memalign(&aligned, 64, 100) == 0, "posix_memalign uses the application aligned_alloc");
  check_owned(aligned, "posix_memalign result comes from the application aligned_alloc");
  check(((uintptr_t)aligned & 63) == 0, "posix_memalign alignment");
  free(aligned);
  aligned = memalign(32, 10);
  check_owned(aligned, "memalign result comes from the application aligned_alloc");
  free(aligned);
  check(aligned_calls == 3, "posix_memalign and memalign each called aligned_alloc once");
  check(posix_memalign(&aligned, 4, 16) == 22, "posix_memalign EINVAL below pointer alignment");
#else
  /* musl's rule: malloc replaced, aligned_alloc not -> the memalign family
     fails with ENOMEM instead of mixing allocators. */
  errno = 0;
  check(aligned_alloc(16, 32) == 0 && errno == ENOMEM, "aligned_alloc fails with ENOMEM");
  errno = 0;
  check(aligned_alloc(3, 3) == 0 && errno == EINVAL, "aligned_alloc still rejects a bad alignment");
  errno = 0;
  check(memalign(16, 32) == 0 && errno == ENOMEM, "memalign fails with ENOMEM");
  errno = 7;
  check(posix_memalign(&aligned, 16, 32) == ENOMEM && aligned == 0, "posix_memalign returns ENOMEM");
  check(errno == 7, "posix_memalign leaves errno alone");
  check(posix_memalign(&aligned, 3, 32) == EINVAL, "posix_memalign still rejects a bad alignment");
#endif
  void *probe = malloc(24);
  check(malloc_usable_size(probe) == 0, "malloc_usable_size is unsupported (0) on a replaced allocator");
  free(probe);

  check(allocations > 0 && frees > 0, "the library used the application allocator");
  if (failures) return 1;
#ifdef REPLACE_ALIGNED
  puts("malloc-replace smoke (malloc+aligned_alloc replaced): ok");
#else
  puts("malloc-replace smoke (malloc replaced): ok");
#endif
  return 0;
}
