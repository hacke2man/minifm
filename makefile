.PHONY = all clean
cc = gcc
libs = -lm -lgit2
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
	[ -d /home/${SUDO_USER}/.local/share/mfm ] || mkdir "/home/${SUDO_USER}/.local/share/mfm"
	cp a.out /usr/local/bin/minifm
