LIBNAME = libTinyGL.a
LIB = lib/$(LIBNAME)

all: $(LIB)
	@echo Done!

$(LIB):
	$(MAKE) -C src

sdl_examples: $(LIB) examples/stb_image.h
	@echo "These demos require SDL2 to build."
	$(MAKE) -C examples/sdl

raw_examples: $(LIB) examples/stb_image_write.h
	@echo "Building the raw demos."
	$(MAKE) -C examples/raw
	
clean:
	$(MAKE) -C src clean
	$(MAKE) -C examples/raw clean
	$(MAKE) -C examples/sdl clean
	$(RM) $(LIB)

examples/stb_image.h:
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
examples/stb_image_write.h:
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

distclean: clean
	$(RM) examples/stb_image.h examples/stb_image_write.h
