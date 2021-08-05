.PHONY = all clean
cc = gcc
libs = -lncurses -lm -lform
flags = -g
# macros = -D_GLFW_X11 -D_GLFW_GLX -D_GLFW_USE_OPENGL

SRCS = $(wildcard src/*.c)
BINS = $(SRCS:src/%.c=%)
LINK = $(BINS:%=build/%.o)

all: ${BINS}
	@echo -n linking...
	@$(cc) $(libs) $(flags) $(LINK)
	@echo " done"

%: src/%.c
	@echo -n compiling $<
	@$(cc) -c $< -o build/$@.o
	@echo " done"

clean:
	@echo cleaning...
	@rm build/* a.out
