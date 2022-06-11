/*
 * Texture Manager
 */

#include "zgl.h"

static GLTexture *find_texture(GLint h)
{
    GLTexture *t;
    GLContext *c = gl_get_context();
    t = c->shared_state.texture_hash_table[h & TEXTURE_HASH_TABLE_MASK];
    for (; t; t = t->next) {
        if (t->handle == h)
            return t;
    }
    return NULL;
}

GLboolean glAreTexturesResident(GLsizei n,
                                const GLuint *textures,
                                GLboolean *residences)
{
#define RETVAL GL_FALSE
    GLboolean retval = GL_TRUE;
#include "error_check_no_context.h"

    for (GLint i = 0; i < n; i++)
        if (find_texture(textures[i])) {
            residences[i] = GL_TRUE;
        } else {
            residences[i] = GL_FALSE;
            retval = GL_FALSE;
        }
    return retval;
}

GLboolean glIsTexture(GLuint texture)
{
#define RETVAL GL_FALSE
#include "error_check.h"
    if (find_texture(texture))
        return GL_TRUE;
    return GL_FALSE;
}

void *glGetTexturePixmap(GLint text, GLint level, GLint *xsize, GLint *ysize)
{
    GLTexture *tex;
#if TGL_HAS(ERROR_CHECK)
    if (!(text >= 0 && level < MAX_TEXTURE_LEVELS))
#define ERROR_FLAG GL_INVALID_ENUM
#define RETVAL NULL
#include "error_check.h"
#endif
        tex = find_texture(text);
    if (!tex)
#if TGL_HAS(ERROR_CHECK)
#define ERROR_FLAG GL_INVALID_ENUM
#define RETVAL NULL
#include "error_check.h"
#else
        return NULL;
#endif
        *xsize = tex->images[level].xsize;
    *ysize = tex->images[level].ysize;
    return tex->images[level].pixmap;
}

static void free_texture(GLContext *c, GLint h)
{
    GLTexture *t = find_texture(h);
    if (!t->prev) {
        GLTexture **ht =
            &c->shared_state
                 .texture_hash_table[t->handle & TEXTURE_HASH_TABLE_MASK];
        *ht = t->next;
    } else {
        t->prev->next = t->next;
    }
    if (t->next)
        t->next->prev = t->prev;

    gl_free(t);
}

GLTexture *alloc_texture(GLint h)
{
    GLContext *c = gl_get_context();
    GLTexture **ht;
#define RETVAL NULL
#include "error_check.h"
    GLTexture *t = gl_zalloc(sizeof(GLTexture));
    if (!t)
#if TGL_HAS(ERROR_CHECK)
#define ERROR_FLAG GL_OUT_OF_MEMORY
#define RETVAL NULL
#include "error_check.h"
#else
        gl_fatal_error("GL_OUT_OF_MEMORY");
#endif

        ht = &c->shared_state.texture_hash_table[h & TEXTURE_HASH_TABLE_MASK];

    if (t) {
        if (ht)
            t->next = *ht;
        t->prev = NULL;
        if (t->next != NULL)
            t->next->prev = t;
        if (ht)
            *ht = t;
        t->handle = h;
    }

    return t;
}

void glInitTextures()
{
    /* textures */
    GLContext *c = gl_get_context();
    c->texture_2d_enabled = 0;
    c->current_texture = find_texture(0);
}

void glGenTextures(GLint n, GLuint *textures)
{
    GLContext *c = gl_get_context();
#include "error_check.h"
    GLint max = 0;
    for (GLint i = 0; i < TEXTURE_HASH_TABLE_SIZE; i++) {
        GLTexture *t = c->shared_state.texture_hash_table[i];
        for (; t; t = t->next) {
            if (t->handle > max)
                max = t->handle;
        }
    }
    for (GLint i = 0; i < n; i++) {
        textures[i] = max + i + 1; /* MARK: How texture handles are created.*/
    }
}

void glDeleteTextures(GLint n, const GLuint *textures)
{
    GLContext *c = gl_get_context();
#include "error_check.h"
    for (GLint i = 0; i < n; i++) {
        GLTexture *t = find_texture(textures[i]);
        if (t) {
            if (t == c->current_texture) {
                glBindTexture(GL_TEXTURE_2D, 0);
#include "error_check.h"
            }
            free_texture(c, textures[i]);
        }
    }
}

