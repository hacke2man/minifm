.PHONY = all clean
cc = gcc
libs = -lm -lgit2
flags = -g

SRCS = $(wildcard src/*.c)
BINS = $(SRCS:src/%.c=%)
LINK = $(BINS:%=build/%.o)

all: checkenv ${BINS}
	$(cc) $(libs) $(flags) $(LINK)

checkenv:
	[ -d build ] || mkdir build

%: src/%.c
	$(cc) -c $< -o build/$@.o

clean:
	[ -f build ] && rm -rf build && mkdir build
	[ -f a.out ] && rm a.out

install: all
	cp a.out /usr/local/bin/minifm
