#ifndef util_h
#define util_h

#include <stdio.h>

// printf to stderr
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

// dummy function that does nothing
static void _log_unused(const int dummy, ...) { (void)dummy; }

// empty macro to trick compiler into thinking arguments are used
#define _LOG_UNUSED(...)         \
  do {                           \
    _log_unused(0, __VA_ARGS__); \
  } while (0)

#ifdef DEBUG
// log to stderr if DEBUG is defined
#define logd(...) eprintf(__VA_ARGS__)
#else
// print nothing if DEBUG isn't defined
#define logd(...) _LOG_UNUSED(__VA_ARGS__)
#endif

#endif