void glopBindTexture(GLParam *p)
{
    GLint texture = p[2].i;
    GLTexture *t;
    GLContext *c = gl_get_context();
#if TGL_HAS(ERROR_CHECK)
    if (!(target == GL_TEXTURE_2D && target > 0))
#define ERROR_FLAG GL_INVALID_ENUM
#include "error_check.h"
#else

#endif
        t = find_texture(texture);
    if (!t) {
        t = alloc_texture(texture);
#include "error_check.h"
    }
    if (!t) {
#if TGL_HAS(ERROR_CHECK)
#define ERROR_FLAG GL_OUT_OF_MEMORY
#include "error_check.h"
#else
        gl_fatal_error("GL_OUT_OF_MEMORY");
#endif
    }
    c->current_texture = t;
}

void glCopyTexImage2D(GLenum target,
                      GLint level,
                      GLenum internalformat,
                      GLint x,
                      GLint y,
                      GLsizei width,
                      GLsizei height,
                      GLint border)
{
#include "error_check_no_context.h"

    GLParam p[9] = {
        [0].op = OP_CopyTexImage2D,
        [1].i = target,
        [2].i = level,
        [3].i = internalformat,
        [4].i = x,
        [5].i = y,
        [6].i = width,
        [7].i = height,
        [8].i = border,
    };
    gl_add_op(p);
}

void glopCopyTexImage2D(GLParam *p)
{
    GLint target = p[1].i;
    GLint level = p[2].i;
    GLint x = p[4].i;
    GLint y = p[5].i;
    GLsizei w = p[6].i;
    GLsizei h = p[7].i;
    GLint border = p[8].i;
    GLContext *c = gl_get_context();
    y -= h;

    if (c->readbuffer != GL_FRONT || c->current_texture == NULL ||
        target != GL_TEXTURE_2D || border != 0 ||
        w != TGL_FEATURE_TEXTURE_DIM || /*TODO Implement image interp */
        h != TGL_FEATURE_TEXTURE_DIM) {
#if TGL_HAS(ERROR_CHECK)
#define ERROR_FLAG GL_INVALID_OPERATION
#include "error_check.h"
#else
        return;
#endif
    }

    GLImage *im = &c->current_texture->images[level];
    PIXEL *data = c->current_texture->images[level].pixmap;
    im->xsize = TGL_FEATURE_TEXTURE_DIM;
    im->ysize = TGL_FEATURE_TEXTURE_DIM;
    /* TODO implement the scaling and stuff that the GL spec says it should
     * have.
     */
#if TGL_HAS(MULTITHREADED_COPY_TEXIMAGE_2D)
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (GLint j = 0; j < h; j++)
        for (GLint i = 0; i < w; i++) {
            data[i + j * w] =
                c->zb->pbuf[((i + x) % (c->zb->xsize)) +
                            ((j + y) % (c->zb->ysize)) * (c->zb->xsize)];
        }
#else
    for (GLint j = 0; j < h; j++)
        for (GLint i = 0; i < w; i++) {
            data[i + j * w] =
                c->zb->pbuf[((i + x) % (c->zb->xsize)) +
                            ((j + y) % (c->zb->ysize)) * (c->zb->xsize)];
        }
#endif
}

