#ifndef HTTINY_ASSERT_H
#define HTTINY_ASSERT_H

#define GNU_SOURCE
#include <stdbool.h>

/*
 * @brief Exits the program with an error message.
 *
 * @param expr_str The expression that failed.
 * @param executable The executable name.
 * @param file The file name.
 * @param line The line number.
 * @param function The function name.
 *
 * @note This function doesn't return and exists with status 1;
 */
_Noreturn void __httiny_assert_fail(const char *expr_str,
                                    const char *executable, const char *file,
                                    int line, const char *function);

// External string for the program name (GNU extension)
extern char *program_invocation_short_name;

/*
 * @brief Asserts that the given expression is true.
 *
 * @param expr The expression to assert.
 *
 * @note if the expression is false, it will call
 * __httiny_assert_fail
 */
#define httiny_assert(expr)                                                    \
  ((expr) ? (void)0                                                            \
          : __httiny_assert_fail(#expr, program_invocation_short_name,         \
                                 __FILE__, __LINE__, __func__))

#endif // !HTTINY_ASSERT_H
