include ../../config.mk

ALL_T= gears t2i bigfont

CFLAGS += -I../../include
LDFLAGS += ../../lib/libTinyGL.a -lm

all: $(ALL_T)

$(ALL_T): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(ALL_T)
