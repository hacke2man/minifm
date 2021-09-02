.PHONY = all clean
cc = gcc
libs = -lm
flags = -g

SRCS = $(wildcard src/*.c)
BINS = $(SRCS:src/%.c=%)
LINK = $(BINS:%=build/%.o)

all: ${BINS}
	$(cc) $(libs) $(flags) $(LINK)

%: src/%.c
	$(cc) -c $< -o build/$@.o

clean:
	rm build/* a.out

install: all
	[ -d ~/.local/share/mfm ] || mkdir ~/.local/share/mfm
	cp a.out /usr/local/bin/minifm
