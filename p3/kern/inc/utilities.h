/** @file utilities.h
 *
 *  @brief Useful utilities which can remain for now
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef UTILITIES_H_
#define UTILITIES_H_
#include <simics.h>
#include <page.h>
#include <vm.h>


#define DEBUG

#ifdef DEBUG
// assert macros courtesy of
// http://stackoverflow.com/questions/3385515/static-assert-in-c
#define STATIC_ASSERT(COND, MSG) \
    typedef char static_assertion_##MSG[(!!(COND)) * 2 - 1]
#define COMPILE_TIME_ASSERT3(X, L) \
    STATIC_ASSERT(X, static_assertion_at_line_##L)
#define COMPILE_TIME_ASSERT2(X, L) COMPILE_TIME_ASSERT3(X, L)
#define COMPILE_TIME_ASSERT(X) COMPILE_TIME_ASSERT2(X, __LINE__)

#define ASSERT_PAGE_ALIGNED(frame)                                      \
    do {                                                                \
        unsigned int aligned = ((unsigned int)(frame)) >> PAGE_SHIFT;   \
        if (aligned << PAGE_SHIFT != (unsigned int)(frame)) {           \
            lprintf("WARN: physical frame misaligned at %d", __LINE__); \
        }                                                               \
    } while (0)

#else

#define COMPILE_TIME_ASSERT(x)
#define ASSERT_ALIGNED(x)


#endif

#endif //UTILITIES_H_
