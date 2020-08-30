// tgl_texture.cpp

#include "tgl.h"

GLTexture *find_texture(GLContext *c, int h)
{
    GLTexture *t;

    t = c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];
    while (t != NULL) {
	if (t->handle == h) {
	    return t;
	}
	t = t->next;
    }
    return NULL;
}

void free_texture(GLContext *c, int h)
{
    GLTexture *t,**ht;
    GLImage *im;
    int i;

    t = find_texture(c,h);
    if (t->prev == NULL) {
	ht = &c->shared_state.texture_hash_table[t->handle % TEXTURE_HASH_TABLE_SIZE];
	*ht = t->next;
    } else {
	t->prev->next = t->next;
    }
    if (t->next != NULL) {
	t->next->prev = t->prev;
    }

    for (i=0; i<MAX_TEXTURE_LEVELS; i++) {
	im = &t->images[i];
	if (im->pixmap != NULL) {
	    gl_free(im->pixmap);
	}
    }

    gl_free(t);
}

GLTexture *alloc_texture(GLContext *c, int h)
{
    GLTexture *t,**ht;

    t = (GLTexture*) gl_zalloc(sizeof(GLTexture));

    ht = &c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];

    t->next = *ht;
    t->prev = NULL;
    if (t->next != NULL) {
	t->next->prev = t;
    }
    *ht = t;

    t->handle = h;

    return t;
}

void gl_init_textures(GLContext *c)
{
    /* textures */

    c->texture.enabled_2d = 0;
    c->texture.current = find_texture(c,0);
}

void glGenTextures(GLsizei n, GLuint *textures)
{
    GLContext *c = gl_get_context();
    int max,i;
    GLTexture *t;

    max = 0;
    for (i=0; i<TEXTURE_HASH_TABLE_SIZE; i++) {
	t = c->shared_state.texture_hash_table[i];
	while (t != NULL) {
	    if (t->handle > max) {
		max = t->handle;
	    }
	    t = t->next;
	}
    }
    for (i=0; i<n; i++) {
	textures[i] = max+i+1;
    }
}

void glDeleteTextures(GLsizei n, const GLuint *textures)
{
    GLContext *c = gl_get_context();
    int i;
    GLTexture *t;

    for (i=0; i<n; i++) {
	t = find_texture(c,textures[i]);
	if (t != NULL && t != 0) {
	    if (t == c->texture.current) {
		glBindTexture(GL_TEXTURE_2D,0);
	    }
	    free_texture(c,textures[i]);
	}
    }
}

void glBindTexture(GLenum target, GLuint texture)
{
    GLContext *c = gl_get_context();
    GLTexture *t;

    assert(target == GL_TEXTURE_2D && texture >= 0);

    t = find_texture(c,texture);
    if (t == NULL) {
	t = alloc_texture(c,texture);
    }
    c->texture.current = t;
}

GLboolean glIsTexture(GLuint texture)
{
    GLContext *c = gl_get_context();
    GLTexture *t = find_texture(c, texture);
    return t != NULL;
}

void glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
		  GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    GLContext *c = gl_get_context();
    GLImage *im;
    unsigned char *pixels_temp1 = NULL;
    unsigned char *pixels_temp2 = NULL;
    unsigned char *pixels_ready = NULL;
    int width_original;
    int height_original;
    int _bitval;

    /*
    #ifdef _DEBUG
    char _t[256];
    FILE* ppmwriter;
    #endif
    */

#ifdef GLQUAKE
    if (level) {
	return;
    }
    if (internalFormat == 3) {
	internalFormat = 4;
    }
