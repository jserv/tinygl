include ../../config.mk

ALL_T= gears texture model helloworld menu game

CFLAGS += -I../../include
LDFLAGS += ../../lib/libTinyGL.a -lm

SDL_CONFIG ?= sdl-config
CFLAGS += $(shell $(SDL_CONFIG) --cflags)
LDFLAGS += $(shell $(SDL_CONFIG) --libs)

all: $(ALL_T)

$(ALL_T): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(ALL_T)
