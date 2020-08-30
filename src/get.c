// tgl_get.cpp

#include "tgl.h"

void glGetIntegerv(GLenum pname, GLint *params)
{
    GLContext *c = gl_get_context();

    switch (pname) {
	case GL_VIEWPORT:
	    params[0] = c->viewport.xmin;
	    params[1] = c->viewport.ymin;
	    params[2] = c->viewport.xsize;
	    params[3] = c->viewport.ysize;
	    break;
	case GL_MAX_MODELVIEW_STACK_DEPTH:
	    *params = MAX_MODELVIEW_STACK_DEPTH;
	    break;
	case GL_MAX_PROJECTION_STACK_DEPTH:
	    *params = MAX_PROJECTION_STACK_DEPTH;
	    break;
	case GL_MAX_LIGHTS:
	    *params = MAX_LIGHTS;
	    break;
	case GL_MAX_TEXTURE_SIZE:
	    *params = MAX_TEXTURE_SIZE;
	    break;
	case GL_MAX_TEXTURE_STACK_DEPTH:
	    *params = MAX_TEXTURE_STACK_DEPTH;
	    break;
	default:
	    *params = 0;
	    tgl_warning("%s option not implemented",__FUNCTION__);
	    break;
    }
}

void glGetFloatv(GLenum pname, GLfloat *params)
{
    int i;
    int mnr = 0; /* just a trick to return the correct matrix */
    GLContext *c = gl_get_context();
    switch (pname) {
	case GL_CURRENT_COLOR: {
	    GLfloat *color = &c->current.color.X;
	    *params++ = *color++;
	    *params++ = *color++;
	    *params++ = *color++;
	    *params++ = *color++;
	    break;
	}
	case GL_CURRENT_NORMAL: {
	    GLfloat *normal = &c->current.normal.X;
	    *params++ = *normal++;
	    *params++ = *normal++;
	    *params++ = *normal++;
	    break;
	}
	case GL_CURRENT_TEXTURE_COORDS: {
	    GLfloat *tex_coord = &c->current.tex_coord.X;
	    *params++ = *tex_coord++;
	    *params++ = *tex_coord++;
	    *params++ = *tex_coord++;
	    *params++ = *tex_coord++;
	    break;
	}
	case GL_TEXTURE_MATRIX:
	    mnr++;
	case GL_PROJECTION_MATRIX:
	    mnr++;
	case GL_MODELVIEW_MATRIX: {
	    float *p = &c->matrix.stack_ptr[mnr]->m[0][0];;
	    for (i = 0; i < 4; i++) {
		*params++ = p[0];
		*params++ = p[4];
		*params++ = p[8];
		*params++ = p[12];
		p++;
	    }
	}
	break;
	case GL_LINE_WIDTH:
	    *params = 1.f;
	    break;
	case GL_LINE_WIDTH_RANGE:
	    params[0] = params[1] = 1.f;
	    break;
	case GL_POINT_SIZE:
	    *params = 1.f;
	    break;
	case GL_POINT_SIZE_RANGE:
	    params[0] = params[1] = 1.f;
	default:
	    tgl_warning("%s unknown pname",__FUNCTION__);
	    break;
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
