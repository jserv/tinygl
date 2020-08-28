// tgl_zbuffer.h

#ifndef __TGL_ZBUFFER_H_
#define __TGL_ZBUFFER_H_

//
// configure
//

/*
 * Matrix of internal and external pixel formats supported. 'Y' means
 * supported.
 * 
 *           External  8    16    24    32
 * Internal 
 *  15                 .     .     .     .
 *  16                 Y     Y     Y     Y
 *  24                 .     Y     Y     .
 *  32                 .     Y     .     Y
 * 
 *
 * 15 bpp does not work yet (although it is easy to add it - ask me if
 * you need it).
 * 
 * Internal pixel format: see TGL_FEATURE_RENDER_BITS
 * External pixel format: see TGL_FEATURE_xxx_BITS 
 */

/* enable various convertion code from internal pixel format (usually
   16 bits per pixel) to any external format */
#define TGL_FEATURE_8_BITS         1
#define TGL_FEATURE_16_BITS        1
#define TGL_FEATURE_24_BITS        1
#define TGL_FEATURE_32_BITS        1

// internal format:
//#define TGL_FEATURE_RENDER_BITS    15
#define TGL_FEATURE_RENDER_BITS    16
//#define TGL_FEATURE_RENDER_BITS    24
//#define TGL_FEATURE_RENDER_BITS    32 

/*
 * Z buffer
 */

#define ZB_Z_BITS 16

#define ZB_POINT_Z_FRAC_BITS 14

#define ZB_POINT_UV_MIN ( (1<<13) )
#define ZB_POINT_UV_MAX ( (1<<22)-(1<<13) )

#define ZB_POINT_RED_MIN ( (1<<10) )
#define ZB_POINT_RED_MAX ( (1<<16)-(1<<10) )
#define ZB_POINT_GREEN_MIN ( (1<<9) )
#define ZB_POINT_GREEN_MAX ( (1<<16)-(1<<9) )
#define ZB_POINT_BLUE_MIN ( (1<<10) )
#define ZB_POINT_BLUE_MAX ( (1<<16)-(1<<10) )

/* display modes */
#define ZB_MODE_5R6G5B  1  /* true color 16 bits */
#define ZB_MODE_INDEX   2  /* color index 8 bits */
#define ZB_MODE_RGBA    3  /* 32 bit rgba mode */
#define ZB_MODE_RGB24   4  /* 24 bit rgb mode */
#define ZB_NB_COLORS    225 /* number of colors for 8 bit display */

#if TGL_FEATURE_RENDER_BITS == 15

#define RGB_TO_PIXEL(r,g,b) ( (((r) >> 1) & 0x7c00) | (((g) >> 6) & 0x03e0) | ((b) >> 11) )
typedef unsigned short PIXEL;
/* bytes per pixel */
#define PSZB 2 
/* bits per pixel = (1 << PSZSH) */
#define PSZSH 4 

#elif TGL_FEATURE_RENDER_BITS == 16

/* 16 bit mode */
#define RGB_TO_PIXEL(r,g,b) ( ((r) & 0xF800) | (((g) >> 5) & 0x07E0) | ((b) >> 11) )
typedef unsigned short PIXEL;
#define PSZB 2 
#define PSZSH 4 

#elif TGL_FEATURE_RENDER_BITS == 24

#define RGB_TO_PIXEL(r,g,b) ( (((r) << 8) & 0xff0000) | ((g) & 0xff00) | ((b) >> 8) )
typedef unsigned char PIXEL;
#define PSZB 3
#define PSZSH 5

#elif TGL_FEATURE_RENDER_BITS == 32

#define RGB_TO_PIXEL(r,g,b) ( (((r) << 8) & 0xff0000) | ((g) & 0xff00) | ((b) >> 8) )
typedef unsigned int PIXEL;
#define PSZB 4
#define PSZSH 5

#else

# error Incorrect number of bits per pixel

#endif

typedef struct
{
    int xsize,ysize;
    int linesize; /* line size, in bytes */
    int mode;
    
    unsigned short *zbuf;
    PIXEL *pbuf;
    int frame_buffer_allocated;
    
    int nb_colors;
    unsigned char *dctable;
    int *ctable;

    PIXEL *current_texture;

	// anchor 2017.02.26
	unsigned short shift[2]; 
	unsigned int uvmask;
} ZBuffer;

typedef struct
{
  int x,y,z;     /* integer coordinates in the zbuffer */
  int s,t;       /* coordinates for the mapping */
  int r,g,b;     /* color indexes */
  
  float sz,tz;   /* temporary coordinates for mapping */
} ZBufferPoint;

ZBuffer *ZB_open(int xsize,int ysize,int mode,int nb_colors,unsigned char *color_indexes,int *color_table,void *frame_buffer);
void ZB_close(ZBuffer *zb);
void ZB_resize(ZBuffer *zb,void *frame_buffer,int xsize,int ysize);
void ZB_clear(ZBuffer *zb,int clear_z,int z,int clear_color,int r,int g,int b);
/* linesize is in BYTES */
void ZB_copyFrameBuffer(ZBuffer *zb,void *buf,int linesize);

// zdither.cpp

void ZB_initDither(ZBuffer *zb,int nb_colors,unsigned char *color_indexes,int *color_table);
void ZB_closeDither(ZBuffer *zb);
void ZB_ditherFrameBuffer(ZBuffer *zb,unsigned char *dest,int linesize);

#endif // __TGL_ZBUFFER_H_
