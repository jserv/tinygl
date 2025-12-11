# mk/src.mk - TinyGL library compilation rules

# Collect all source files from src directory
TINYGL_SRC := $(wildcard $(SRC_DIR)/*.c)
TINYGL_OBJS := $(TINYGL_SRC:.c=.o)
TINYGL_DEPS := $(TINYGL_SRC:.c=.d)

# Define the library name and path
LIBNAME := libTinyGL.a
LIB_DIR := lib
LIB := $(LIB_DIR)/$(LIBNAME)

# Create dependency files for each object
deps := $(TINYGL_OBJS:.o=.d)

# Rule to build the library
$(LIB): $(TINYGL_OBJS)
	@mkdir -p $(LIB_DIR)
	$(VECHO) "  AR\t$@"
	$(Q)rm -f $(LIB)
	$(Q)ar rcs $(LIB) $(TINYGL_OBJS)

# Compilation rule for individual objects
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(VECHO) "  CC\t$@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(INCLUDES) -c -MMD -MF $(@:.o=.d) $<

# Include all dependency files
-include $(deps)

# Clean up build artifacts for the library
clean-lib:
	$(VECHO) "  CLEAN LIB"
	$(Q)$(RM) $(TINYGL_OBJS) $(deps) $(LIB)

.PHONY: clean-lib