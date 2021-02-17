#include "zgl.h"
//#include <stdio.h>
/* glVertex */

void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	GLParam p[5];

	p[0].op = OP_Vertex;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;
	p[4].f = w;

	gl_add_op(p);
}

void glVertex2f(GLfloat x, GLfloat y) { glVertex4f(x, y, 0, 1); }

void glVertex3f(GLfloat x, GLfloat y, GLfloat z) { glVertex4f(x, y, z, 1); }

void glVertex3fv(GLfloat* v) { glVertex4f(v[0], v[1], v[2], 1); }

/* glNormal */

void glNormal3f(GLfloat x, GLfloat y, GLfloat z) {
	GLParam p[4];

	p[0].op = OP_Normal;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	gl_add_op(p);
}

void glNormal3fv(GLfloat* v) { glNormal3f(v[0], v[1], v[2]); }

/* glColor */

void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GLParam p[8];

	p[0].op = OP_Color;
	p[1].f = r;
	p[2].f = g;
	p[3].f = b;
	p[4].f = a;
	/* direct convertion to GLinteger to go faster if no shading */
	/*
	p[5].ui = (GLuint) (r * (ZB_POINT_RED_MAX - ZB_POINT_RED_MIN) +
							  ZB_POINT_RED_MIN);
	p[6].ui = (GLuint) (g * (ZB_POINT_GREEN_MAX - ZB_POINT_GREEN_MIN) +
							  ZB_POINT_GREEN_MIN);
	p[7].ui = (GLuint) (b * (ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN) +
							  ZB_POINT_BLUE_MIN);
	*/
	p[5].ui = (((GLuint)(r * 65535)) & 65535);
	p[6].ui = (((GLuint)(g * 65535)) & 65535);
	p[7].ui = (((GLuint)(b * 65535)) & 65535);
	gl_add_op(p);
}

void glColor4fv(GLfloat* v) {
	GLParam p[8];

	p[0].op = OP_Color;
	p[1].f = v[0];
	p[2].f = v[1];
	p[3].f = v[2];
	p[4].f = v[3];
	/* direct convertion to GLinteger to go faster if no shading */
	/*
	p[5].ui = (GLuint) (v[0] * (ZB_POINT_RED_MAX - ZB_POINT_RED_MIN) +
							  ZB_POINT_RED_MIN);
	p[6].ui = (GLuint) (v[1] * (ZB_POINT_GREEN_MAX - ZB_POINT_GREEN_MIN) +
							  ZB_POINT_GREEN_MIN);
	p[7].ui = (GLuint) (v[2] * (ZB_POINT_BLUE_MAX - ZB_POINT_BLUE_MIN) +
							  ZB_POINT_BLUE_MIN);
	*/
	p[5].ui = (((GLuint)(v[0] * 65535)) & 65535);
	p[6].ui = (((GLuint)(v[1] * 65535)) & 65535);
	p[7].ui = (((GLuint)(v[2] * 65535)) & 65535);

	gl_add_op(p);
}

void glColor3f(GLfloat x, GLfloat y, GLfloat z) { glColor4f(x, y, z, 1); }

void glColor3fv(GLfloat* v) { glColor4f(v[0], v[1], v[2], 1); }

/* TexCoord */

void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
	GLParam p[5];

	p[0].op = OP_TexCoord;
	p[1].f = s;
	p[2].f = t;
	p[3].f = r;
	p[4].f = q;

	gl_add_op(p);
}

void glTexCoord2f(GLfloat s, GLfloat t) { glTexCoord4f(s, t, 0, 1); }

void glTexCoord2fv(GLfloat* v) { glTexCoord4f(v[0], v[1], 0, 1); }

void glEdgeFlag(GLint flag) {
	GLParam p[2];

	p[0].op = OP_EdgeFlag;
	p[1].i = flag;

	gl_add_op(p);
}

/* misc */

void glShadeModel(GLint mode) {
	GLParam p[2];

	assert(mode == GL_FLAT || mode == GL_SMOOTH);

	p[0].op = OP_ShadeModel;
	p[1].i = mode;

	gl_add_op(p);
}

void glCullFace(GLint mode) {
	GLParam p[2];

	assert(mode == GL_BACK || mode == GL_FRONT || mode == GL_FRONT_AND_BACK);

	p[0].op = OP_CullFace;
	p[1].i = mode;

	gl_add_op(p);
}

void glFrontFace(GLint mode) {
	GLParam p[2];

	assert(mode == GL_CCW || mode == GL_CW);

	mode = (mode != GL_CCW);

	p[0].op = OP_FrontFace;
	p[1].i = mode;

	gl_add_op(p);
}

