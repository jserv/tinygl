/*
 * Z buffer: 16 bits Z / 16 bits color
 */

#include <stdlib.h>
#include <string.h>

#include "msghandling.h"
#include "zbuffer.h"

ZBuffer *ZB_open(GLint xsize,
                 GLint ysize,
                 GLint mode,

                 void *frame_buffer)
{
    ZBuffer *zb;
    GLint size;

    zb = gl_malloc(sizeof(ZBuffer));
    if (zb == NULL)
        return NULL;

    zb->xsize = xsize & ~3;
    zb->ysize = ysize;

    zb->linesize = (xsize * PSZB);

    switch (mode) {
#if TGL_HAS(32_BITS)
    case ZB_MODE_RGBA:
        break;
#endif
#if TGL_HAS(16_BITS)
    case ZB_MODE_5R6G5B:
        break;
#endif

    default:
        goto error;
    }

    size = zb->xsize * zb->ysize * sizeof(GLushort);

    zb->zbuf = gl_malloc(size);
    if (zb->zbuf == NULL)
        goto error;

    if (frame_buffer == NULL) {
        zb->pbuf = gl_malloc(zb->ysize * zb->linesize);
        if (zb->pbuf == NULL) {
            gl_free(zb->zbuf);
            goto error;
        }
        zb->frame_buffer_allocated = 1;
    } else {
        zb->frame_buffer_allocated = 0;
        zb->pbuf = frame_buffer;
    }

    zb->current_texture = NULL;

    return zb;
error:
    gl_free(zb);
    return NULL;
}

void ZB_close(ZBuffer *zb)
{
    if (zb->frame_buffer_allocated)
        gl_free(zb->pbuf);

    gl_free(zb->zbuf);
    gl_free(zb);
}

/*
 * Resize the ZBuffer. Returns 0 on success, -1 on allocation failure.
 * On failure, the original buffer state is preserved.
 */
GLint ZB_resize(ZBuffer *zb, void *frame_buffer, GLint xsize, GLint ysize)
{
    GLint size;
    GLushort *new_zbuf;
    PIXEL *new_pbuf = NULL;

    /* xsize must be a multiple of 4 */
    xsize = xsize & ~3;

    size = xsize * ysize * sizeof(GLushort);

    new_zbuf = gl_malloc(size);
    if (new_zbuf == NULL)
        return -1;

    if (frame_buffer == NULL) {
        new_pbuf = gl_malloc(ysize * (xsize * PSZB));
        if (!new_pbuf) {
            gl_free(new_zbuf);
            return -1;
        }
    }

    /* Only free old buffers after successful allocation */
    gl_free(zb->zbuf);
    if (zb->frame_buffer_allocated)
        gl_free(zb->pbuf);

    zb->zbuf = new_zbuf;
    zb->xsize = xsize;
    zb->ysize = ysize;
    zb->linesize = (xsize * PSZB);

    if (frame_buffer == NULL) {
        zb->pbuf = new_pbuf;
        zb->frame_buffer_allocated = 1;
    } else {
        zb->pbuf = frame_buffer;
        zb->frame_buffer_allocated = 0;
    }

    return 0;
}

#if TGL_HAS(32_BITS)
PIXEL pxReverse32(PIXEL x)
{
    return ((x & 0xFF000000) >> 24) | /*______AA*/
           ((x & 0x00FF0000) >> 8) |  /*____RR__*/
           ((x & 0x0000FF00) << 8) |  /*__GG____*/
           ((x & 0x000000FF) << 24);  /* BB______*/
}
#endif

static void ZB_copyBuffer(ZBuffer *zb, void *buf, GLint linesize)
{
    GLint y;
#if TGL_HAS(NO_COPY_COLOR)
    GLint i;
#endif
#if TGL_HAS(MULTITHREADED_ZB_COPYBUFFER)
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (y = 0; y < zb->ysize; y++) {
        PIXEL *q;
        GLubyte *p1;
        q = zb->pbuf + y * zb->xsize;
        p1 = (GLubyte *) buf + y * linesize;
#if TGL_HAS(NO_COPY_COLOR)
        for (i = 0; i < zb->xsize; i++) {
            if ((*(q + i) & TGL_COLOR_MASK) != TGL_NO_COPY_COLOR)
                *(((PIXEL *) p1) + i) = *(q + i);
        }
#else
        memcpy(p1, q, linesize);
#endif
    }
#else
    for (y = 0; y < zb->ysize; y++) {
        PIXEL *q;
        GLubyte *p1;
        q = zb->pbuf + y * zb->xsize;
        p1 = (GLubyte *) buf + y * linesize;
#if TGL_HAS(NO_COPY_COLOR)
        for (i = 0; i < zb->xsize; i++) {
            if ((*(q + i) & TGL_COLOR_MASK) != TGL_NO_COPY_COLOR)
                *(((PIXEL *) p1) + i) = *(q + i);
        }
#else
        memcpy(p1, q, linesize);
#endif
    }
#endif
}

