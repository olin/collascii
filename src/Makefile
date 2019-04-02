CC=gcc
CFLAGS+=-Wall

# add debug annotations, turn off optimizations, and #define DEBUG
# use in bash with `DEBUG=1 make <foo>`
# use in fish with `env DEBUG=1 make <foo>`
ifdef DEBUG
CFLAGS+=-g -O0 -DDEBUG
endif

test_%: CFLAGS+=-lrt -lm
test_%: 

clean:
	rm *.o *.out