#ifndef HTTINY_ASSERT_H
#define HTTINY_ASSERT_H

#define GNU_SOURCE
#include <stdbool.h>
_Noreturn void __httiny_assert(const char *expr_str, const char *executable,
                               const char *file, int line,
                               const char *function);

extern char *program_invocation_short_name;

#define httiny_assert(expr)                                                    \
  ((expr) ? (void)0                                                            \
          : __httiny_assert(#expr, program_invocation_short_name, __FILE__,    \
                            __LINE__, __func__))

#endif // !HTTINY_ASSERT_H
