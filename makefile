.PHONY = all clean
cc = gcc
libs = -lm -lgit2
flags = -g

SRCS = $(wildcard src/*.c)
BINS = $(SRCS:src/%.c=%)
LINK = $(BINS:%=build/%.o)

default: checkenv compile finish

compile: ${BINS}
	$(cc) $(libs) $(flags) $(LINK)

checkenv:
	@[ -d build ] || mkdir build

finish:
	@if [ -n "$(SUDO_USER)" ]; then\
		chown $(SUDO_USER) -R build a.out;\
	fi

%: src/%.c
	$(cc) -c $< -o build/$@.o

clean:
	@if [ -d "build" ]; then\
		rm -rf build;\
	fi
	@if [ -f "a.out" ]; then\
		rm a.out;\
	fi

install: default
	cp a.out /usr/local/bin/minifm
