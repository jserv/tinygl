include ../../config.mk

ALL_T= gears texture model hello game

CFLAGS += -I../../include -I..
LDFLAGS += ../../lib/libTinyGL.a -lm

SDL_CONFIG ?= sdl2-config
CFLAGS += $(shell $(SDL_CONFIG) --cflags)
LDFLAGS += $(shell $(SDL_CONFIG) --libs)

all: $(ALL_T)

OBJS := $(ALL_T:%=%.o)
deps := $(OBJS:%.o=.%.o.d)

$(ALL_T): %: %.o
	$(CC) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

clean:
	$(RM) $(ALL_T) $(OBJS) $(deps)

-include $(deps)
