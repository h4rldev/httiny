#include <stdio.h>
#include <stdlib.h>

#include <httiny/assert.h>

_Noreturn void __httiny_assert_fail(const char *expr_str,
                                    const char *executable, const char *file,
                                    int line, const char *function) {
  printf("%s: %s:%i: %s: Assertion `%s` failed.\n", executable, file, line,
         function, expr_str);
  exit(1);
}
