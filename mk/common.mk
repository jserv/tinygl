# mk/common.mk - Common definitions and platform detection for TinyGL

# Platform detection
UNAME_S := $(shell uname -s)

# Compiler and base flags
CC ?= gcc
CFLAGS ?= -Wall -O3 -std=c99 -DNDEBUG
CFLAGS += -Wno-unused-function

# Debug build support (append, don't replace)
ifeq ($(DEBUG), 1)
    CFLAGS := -g -DDEBUG -Wall -Wextra
endif

# Sanitizer support
ifeq ($(SANITIZE), 1)
    CFLAGS += -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
endif

# Directory structure
INCLUDE_DIR := include
SRC_DIR := src
EXAMPLES_DIR := examples
TESTS_DIR := tests

# Common include path
INCLUDES := -I$(INCLUDE_DIR)

# Common libraries
LIBS := -lm

# Utility function: check if command exists
define has
$(shell command -v $(1) > /dev/null 2>&1 && echo 1)
endef

# Verbosity control (V=1 for verbose)
ifeq ($(V), 1)
    Q :=
    VECHO := @true
else
    Q := @
    VECHO := @echo
endif