CC ?= gcc
CFLAGS = -Wall -O3 -std=c99 -DNDEBUG
CFLAGS += -Wno-unused-function

# OpenMP (optional)
# CFLAGS += -fopenmp

LDFLAGS =
