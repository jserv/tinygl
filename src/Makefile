include ../config.mk

OBJS= api.o list.o vertex.o init.o matrix.o texture.o \
      misc.o clear.o light.o clip.o select.o get.o \
      zbuffer.o zline.o ztriangle.o \
      zmath.o image_util.o msghandling.o \
      arrays.o specbuf.o memory.o ztext.o zraster.o zpostprocess.o

deps := $(OBJS:%.o=.%.o.d)

CFLAGS += -I../include
LIB = ../lib/libTinyGL.a

all: $(LIB)

$(LIB): $(OBJS)
	rm -f $(LIB)
	ar rcs $(LIB) $(OBJS)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

clean:
	$(RM) $(OBJS) $(deps) $(LIB)

-include $(deps)
