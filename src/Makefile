CC=gcc
CFLAGS+=-Wall -std=c99

# add debug annotations, turn off optimizations, and #define DEBUG
# use in bash with `DEBUG=1 make <foo>`
# use in fish with `env DEBUG=1 make <foo>`
ifdef DEBUG
CFLAGS+=-g -O0 -DDEBUG
endif

## EXAMPLES

# link ncurses library
# foo: LDLIBS+=-lncurses

# require library canvas.c for foo
# foo.out: canvas.o

## FILE-SPECIFIC MODIFICATIONS

frontend.out: LDLIBS +=-lncurses
frontend.out: cursor.o

client.out: network.o canvas.o
server.out: network.o canvas.o

## PATTERNS

# Generate .out files for easier .gitignore. Based on the builtin make rule
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
%_test: LDLIBS+=-lrt -lm
# require foo.c, foo_test, and minunit.h for foo_test
%_test: %.o %_test.c minunit.h
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-rm *.o *_test *.out