#endif

    pixels_ready = (unsigned char*) pixels;

    im = &c->texture.current->images[level];

    if (target != GL_TEXTURE_2D || level != 0 || border != 0 || type != GL_UNSIGNED_BYTE) {
	tgl_fatal_error("%s combination of parameters not handled",__FUNCTION__);
    }

    // check format
    if (/*format == GL_RGB &&*/ (internalFormat == GL_RGB8 || internalFormat == 3)) {
	// nothing to do
    } else if (/*format == GL_RGBA && */ (internalFormat == GL_RGBA8 || internalFormat == 4)) {
	pixels_temp2 = (unsigned char*) gl_malloc(width * height * 3);
	gl_convertRGBA32_to_RGB24(pixels_temp2,(unsigned char*)pixels,width,height);
	pixels_ready = pixels_temp2;
    } else if (internalFormat == 1) {
	im->pixmap = NULL;
	return;
    } else {
	tgl_fatal_error("%s this texture format is not supported",__FUNCTION__);
    }

    if (width != height || width < 64 || height < 64) {
	// make suare texture
	width_original = width;
	height_original = height;
	width = MAX(MAX(width,height),64);
	height = width;
	pixels_temp1 = (unsigned char*) gl_malloc(width * height * 3);
	// no interpolation is done here to respect the original image aliasing !
	gl_resizeImageNoInterpolate(
	    pixels_temp1,width,height,pixels_ready,width_original,height_original);
	pixels_ready = pixels_temp1;
    }

    im->xsize = width;
    im->ysize = height;

    /*
    #ifdef _DEBUG
    sprintf(_t,"texture_%04d.ppm",c->texture.current->handle);
    ppmwriter = fopen(_t,"wb");
    if ( ppmwriter )
    {
    	width_original = sprintf(_t,"P6\n# Created by anchor\n%d %d\n255\n",width,height);
    	fwrite(_t,1,width_original,ppmwriter);
    	fwrite(pixels_ready,1,width*height*3,ppmwriter);
    	fclose(ppmwriter);
    }
    #endif
    */

    _bitval = PSZSH;
    if (width == 64) {
	im->shift[0] = 6;
	im->shift[1] = 19-_bitval;
	im->uvmask   = 0x003F0000; // t,s
    } else if (width == 128) {
	im->shift[0] = 7;
	im->shift[1] = 18-_bitval;
	im->uvmask   = 0x003F8000; // t,s
    } else if (width == 256) {
	im->shift[0] = 8;
	im->shift[1] = 17-_bitval;
	im->uvmask   = 0x003FC000; // t,s
    } else if (width == 512) {
	im->shift[0] = 9;
	im->shift[1] = 16-_bitval;
	im->uvmask   = 0x003FE000; // t,s
    } else if (width == 1024) {
	im->shift[0] = 10;
	im->shift[1] = 15-_bitval;
	im->uvmask   = 0x003FF000; // t,s
    } else if (width == 2048) {
	im->shift[0] = 11;
	im->shift[1] = 14-_bitval;
	im->uvmask   = 0x003FF800; // t,s
    } else {
	tgl_fatal_error("%s invalid texture size",__FUNCTION__);
    }

    if (im->pixmap != NULL) {
	gl_free(im->pixmap);
    }
    im->pixmap = gl_malloc(width*height*4);
    if (im->pixmap) {
	gl_convertRGB24_to_ARGB32((PIXEL*)im->pixmap,pixels_ready,width,height);
    }
    if (pixels_temp1) {
	gl_free(pixels_temp1);
    }
    if (pixels_temp2) {
	gl_free(pixels_temp2);
    }
}

/* TODO: not all tests are done */
void glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    if (target != GL_TEXTURE_ENV || pname != GL_TEXTURE_ENV_MODE || param != GL_DECAL) {
	tgl_warning("%s unsupported option",__FUNCTION__);
    }
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    int pi = (int) param;
    glTexParameteri(target,pname,pi);
}

/* TODO: not all tests are done */
void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    if (target != GL_TEXTURE_2D) {
	tgl_warning("%s unsupported option",__FUNCTION__);
    }

    switch (pname) {
	case GL_TEXTURE_WRAP_S:
	case GL_TEXTURE_WRAP_T:
	    if (param != GL_REPEAT) {
		tgl_warning("%s unsupported option",__FUNCTION__);
	    }
	    break;
    }
}

void glPixelStorei(GLenum pname, GLint param)
{
    if (pname != GL_UNPACK_ALIGNMENT || param != 1) {
	tgl_warning("%s unsupported option",__FUNCTION__);
    }
}

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