void glPolygonMode(GLint face, GLint mode) {
	GLParam p[3];

	assert(face == GL_BACK || face == GL_FRONT || face == GL_FRONT_AND_BACK);
	assert(mode == GL_POINT || mode == GL_LINE || mode == GL_FILL);

	p[0].op = OP_PolygonMode;
	p[1].i = face;
	p[2].i = mode;

	gl_add_op(p);
}

void glDepthMask(GLint i){
	gl_get_context()->zb->depth_write = (i==GL_TRUE);
}
/* glEnable / glDisable */

void glEnable(GLint cap) {
	GLParam p[3];

	p[0].op = OP_EnableDisable;
	p[1].i = cap;
	p[2].i = 1;

	gl_add_op(p);
}

void glDisable(GLint cap) {
	GLParam p[3];

	p[0].op = OP_EnableDisable;
	p[1].i = cap;
	p[2].i = 0;

	gl_add_op(p);
}

/* glBegin / glEnd */

void glBegin(GLint mode) {
	GLParam p[2];

	p[0].op = OP_Begin;
	p[1].i = mode;

	gl_add_op(p);
}

void glEnd(void) {
	GLParam p[1];

	p[0].op = OP_End;

	gl_add_op(p);
}

/* matrix */

void glMatrixMode(GLint mode) {
	GLParam p[2];

	p[0].op = OP_MatrixMode;
	p[1].i = mode;

	gl_add_op(p);
}

void glLoadMatrixf(const GLfloat* m) {
	GLParam p[17];
	GLint i;

	p[0].op = OP_LoadMatrix;
	for (i = 0; i < 16; i++)
		p[i + 1].f = m[i];

	gl_add_op(p);
}

void glLoadIdentity(void) {
	GLParam p[1];

	p[0].op = OP_LoadIdentity;

	gl_add_op(p);
}

void glMultMatrixf(const GLfloat* m) {
	GLParam p[17];
	GLint i;

	p[0].op = OP_MultMatrix;
	for (i = 0; i < 16; i++)
		p[i + 1].f = m[i];

	gl_add_op(p);
}

void glPushMatrix(void) {
	GLParam p[1];

	p[0].op = OP_PushMatrix;

	gl_add_op(p);
}

void glPopMatrix(void) {
	GLParam p[1];

	p[0].op = OP_PopMatrix;

	gl_add_op(p);
}

void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	GLParam p[5];

	p[0].op = OP_Rotate;
	p[1].f = angle;
	p[2].f = x;
	p[3].f = y;
	p[4].f = z;

	gl_add_op(p);
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
	GLParam p[4];

	p[0].op = OP_Translate;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	gl_add_op(p);
}

void glScalef(GLfloat x, GLfloat y, GLfloat z) {
	GLParam p[4];

	p[0].op = OP_Scale;
	p[1].f = x;
	p[2].f = y;
	p[3].f = z;

	gl_add_op(p);
}

void glViewport(GLint x, GLint y, GLint width, GLint height) {
	GLParam p[5];

	p[0].op = OP_Viewport;
	p[1].i = x;
	p[2].i = y;
	p[3].i = width;
	p[4].i = height;

	gl_add_op(p);
}

void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble farv) {
	GLParam p[7];

	p[0].op = OP_Frustum;
	p[1].f = left;
	p[2].f = right;
	p[3].f = bottom;
	p[4].f = top;
	p[5].f = near;
	p[6].f = farv;

	gl_add_op(p);
}

/* lightening */

void glMaterialfv(GLint mode, GLint type, GLfloat* v) {
	GLParam p[7];
	GLint i, n;

	assert(mode == GL_FRONT || mode == GL_BACK || mode == GL_FRONT_AND_BACK);

	p[0].op = OP_Material;
	p[1].i = mode;
	p[2].i = type;
	n = 4;
	if (type == GL_SHININESS)
		n = 1;
	for (i = 0; i < 4; i++)
		p[3 + i].f = v[i];
	for (i = n; i < 4; i++)
		p[3 + i].f = 0;

	gl_add_op(p);
}

void glMaterialf(GLint mode, GLint type, GLfloat v) {
	GLParam p[7];
	GLint i;

	p[0].op = OP_Material;
	p[1].i = mode;
	p[2].i = type;
	p[3].f = v;
	for (i = 0; i < 3; i++)
		p[4 + i].f = 0;

	gl_add_op(p);
}

void glColorMaterial(GLint mode, GLint type) {
	GLParam p[3];

	p[0].op = OP_ColorMaterial;
	p[1].i = mode;
	p[2].i = type;

	gl_add_op(p);
}

void glLightfv(GLint light, GLint type, GLfloat* v) {
	GLParam p[7];
	GLint i;

	p[0].op = OP_Light;
	p[1].i = light;
	p[2].i = type;
	/* TODO: 3 composants ? */
	for (i = 0; i < 4; i++)
		p[3 + i].f = v[i];

	gl_add_op(p);
}

