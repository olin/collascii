CC=gcc
CFLAGS+=-Wall

# add debug annotations, turn off optimizations, and #define DEBUG
# use in bash with `DEBUG=1 make <foo>`
# use in fish with `env DEBUG=1 make <foo>`
ifdef DEBUG
CFLAGS+=-g -O0 -DDEBUG
endif

# run test files
# add test file by adding to the prerequisites in test:
test: run-test_foo
# run a single test file
run-test_%: test_%
	./$<
	rm $<
# add flags for minunit libraries, works with `make test_foo` too
test_%: CFLAGS+=-lrt -lm
test_%: test_%.c minunit.h
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm *.o