#include <stdio.h>
#include <stdlib.h>

#include <httiny/assert.h>

/*
 * @brief A custom assert that doesn't use abort.
 *
 * @param expr_str The expression string.
 * @param executable The executable name.
 * @param file The file name.
 * @param line The line number.
 * @param function The function name.
 *
 * @note This depends on a GNU extension.
 */
_Noreturn void __httiny_assert(const char *expr_str, const char *executable,
                               const char *file, int line,
                               const char *function) {
  printf("%s: %s:%i: %s: Assertion `%s` failed.\n", executable, file, line,
         function, expr_str);
  exit(1);
}
