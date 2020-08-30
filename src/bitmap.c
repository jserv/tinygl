// tgl_bitmap.cpp

#include "tgl.h"

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    pos->x = x;
    pos->y = y;
    // TODO: update the zbuffer?
    pos->z = z;
}

const unsigned int* pbuf_pos(ZBuffer *zb, int x, int y)
{
    return (unsigned int*)zb->pbuf + (y * zb->linesize) + x * PSZB;
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
	      GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    const GLubyte *from;
    const unsigned int* to;
    int x, y;
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    ZBuffer *zb = c->zb;
    GLuint b;
    int j;
    GLuint value;

    // TODO: shouldn't be drawn if the raster pos is outside the viewport?
    // TODO: negative width/height mirrors bitmap?
    if (!width && !height) {
	pos->x += xmove;
	pos->y -= ymove;
	return;
    }

    // TODO: support 16-bit pixel size?

    // copy to pixel data
    // TODO: strip blank lines and mirror vertically?
    for (y = 0; y < height; y++) {
	to = pbuf_pos(zb, (int)pos->x, (int)(pos->y - y));
	from = bitmap + (y * 2);
	for (x = 0; x < width; x += 8) {
	    if (pos->x + x > zb->xsize || pos->y - y > zb->ysize) {
		continue;
	    }

	    b = *from++;
	    for (j = 8; j--;) {
		value = (b & (1 << j)) ? 0xFFFFFFFF : 0;
		*(TGL_PIXEL_TYPE *)to |= value;
		to += PSZB;
	    }
	}
    }

    pos->x += xmove;
    pos->y += ymove;
}

/* Unfinished */
#if 0
void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *data)
{
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    ZBuffer *zb = c->zb;
    GLViewport *viewport = &c->viewport;
    int	ystart;
    int xstart;
    int screen_width;

    /*
    const GLvoid *from;
    GLvoid *to;
    */

    // shrink our pixel ranges to stay inside the viewport
    ystart = MAX(0, (int)-pos->y);
    height = MIN((int)pos->y, height);
    if (pos->y >= viewport->ysize) {
	ystart += (int)pos->y - viewport->ysize - 1;
    }
    xstart = MAX(0, (int)-pos->x);
    screen_width = MIN((int)(viewport->xsize - pos->x), width - 1);
    if (screen_width < 0) {
	return;
    }

    tgl_warning("%s unfinished function",__FUNCTION__);

    /*
    GLsizei src_stride = gl_pixel_sizeof(format, type);
    for (int y = ystart; y < height; y++)
    {
    to = (GLubyte *)pbuf_pos(zb, int(pos->x), int(pos->y - y - 1));
    from = (void*)((char*)data + src_stride * (xstart + y * width));
    pixel_convert_direct(from, to, screen_width, format, type, src_stride, GL_RGBA, TGL_PIXEL_ENUM, PSZB);
    }
    */
}
#endif

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
