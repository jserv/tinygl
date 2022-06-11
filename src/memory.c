/*
 * Memory allocator for TinyGL
 */

#include <stdlib.h>
#include "zgl.h"

void gl_free(void *p)
{
    free(p);
}

void *gl_malloc(GLint size)
{
    return malloc(size);
}

void *gl_zalloc(GLint size)
{
    return calloc(1, size);
}
