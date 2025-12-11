# mk/test.mk - Test build rules for TinyGL

# Test dependencies
TEST_DEPS := $(LIB) examples/stb_image_write.h

# Test target dependencies
TEST_EXAMPLES := $(filter-out $(SDL_EXAMPLES),$(RAW_EXAMPLES))

# Test rules
check: $(TEST_DEPS) $(TEST_EXAMPLES)
	@echo "Running test suite..."
	CC=$(CC) ./tests/driver.sh

check-generate: $(TEST_DEPS) $(TEST_EXAMPLES)
	@echo "Generating reference outputs..."
	CC=$(CC) ./tests/driver.sh --generate-expected

# Phony declarations for test targets
.PHONY: check check-generate