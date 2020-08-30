// tgl_api.cpp

#include "tgl.h"

// stubs

void glDrawBuffer(GLenum mode)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glFinish()
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glDepthMask(GLboolean flag)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glDepthRange(GLclampd zNear, GLclampd zFar)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glDepthFunc(GLenum func)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glReadBuffer(GLenum mode)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glReadPixels(GLint x, GLint y,GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glColor3ubv(const GLubyte *v)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) 
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) 
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glClearDepthf(GLclampf depth) 
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glFogfv(GLenum pname, const GLfloat *params)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glFogf(GLenum pname, GLfloat param) 
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glLineWidth(GLfloat width)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void glPointSize(GLfloat size)
{
	tgl_info("%s STUB",__FUNCTION__);
}

void gluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan( fovy * M_PI / 360.0 );
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

GLenum glGetError()
{
	return 0;
} 

const GLubyte *glGetString(GLenum name)
{
	switch ( name )
	{
	case GL_VENDOR:
		return (const GLubyte*) "Amiga";
	case GL_RENDERER:
#if TGL_FEATURE_RENDER_BITS == 15
		return (const GLubyte*) "TinyGL (15bit)";
#elif TGL_FEATURE_RENDER_BITS == 16
		return (const GLubyte*) "TinyGL (16bit)";
#elif TGL_FEATURE_RENDER_BITS == 24
		return (const GLubyte*) "TinyGL (24bit)";
#elif TGL_FEATURE_RENDER_BITS == 32
		return (const GLubyte*) "TinyGL (32bit)";
#else
		return (const GLubyte*) "TinyGL";
#endif
	case GL_VERSION:
		return (const GLubyte*) "0.6.0";
	case GL_EXTENSIONS:
		return (const GLubyte*) "";
	default:
		tgl_warning("%s invalid parameter",__FUNCTION__);
		break;
	}
	return NULL;
}

/* Non standard functions */

void glDebug(int mode)
{
	GLContext *c = gl_get_context();
	c->print_flag = mode;
}
