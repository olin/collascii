CC=gcc
CFLAGS+=-Wall
# global includes
# LDFLAGS+=-lncurses

# add debug annotations, turn off optimizations, and #define DEBUG
# use in bash with `DEBUG=1 make <foo>`
# use in fish with `env DEBUG=1 make <foo>`
ifdef DEBUG
CFLAGS+=-g -O0 -DDEBUG
endif

# link ncurses library
# foo: LDFLAGS+=-lncurses

# Generate .out files for easier .gitignore. Based on the default make rule
# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html#Catalogue-of-Rules
%.out: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)

# **GNU Make only** run all test files (any file ending in _test.c)
# turns each `foo_test.c` into `.run-foo_test.c` with "Text Functions"
test: $(patsubst %.c, .run-%.c, $(wildcard *_test.c))

# run a single file
# given `.run-foo.c`, requires `foo.c` and runs `./foo`
.run-%.c: %
	./$<

# add flags for minunit libraries, works with `make test_foo` too
%_test: LDFLAGS+=-lrt -lm
%_test: %_test.c minunit.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# frontend.out: frontend.c frontend.h
# 	$(CC) frontend.c $(CFLAGS) $(LDLIBS) -o frontend.out
# frontend.out: cursor.o
frontend.out: LDLIBS +=-lncurses

clean:
	-rm *.o *_test *.out