void glopTexImage1D(GLParam *p)
{
    GLint target = p[1].i;
    GLint level = p[2].i;
    GLint components = p[3].i;
    GLint width = p[4].i;
    /* GLint height = p[5].i;*/
    GLint height = 1;
    GLint border = p[5].i;
    GLint format = p[6].i;
    GLint type = p[7].i;
    void *pixels = p[8].p;
    GLubyte *pixels1;
    GLint do_free = 0;
    GLContext *c = gl_get_context();

    {
#if TGL_HAS(ERROR_CHECK)
        if (!(c->current_texture != NULL && target == GL_TEXTURE_1D &&
              level == 0 && components == 3 && border == 0 &&
              format == GL_RGB && type == GL_UNSIGNED_BYTE))
#define ERROR_FLAG GL_INVALID_ENUM
#include "error_check.h"
#else
        if (!(c->current_texture != NULL && target == GL_TEXTURE_1D &&
              level == 0 && components == 3 && border == 0 &&
              format == GL_RGB && type == GL_UNSIGNED_BYTE))
            gl_fatal_error(
                "glTexImage2D: combination of parameters not handled!!");
#endif
    }

    if (width != TGL_FEATURE_TEXTURE_DIM || height != TGL_FEATURE_TEXTURE_DIM) {
        pixels1 = gl_malloc(TGL_FEATURE_TEXTURE_DIM * TGL_FEATURE_TEXTURE_DIM *
                            3); /* GUARDED */
        if (!pixels1) {
#if TGL_HAS(ERROR_CHECK)
#define ERROR_FLAG GL_OUT_OF_MEMORY
#include "error_check.h"
#else
            gl_fatal_error("GL_OUT_OF_MEMORY");
#endif
        }

        /* no GLinterpolation is done here to respect the original image
         * aliasing.
         */
        gl_resizeImageNoInterpolate(pixels1, TGL_FEATURE_TEXTURE_DIM,
                                    TGL_FEATURE_TEXTURE_DIM, pixels, width,
                                    height);
        do_free = 1;
        width = TGL_FEATURE_TEXTURE_DIM;
        height = TGL_FEATURE_TEXTURE_DIM;
    } else {
        pixels1 = pixels;
    }

    GLImage *im = &c->current_texture->images[level];
    im->xsize = width;
    im->ysize = height;
#if TGL_FEATURE_RENDER_BITS == 32
    gl_convertRGB_to_8A8R8G8B(im->pixmap, pixels1, width, height);
#elif TGL_FEATURE_RENDER_BITS == 16
    gl_convertRGB_to_5R6G5B(im->pixmap, pixels1, width, height);
#else
#error bad TGL_FEATURE_RENDER_BITS
#endif
    if (do_free)
        gl_free(pixels1);
}

void glopTexImage2D(GLParam *p)
{
    GLint target = p[1].i;
    GLint level = p[2].i;
    GLint components = p[3].i;
    GLint width = p[4].i;
    GLint height = p[5].i;
    GLint border = p[6].i;
    GLint format = p[7].i;
    GLint type = p[8].i;
    void *pixels = p[9].p;
    GLubyte *pixels1;
    GLint do_free = 0;
    GLContext *c = gl_get_context();
    {
#if TGL_HAS(ERROR_CHECK)
        if (!(c->current_texture != NULL && target == GL_TEXTURE_2D &&
              level == 0 && components == 3 && border == 0 &&
              format == GL_RGB && type == GL_UNSIGNED_BYTE))
#define ERROR_FLAG GL_INVALID_ENUM
#include "error_check.h"

#else
        if (!(c->current_texture != NULL && target == GL_TEXTURE_2D &&
              level == 0 && components == 3 && border == 0 &&
              format == GL_RGB && type == GL_UNSIGNED_BYTE))
            gl_fatal_error(
                "glTexImage2D: combination of parameters not handled!!");
#endif
    }
    if (width != TGL_FEATURE_TEXTURE_DIM || height != TGL_FEATURE_TEXTURE_DIM) {
        pixels1 = gl_malloc(TGL_FEATURE_TEXTURE_DIM * TGL_FEATURE_TEXTURE_DIM *
                            3); /* GUARDED*/
        if (!pixels1) {
#if TGL_HAS(ERROR_CHECK)
#define ERROR_FLAG GL_OUT_OF_MEMORY
#include "error_check.h"
#else
            gl_fatal_error("GL_OUT_OF_MEMORY");
#endif
        }
        /* no GLinterpolation is done here to respect the original image
         * aliasing ! */

        gl_resizeImageNoInterpolate(pixels1, TGL_FEATURE_TEXTURE_DIM,
                                    TGL_FEATURE_TEXTURE_DIM, pixels, width,
                                    height);
        do_free = 1;
        width = TGL_FEATURE_TEXTURE_DIM;
        height = TGL_FEATURE_TEXTURE_DIM;
    } else {
        pixels1 = pixels;
    }

    GLImage *im = &c->current_texture->images[level];
    im->xsize = width;
    im->ysize = height;
#if TGL_FEATURE_RENDER_BITS == 32
    gl_convertRGB_to_8A8R8G8B(im->pixmap, pixels1, width, height);
#elif TGL_FEATURE_RENDER_BITS == 16
    gl_convertRGB_to_5R6G5B(im->pixmap, pixels1, width, height);
#else
#error Bad TGL_FEATURE_RENDER_BITS
#endif
    if (do_free)
        gl_free(pixels1);
}
