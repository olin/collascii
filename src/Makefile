CC=gcc
CFLAGS+=-Wall
# LDFLAGS

# add debug annotations, turn off optimizations, and #define DEBUG
# use in bash with `DEBUG=1 make <foo>`
# use in fish with `env DEBUG=1 make <foo>`
ifdef DEBUG
CFLAGS+=-g -O0 -DDEBUG
endif

# link ncurses library
# foo: LDFLAGS+=-lncurses

# run test files (any file ending in _test.c)
test: $(patsubst %.c, .run-%.c, $(wildcard *_test.c))
# run a single file
.run-%.c: %
	./$<
# add flags for minunit libraries, works with `make test_foo` too
%_test: LDFLAGS+=-lrt -lm
%_test: %_test.c minunit.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	-rm *.o *_test