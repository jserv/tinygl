LIBNAME = libTinyGL.a
LIB = lib/$(LIBNAME)

all: $(LIB)
	@echo Done!

$(LIB):
	cd src && $(MAKE) && cd ..
	cp src/*.a ./lib/

sdl_examples: $(LIB) examples/stb_image.h
	@echo "These demos require SDL 1.2 to compile."
	$(MAKE) -C examples/sdl

raw_examples: $(LIB) examples/stb_image_write.h
	@echo "Building the RAW DEMOS. These do not require anything special on your system, so they should succeed."
	$(MAKE) -C examples/raw
	
clean:
	$(MAKE) -C src clean
	$(MAKE) -C examples/raw clean
	$(MAKE) -C examples/sdl clean
	$(RM) lib/*.a

examples/stb_image.h:
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
examples/stb_image_write.h:
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

distclean: clean
	$(RM) examples/stb_image.h examples/stb_image_write.h
