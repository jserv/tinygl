// tgl_clear.cpp

#include "tgl.h"

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	GLContext *c = gl_get_context();
	c->clear.color.X = red;
	c->clear.color.Y = green;
	c->clear.color.Z = blue;
	c->clear.color.W = alpha;
}

void glClearDepth(GLclampd depth) 
{
	GLContext *c = gl_get_context();
	c->clear.depth = (float) depth;
}

void glClear(GLbitfield mask) 
{
	GLContext *c = gl_get_context();
	int z = 0;
	int r = (int)(c->clear.color.X * 65535);
	int g = (int)(c->clear.color.Y * 65535);
	int b = (int)(c->clear.color.Z * 65535);

	/* TODO : correct value of Z */

	ZB_clear(c->zb, mask & GL_DEPTH_BUFFER_BIT, z, mask & GL_COLOR_BUFFER_BIT, r, g, b);
} 
