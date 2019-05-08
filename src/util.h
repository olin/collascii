#ifndef util_h
#define util_h

#include <stdio.h>

#include <ncurses.h>

// printf to stderr
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// LOGGING
// techniques for preventing unused variables/function warnings based on zf_log:
// https://github.com/wonder-mice/zf_log/

/* Dummy function that does nothing with variadic args.
 *
 * Static b/c it shouldn't be used directly anywhere outside of this header.
 * Inline b/c it fixes the "defined but not used" warning, and it will be called
 * many times to do nothing.
 * https://stackoverflow.com/a/2765211
 * https://stackoverflow.com/q/2845748
 * https://stackoverflow.com/a/1932371
 * https://stackoverflow.com/q/7762731
 */
static inline void _log_unused(const int dummy, ...) { (void)dummy; }

#ifdef DEBUG
// log to stderr if DEBUG is defined
#define logd(...) eprintf(__VA_ARGS__)
#else
// empty macro to "trick" compiler into thinking the arguments to logd are used
// even when it is ignored, preventing "unused variable" warning
#define _LOG_UNUSED(...)         \
  do {                           \
    _log_unused(0, __VA_ARGS__); \
  } while (0)

// print nothing if DEBUG isn't defined
#define logd(...) _LOG_UNUSED(__VA_ARGS__)
#endif

// min/max macros
// from https://stackoverflow.com/questions/3437404/min-and-max-in-c

#define max(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

#define min(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

// ncurses-compatible CTRL+KEY definition
// NOTE: c should be lowercase
#define KEY_CTRL(c) ((c)&037)

#endif
