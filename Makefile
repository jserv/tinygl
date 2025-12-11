# Configuration options:
#   ENABLE_SDL2=0/1 - Toggle SDL2 examples (auto-detected)
#   DEBUG=1         - Debug build with symbols
#   SANITIZE=1      - Enable address/undefined sanitizers
#   V=1             - Verbose output

# Default target (must be before includes that define targets)
.DEFAULT_GOAL := all

# Include modular build components
include mk/common.mk
include mk/src.mk
include mk/examples.mk
include mk/test.mk

# Main targets
all: $(LIB)
	@echo Done!

# Build SDL examples
sdl_examples: $(LIB) examples/stb_image.h $(SDL_EXAMPLES)

# Build raw examples
raw_examples: $(LIB) examples/stb_image_write.h $(RAW_EXAMPLES)


# Clean all build artifacts
clean: clean-lib clean-examples
	$(VECHO) "  CLEAN"
	$(Q)$(RM) -r $(LIB_DIR)

# Download external dependencies
examples/stb_image.h:
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
examples/stb_image_write.h:
	curl -o $@ https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Clean everything including generated assets
distclean: clean
	$(VECHO) "  DISTCLEAN"
	$(Q)$(RM) examples/stb_image.h examples/stb_image_write.h

# Show configuration
config:
	@echo "TinyGL build configuration:"
	@echo "  Platform:    $(UNAME_S)"
	@echo "  Compiler:    $(CC)"
	@echo "  CFLAGS:      $(CFLAGS)"
	@echo "  LDFLAGS:     $(LDFLAGS)"
	@echo "  SDL2:        $(if $(filter 1,$(ENABLE_SDL2)),enabled,disabled)"
	@echo "  Examples:    $(ALL_EXAMPLE_NAMES)"

.PHONY: all sdl_examples raw_examples check check-generate clean distclean config $(ALL_EXAMPLES)
