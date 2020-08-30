// tgl_zbuffer.h

#ifndef __TGL_ZBUFFER_H_
#define __TGL_ZBUFFER_H_

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


#define RGB_TO_PIXEL(r,g,b) ( (((r) << 8) & 0xff0000) | ((g) & 0xff00) | ((b) >> 8) )
typedef unsigned int PIXEL;
#define PSZB 4
#define PSZSH 5


typedef struct {
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

typedef struct {
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

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
