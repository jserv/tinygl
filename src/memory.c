/*
 * Memory allocator for TinyGL
 */
#include "zgl.h"
#include <stdlib.h>
#include <string.h>

static inline void required_for_compilation_() { return; }

/* modify these functions so that they suit your needs */
#include <string.h>
void gl_free(void* p) { free(p); }

void* gl_malloc(GLint size) { return malloc(size); }

void* gl_zalloc(GLint size) { return calloc(1, size); }
