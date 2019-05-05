#ifndef util_h
#define util_h

#include <stdio.h>

// printf to stderr
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
// log to stderr if DEBUG is defined
#define logd(...) eprintf(__VA_ARGS__)
#else
// do nothing
#define logd(...)
#endif

#endif
