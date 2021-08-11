.PHONY = all clean
cc = gcc
libs = -lncurses -lm -lform
flags = -g
# macros = -D_GLFW_X11 -D_GLFW_GLX -D_GLFW_USE_OPENGL

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
	cp a.out /bin/mfm
