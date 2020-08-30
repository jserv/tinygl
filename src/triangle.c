// tgl_triangle.cpp

#include "tgl_triangle.h"

void ZB_fillTriangleFlat(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2)
{
    int color;

#define INTERP_Z


#define DRAW_INIT()				\
{						\
  color = RGB_TO_PIXEL(p2->r,p2->g,p2->b);	\
}

#define PUT_PIXEL(_a)				\
{						\
    zz = z >> ZB_POINT_Z_FRAC_BITS;		\
    if ( ZCMP(zz,pz[_a]) ) \
	{				\
      pp[_a] = color;				\
      pz[_a] = zz;				\
    }						\
    z += dzdx;					\
}

#include "tgl_triangle_inc.h"
}

/*
 * Smooth filled triangle.
 * The code below is very tricky :)
 */

void ZB_fillTriangleSmooth(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2)
{

#define INTERP_Z
#define INTERP_RGB

#define SAR_RND_TO_ZERO(v,n) (v / (1<<n))


#define DRAW_INIT() 				\
{						\
}

#define PUT_PIXEL(_a)				\
{						\
    zz=z >> ZB_POINT_Z_FRAC_BITS;		\
    if (ZCMP(zz,pz[_a])) {				\
      pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);\
      pz[_a]=zz;				\
    }\
    z+=dzdx;					\
    og1+=dgdx;					\
    or1+=drdx;					\
    ob1+=dbdx;					\
}


#include "tgl_triangle_inc.h"
}

void ZB_setTexture(ZBuffer *zb, void *image)
{
    GLImage* _image = (GLImage*) image;
    zb->current_texture = (PIXEL*) _image->pixmap;
    zb->shift[0] = _image->shift[0];
    zb->shift[1] = _image->shift[1];
    zb->uvmask   = _image->uvmask;
}

// used - anchor
void ZB_fillTriangleMapping(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2)
{
    PIXEL *texture;

#define INTERP_Z
#define INTERP_ST

#define DRAW_INIT()				\
{						\
  texture = zb->current_texture;			\
}


#define PUT_PIXEL(_a)				\
{						\
   zz=z >> ZB_POINT_Z_FRAC_BITS;		\
     if (ZCMP(zz,pz[_a])) {				\
       pp[_a]=texture[((t & 0x3FC00000) | s) >> 14];	\
       pz[_a]=zz;				\
    }						\
    z+=dzdx;					\
    s+=dsdx;					\
    t+=dtdx;					\
}


#include "tgl_triangle_inc.h"
}

/*
 * Texture mapping with perspective correction.
 * We use the gradient method to make less divisions.
 * TODO: pipeline the division
 */
#if 1

void ZB_fillTriangleMappingPerspective(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2)
{
    PIXEL *texture;
    float fdzdx,fndzdx,ndszdx,ndtzdx;

    // anchor 2017.02.26
    unsigned short sh1 = zb->shift[0];
    unsigned short sh2 = zb->shift[1];
    unsigned int mask  = zb->uvmask;

#define INTERP_Z
#define INTERP_STZ

#define NB_INTERP 8

#define DRAW_INIT()				\
{						\
	texture = zb->current_texture;\
	fdzdx = (float)dzdx;\
	fndzdx = NB_INTERP * fdzdx;\
	ndszdx = NB_INTERP * dszdx;\
	ndtzdx = NB_INTERP * dtzdx;\
}


#define PUT_PIXEL(_a)				\
{						\
	zz = z >> ZB_POINT_Z_FRAC_BITS;		\
	if ( ZCMP(zz,pz[_a]) ) \
	{				\
		pp[_a] = *(PIXEL *)((char *)texture+ \
			(( ((t & mask)<<sh1) | (s & mask) ) >> sh2)); \
		pz[_a] = zz;				\
	}						\
	z += dzdx;					\
	s += dsdx;					\
	t += dtdx;					\
}


#define DRAW_LINE()				\
{						\
  register unsigned short *pz;		\
  register PIXEL *pp;		\
  register unsigned int s,t,z,zz;	\
  register int n,dsdx,dtdx;		\
  float sz,tz,fz,zinv; \
  n=(x2>>16)-x1;                             \
  fz=(float)z1;\
  zinv=1.f / fz;\
  pp=(PIXEL *)((char *)pp1 + x1 * PSZB); \
  pz=pz1+x1;					\
  z=z1;						\
  sz=sz1;\
  tz=tz1;\
  while (n>=(NB_INTERP-1)) {						   \
    {\
      float ss,tt;\
      ss=(sz * zinv);\
      tt=(tz * zinv);\
      s=(int) ss;\
      t=(int) tt;\
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );\
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );\
      fz+=fndzdx;\
      zinv=1.f / fz;\
    }\
    PUT_PIXEL(0);							   \
    PUT_PIXEL(1);							   \
    PUT_PIXEL(2);							   \
    PUT_PIXEL(3);							   \
    PUT_PIXEL(4);							   \
    PUT_PIXEL(5);							   \
    PUT_PIXEL(6);							   \
    PUT_PIXEL(7);							   \
    pz+=NB_INTERP;							   \
    pp=(PIXEL *)((char *)pp + NB_INTERP * PSZB);\
    n-=NB_INTERP;							   \
    sz+=ndszdx;\
    tz+=ndtzdx;\
  }									   \
    {\
      float ss,tt;\
      ss=(sz * zinv);\
      tt=(tz * zinv);\
      s=(int) ss;\
      t=(int) tt;\
      dsdx= (int)( (dszdx - ss*fdzdx)*zinv );\
      dtdx= (int)( (dtzdx - tt*fdzdx)*zinv );\
    }\
  while (n>=0) {							   \
    PUT_PIXEL(0);							   \
    pz+=1;								   \
    pp=(PIXEL *)((char *)pp + PSZB);\
    n-=1;								   \
  }									   \
}

#include "tgl_triangle_inc.h"
}

#endif

#if 0

/* slow but exact version (only there for reference, incorrect for 24 bits) */

void ZB_fillTriangleMappingPerspective(ZBuffer *zb, ZBufferPoint *p0, ZBufferPoint *p1, ZBufferPoint *p2)
{
    PIXEL *texture;

    // anchor 2017.02.26
    unsigned short rs0 = zb->rshift[0];
    unsigned short rs1 = zb->rshift[1];
    unsigned short rs2 = zb->rshift[2];
    unsigned int uvm0 = zb->uvmask[0];
    unsigned int uvm1 = zb->uvmask[1];

#define INTERP_Z
#define INTERP_STZ

#define DRAW_INIT()				\
{						\
  texture = zb->current_texture;			\
}

#define PUT_PIXEL(_a)				\
{						\
	float zinv; \
	int s,t; \
	zz = z >> ZB_POINT_Z_FRAC_BITS;		\
	if ( ZCMP(zz,pz[_a]) ) \
	{				\
		zinv = 1.f / (float) z; \
		s = (int) (sz * zinv); \
		t = (int) (tz * zinv); \
		pp[_a] = *(PIXEL *)((char *)texture+ \
			(( ((t & uvm0)>>rs0) | ((s & uvm1)>>rs1) ) >> rs2)); \
		pz[_a] = zz;				\
	}						\
	z += dzdx;					\
	sz += dszdx;					\
	tz += dtzdx;					\
}

#include "tgl_triangle_inc.h"
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
