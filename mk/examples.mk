# mk/examples.mk - Example build rules for TinyGL

# Explicitly listed examples (add new examples here)
RAW_EXAMPLES_ALL := gears
SDL_EXAMPLES_ALL := hello texture game model gears_sdl

# Detect whether SDL2 is available
ifeq ($(origin ENABLE_SDL2), undefined)
    SDL2_CONFIG := $(call has, sdl2-config)
    ifeq ($(SDL2_CONFIG), 1)
        ENABLE_SDL2 := 1
        SDL2_DETECT := sdl2-config
    else
        PKG_CONFIG := $(call has, pkg-config)
        ifeq ($(PKG_CONFIG), 1)
            PKGCONFIG_SDL2 := $(shell pkg-config --exists sdl2 2>/dev/null && echo 1)
        else
            PKGCONFIG_SDL2 :=
        endif
        ifeq ($(PKGCONFIG_SDL2), 1)
            ENABLE_SDL2 := 1
            SDL2_DETECT := pkg-config
        else
            ENABLE_SDL2 := 0
            $(warning SDL2 not found via sdl2-config or pkg-config, SDL2 examples disabled)
        endif
    endif
endif

# Configure SDL2 flags when enabled
ifeq ($(ENABLE_SDL2), 1)
    ifeq ($(SDL2_DETECT), pkg-config)
        SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
        SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)
    else
        SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
        SDL_LIBS := $(shell sdl2-config --libs 2>/dev/null)
    endif
else
    SDL_CFLAGS :=
    SDL_LIBS :=
endif

# Enable examples based on availability
ifeq ($(ENABLE_SDL2), 1)
    SDL_EXAMPLES := $(SDL_EXAMPLES_ALL)
else
    SDL_EXAMPLES :=
endif

# Raw examples don't require SDL
RAW_EXAMPLE_NAMES := $(RAW_EXAMPLES_ALL)
RAW_EXAMPLES := $(RAW_EXAMPLE_NAMES)

# All examples to build
ALL_EXAMPLES := $(SDL_EXAMPLES) $(RAW_EXAMPLES)
# Human-readable list of enabled examples
ALL_EXAMPLE_NAMES := $(if $(ENABLE_SDL2),$(SDL_EXAMPLES_ALL),) $(RAW_EXAMPLE_NAMES)

# All possible examples (for clean target)
ALL_EXAMPLES_CLEAN := $(SDL_EXAMPLES_ALL) $(RAW_EXAMPLES_ALL)

# Auto-generate source mappings: <name>_SRC
$(foreach ex,$(RAW_EXAMPLES_ALL),$(eval $(ex)_SRC := $(EXAMPLES_DIR)/raw/$(ex).c))
$(foreach ex,$(SDL_EXAMPLES_ALL),$(eval $(ex)_SRC := $(EXAMPLES_DIR)/sdl/$(ex).c))
gears_sdl_SRC := $(EXAMPLES_DIR)/sdl/gears.c

# Extra dependencies (header-only libs, assets, etc.)
gears_DEPS := $(INCLUDE_DIR)/zbuffer.h
game_DEPS := $(EXAMPLES_DIR)/3dmath.h $(EXAMPLES_DIR)/tobjparse.h
model_DEPS := $(EXAMPLES_DIR)/3dmath.h $(EXAMPLES_DIR)/tobjparse.h

# Per-example CFLAGS (if needed)
# gear_CFLAGS := -DGEAR_CUSTOM_FLAG

# Per-example LDFLAGS (if needed)
# model_LDFLAGS := -lpng

# Rule to ensure library is built before examples
EXAMPLES_PREREQ := $(LIB)

# Include path specifically for examples (includes the examples directory for stb headers)
EXAMPLES_INCLUDES := -I$(INCLUDE_DIR) -I$(EXAMPLES_DIR)

# Separate include path for SDL examples that need headers from examples/sdl subdirectory
SDL_INCLUDES := -I$(INCLUDE_DIR) -I$(EXAMPLES_DIR) -I$(EXAMPLES_DIR)/sdl

# Pattern rule for SDL examples
define sdl_example_rule
$(1): $$(EXAMPLES_PREREQ) $$($(1)_SRC) $$(LIB) $$($(1)_DEPS)
	$$(VECHO) "  CC\t$$@"
	$$(Q)$$(CC) $$(CFLAGS) $$(SDL_INCLUDES) $$(SDL_CFLAGS) $$(filter %.c,$$^) -o $$@ $$(LIB) $$(SDL_LIBS) $$(LIBS) $$(LDFLAGS) $$($(1)_LDFLAGS)
endef

# Pattern rule for raw examples
define raw_example_rule
$(1): $$(EXAMPLES_PREREQ) $$($(1)_SRC) $$(LIB) $$($(1)_DEPS)
	$$(VECHO) "  CC\t$$@"
	$$(Q)$$(CC) $$(CFLAGS) $$(EXAMPLES_INCLUDES) $$(filter %.c,$$^) -o $$@ $$(LIB) $$(LIBS) $$(LDFLAGS) $$($(1)_LDFLAGS)
endef

# Generate build rules for all examples
$(foreach ex,$(SDL_EXAMPLES),$(eval $(call sdl_example_rule,$(ex))))
$(foreach ex,$(RAW_EXAMPLE_NAMES),$(eval $(call raw_example_rule,$(ex))))

# Build aliases: build-<example>
$(foreach ex,$(SDL_EXAMPLES),$(eval build-$(ex): $(ex)))
$(foreach ex,$(RAW_EXAMPLE_NAMES),$(eval build-$(ex): $(ex)))

# Run targets: run-<example>
define run_sdl_rule
run-$(1): $(1)
	$$(VECHO) "  RUN\t$(1)"
	$$(Q)./$(1)
endef
define run_raw_rule
run-$(1): $(1)
	$$(VECHO) "  RUN\t$(1)"
	$$(Q)./$(1)
endef
$(foreach ex,$(SDL_EXAMPLES),$(eval $(call run_sdl_rule,$(ex))))
$(foreach ex,$(RAW_EXAMPLE_NAMES),$(eval $(call run_raw_rule,$(ex))))

# Clean examples
clean-examples:
	$(VECHO) "  CLEAN EXAMPLES"
	$(Q)$(RM) $(RAW_EXAMPLES_ALL) $(SDL_EXAMPLES_ALL)

# Phony declarations for build/run targets
BUILD_TARGETS := $(addprefix build-,$(SDL_EXAMPLES) $(RAW_EXAMPLE_NAMES))
RUN_TARGETS := $(addprefix run-,$(SDL_EXAMPLES) $(RAW_EXAMPLE_NAMES))

.PHONY: clean-examples $(BUILD_TARGETS) $(RUN_TARGETS)