#if TGL_FEATURE_RENDER_BITS == 16
void ZB_copyFrameBuffer(ZBuffer *zb, void *buf, GLint linesize)
{
    ZB_copyBuffer(zb, buf, linesize);
}
#endif

#if TGL_FEATURE_RENDER_BITS == 32
#define RGB32_TO_RGB16(v) \
    (((v >> 8) & 0xf800) | (((v) >> 5) & 0x07e0) | (((v) & 0xff) >> 3))

void ZB_copyFrameBuffer(ZBuffer *zb, void *buf, GLint linesize)
{
    ZB_copyBuffer(zb, buf, linesize);
}
#endif

/*
 * adr must be aligned on an 'int'
 */
static void memset_s(void *adr, GLint val, GLint count)
{
    GLint i, n, v;
    GLuint *p;
    GLushort *q;

    p = adr;
    v = val | (val << 16);

    n = count >> 3;
    for (i = 0; i < n; i++) {
        p[0] = v;
        p[1] = v;
        p[2] = v;
        p[3] = v;
        p += 4;
    }

    q = (GLushort *) p;
    n = count & 7;
    for (i = 0; i < n; i++)
        *q++ = val;
}

/* Used in 32 bit mode*/
static void memset_l(void *adr, GLint val, GLint count)
{
    GLint i, n, v;
    GLuint *p;
    p = adr;
    v = val;
    n = count >> 2;
    for (i = 0; i < n; i++) {
        p[0] = v;
        p[1] = v;
        p[2] = v;
        p[3] = v;
        p += 4;
    }
    n = count & 3;
    for (i = 0; i < n; i++)
        *p++ = val;
}

void ZB_clear(ZBuffer *zb,
              GLint clear_z,
              GLint z,
              GLint clear_color,
              GLint r,
              GLint g,
              GLint b)
{
    GLuint color;
    GLint y;
    PIXEL *pp;
    if (clear_z) {
        GLint zbuf_size = zb->xsize * zb->ysize;
        if (z == 0) {
            /* All bytes 0x00 - use optimized memset */
            memset(zb->zbuf, 0, zbuf_size * sizeof(GLushort));
        } else if ((z & 0xFFFF) == 0xFFFF) {
            /* All bytes 0xFF - use optimized memset (catches -1 and 0xFFFF) */
            memset(zb->zbuf, 0xFF, zbuf_size * sizeof(GLushort));
        } else {
            /* Arbitrary 16-bit value - use manual loop */
            memset_s(zb->zbuf, z, zbuf_size);
        }
    }
    if (clear_color) {
        pp = zb->pbuf;
        for (y = 0; y < zb->ysize; y++) {
#if TGL_FEATURE_RENDER_BITS == 15 || TGL_FEATURE_RENDER_BITS == 16
            // color = RGB_TO_PIXEL(r, g, b);
#if TGL_FEATURE_FORCE_CLEAR_NO_COPY_COLOR
            color = TGL_NO_COPY_COLOR;
#else
            color = RGB_TO_PIXEL(r, g, b);
#endif
            memset_s(pp, color, zb->xsize);
#elif TGL_FEATURE_RENDER_BITS == 32
#if TGL_HAS(FORCE_CLEAR_NO_COPY_COLOR)
            color = TGL_NO_COPY_COLOR;
#else
            color = RGB_TO_PIXEL(r, g, b);
#endif
            memset_l(pp, color, zb->xsize);
#else
#error BADJUJU
#endif
            pp = (PIXEL *) ((GLbyte *) pp + zb->linesize);
        }
    }
}
