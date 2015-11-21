/** @file debug_assert.h
 *
 *  @brief Useful utilities which can remain for now
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef H_DEBUG_ASSERT
#define H_DEBUG_ASSERT

#include <debug_print.h>
#include <page.h>
#include <vm.h>

/** @brief Define to allow assertions for debugging */
#define DEBUG_ASSERTS

#ifdef DEBUG_ASSERTS
// assert macros courtesy of
// http://stackoverflow.com/questions/3385515/static-assert-in-c
/** @brief Static assertion of the given condition */
#define STATIC_ASSERT(COND, MSG) \
    typedef char static_assertion_##MSG[(!!(COND)) * 2 - 1]
/** @brief Sub-macro 3 for compile time assertions */
#define COMPILE_TIME_ASSERT3(X, L) \
    STATIC_ASSERT(X, static_assertion_at_line_##L)
/** @brief Sub-macro 2 for compile time assertions */
#define COMPILE_TIME_ASSERT2(X, L) COMPILE_TIME_ASSERT3(X, L)
/** @brief Compile time assertion of the given condition */
#define COMPILE_TIME_ASSERT(X) COMPILE_TIME_ASSERT2(X, __LINE__)

/** @brief Macro to assert that a frame is page aligned */
#define ASSERT_PAGE_ALIGNED(frame)                                        \
    do {                                                                  \
        unsigned int aligned = ((unsigned int)(frame)) >> PAGE_SHIFT;     \
        if (aligned << PAGE_SHIFT != (unsigned int)(frame)) {             \
            DPRINTF("WARN: physical frame misaligned at %d", __LINE__); \
        }                                                                 \
    } while (0)

#else

/** @brief Blank macro for non-debugging state */
#define COMPILE_TIME_ASSERT(x)
/** @brief Blank macro for non-debugging state */
#define ASSERT_ALIGNED(x)

#endif

#endif // H_DEBUG_ASSERT
