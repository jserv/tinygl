#include "../include/zbuffer.h"
#include <stdlib.h>

#define ZCMP(z, zpix) (!(zb->depth_test) || z >= (zpix))

/* TODO: Implement blending for lines.*/

void ZB_plot(ZBuffer* zb, ZBufferPoint* p) {
	GLushort* pz;
	PIXEL* pp;
	GLint zz;
	//	PIXEL col;
	pz = zb->zbuf + (p->y * zb->xsize + p->x);
	pp = (PIXEL*)((GLbyte*)zb->pbuf + zb->linesize * p->y + p->x * PSZB);
	zz = p->z >> ZB_POINT_Z_FRAC_BITS;
	if (ZCMP(zz, *pz)) {
		//*pp =
		TGL_BLEND_FUNC_RGB(p->r, p->g, p->b, (*pp))
		if(zb->depth_write)
			*pz = zz;
	}
}

#define INTERP_Z
static void ZB_line_flat_z(ZBuffer* zb, ZBufferPoint* p1, ZBufferPoint* p2, GLint color) {
#include "zline.h"
}

/* line with color GLinterpolation */
#define INTERP_Z
#define INTERP_RGB
static void ZB_line_interp_z(ZBuffer* zb, ZBufferPoint* p1, ZBufferPoint* p2) {
#include "zline.h"
}

/* no Z GLinterpolation */

static void ZB_line_flat(ZBuffer* zb, ZBufferPoint* p1, ZBufferPoint* p2, GLint color) {
#include "zline.h"
}

#define INTERP_RGB
static void ZB_line_interp(ZBuffer* zb, ZBufferPoint* p1, ZBufferPoint* p2) {
#include "zline.h"
}

void ZB_line_z(ZBuffer* zb, ZBufferPoint* p1, ZBufferPoint* p2) {
	GLint color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	/* choose if the line should have its color GLinterpolated or not */
	if (color1 == color2) {
		ZB_line_flat_z(zb, p1, p2, color1);
	} else {
		ZB_line_interp_z(zb, p1, p2);
	}
}

void ZB_line(ZBuffer* zb, ZBufferPoint* p1, ZBufferPoint* p2) {
	GLint color1, color2;

	color1 = RGB_TO_PIXEL(p1->r, p1->g, p1->b);
	color2 = RGB_TO_PIXEL(p2->r, p2->g, p2->b);

	/* choose if the line should have its color GLinterpolated or not */
	if (color1 == color2) {
		ZB_line_flat(zb, p1, p2, color1);
	} else {
		ZB_line_interp(zb, p1, p2);
	}
}