void glLightf(GLint light, GLint type, GLfloat v) {
	GLParam p[7];
	GLint i;

	p[0].op = OP_Light;
	p[1].i = light;
	p[2].i = type;
	p[3].f = v;
	for (i = 0; i < 3; i++)
		p[4 + i].f = 0;

	gl_add_op(p);
}

void glLightModeli(GLint pname, GLint param) {
	GLParam p[6];

	p[0].op = OP_LightModel;
	p[1].i = pname;
	p[2].f = (GLfloat)param;
	//  for(i=0;i<4;i++) p[3+i].f=0;
	//  for(i=0;i<3;i++) p[3+i].f=0;
	p[3].f = 0;
	p[4].f = 0;
	p[5].f = 0;
	gl_add_op(p);
}

void glLightModelfv(GLint pname, GLfloat* param) {
	GLParam p[6];
	GLint i;

	p[0].op = OP_LightModel;
	p[1].i = pname;
	for (i = 0; i < 4; i++)
		p[2 + i].f = param[i];

	gl_add_op(p);
}

/* clear */

void glClear(GLint mask) {
	GLParam p[2];

	p[0].op = OP_Clear;
	p[1].i = mask;

	gl_add_op(p);
}

void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	GLParam p[5];

	p[0].op = OP_ClearColor;
	p[1].f = r;
	p[2].f = g;
	p[3].f = b;
	p[4].f = a;

	gl_add_op(p);
}

void glClearDepth(GLdouble depth) {
	GLParam p[2];

	p[0].op = OP_ClearDepth;
	p[1].f = depth;

	gl_add_op(p);
}

/* textures */

void glTexImage2D(GLint target, GLint level, GLint components, GLint width, GLint height, GLint border, GLint format, GLint type, void* pixels) {
	GLParam p[10];

	p[0].op = OP_TexImage2D;
	p[1].i = target;
	p[2].i = level;
	p[3].i = components;
	p[4].i = width;
	p[5].i = height;
	p[6].i = border;
	p[7].i = format;
	p[8].i = type;
	p[9].p = pixels;

	gl_add_op(p);
}

void glBindTexture(GLint target, GLint texture) {
	GLParam p[3];

	p[0].op = OP_BindTexture;
	p[1].i = target;
	p[2].i = texture;

	gl_add_op(p);
}

void glTexEnvi(GLint target, GLint pname, GLint param) {
	GLParam p[8];

	p[0].op = OP_TexEnv;
	p[1].i = target;
	p[2].i = pname;
	p[3].i = param;
	p[4].f = 0;
	p[5].f = 0;
	p[6].f = 0;
	p[7].f = 0;

	gl_add_op(p);
}

void glTexParameteri(GLint target, GLint pname, GLint param) {
	GLParam p[8];

	p[0].op = OP_TexParameter;
	p[1].i = target;
	p[2].i = pname;
	p[3].i = param;
	p[4].f = 0;
	p[5].f = 0;
	p[6].f = 0;
	p[7].f = 0;

	gl_add_op(p);
}

void glPixelStorei(GLint pname, GLint param) {
	GLParam p[3];

	p[0].op = OP_PixelStore;
	p[1].i = pname;
	p[2].i = param;

	gl_add_op(p);
}

/* selection */

void glInitNames(void) {
	GLParam p[1];

	p[0].op = OP_InitNames;

	gl_add_op(p);
}

void glPushName(GLuint name) {
	GLParam p[2];

	p[0].op = OP_PushName;
	p[1].i = name;

	gl_add_op(p);
}

void glPopName(void) {
	GLParam p[1];

	p[0].op = OP_PopName;

	gl_add_op(p);
}

void glLoadName(GLuint name) {
	GLParam p[2];

	p[0].op = OP_LoadName;
	p[1].i = name;

	gl_add_op(p);
}

void glPolygonOffset(GLfloat factor, GLfloat units) {
	GLParam p[3];
	p[0].op = OP_PolygonOffset;
	p[1].f = factor;
	p[2].f = units;
	gl_add_op(p);
}

/* Special Functions */

void glCallList(GLuint list) {
	GLParam p[2];

	p[0].op = OP_CallList;
	p[1].i = list;

	gl_add_op(p);
}

void glFlush(void) { /* nothing to do */
}

void glHint(GLint target, GLint mode) {
	GLParam p[3];

	p[0].op = OP_Hint;
	p[1].i = target;
	p[2].i = mode;

	gl_add_op(p);
}

/* Non standard functions */

void glDebug(GLint mode) {
	GLContext* c = gl_get_context();
	c->print_flag = mode;
}
