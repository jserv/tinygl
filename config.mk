CC ?= gcc
CFLAGS = -Wall -O3 -std=c99 -DNDEBUG
CFLAGS += -march=native -Wno-unused-function

# OpenMP (optional)
# CFLAGS += -fopenmp

LDFLAGS =
