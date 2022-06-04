include ../../config.mk

ALL_T= gears

CFLAGS += -I../../include -I..
LDFLAGS += ../../lib/libTinyGL.a -lm

all: $(ALL_T)

OBJS := $(ALL_T:%=%.o)
deps := $(OBJS:%.o=.%.o.d)

$(ALL_T): %: %.o
	$(CC) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

clean:
	$(RM) $(ALL_T) $(OBJS) $(deps)
	$(RM) render.png

-include $(deps)
