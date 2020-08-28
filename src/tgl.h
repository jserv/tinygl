// tgl.h

#ifndef __TGL_H_
#define __TGL_H_

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "TGL/gl.h"
#include "tgl_zbuffer.h"
#include "tgl_math.h"

/* Define BYTE_ORDER correctly where missing */
#ifndef BYTE_ORDER
#define LITTLE_ENDIAN	1234
#define BIG_ENDIAN	4321

#if (defined(__i386__) || defined(__i386)) || \
     defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
#define BYTE_ORDER	LITTLE_ENDIAN
#else
#define BYTE_ORDER	BIG_ENDIAN
#endif
#endif

#ifndef M_PI
# define M_PI 3.14159265f
#endif

#ifndef MIN
# define MIN(a, b) (((a) < (b) ? (a) : (b)))
#endif

#ifndef MAX
# define MAX(a, b) (((a) > (b) ? (a) : (b)))
#endif

/* initially # of allocated GLVertexes (will grow when necessary) */
#define POLYGON_MAX_VERTEX 16

/* Max # of specular light pow buffers */
#define MAX_SPECULAR_BUFFERS 8
/* # of entries in specular buffer */
#define SPECULAR_BUFFER_SIZE 1024
/* specular buffer granularity */
#define SPECULAR_BUFFER_RESOLUTION 1024

#define MAX_MODELVIEW_STACK_DEPTH  32
#define MAX_PROJECTION_STACK_DEPTH 8
#define MAX_TEXTURE_STACK_DEPTH    8
#define MAX_NAME_STACK_DEPTH       64
#define MAX_TEXTURE_LEVELS         11
#define MAX_LIGHTS                 16
#define MAX_TEXTURE_SIZE           2048

#define VERTEX_HASH_SIZE 1031

//#define MAX_DISPLAY_LISTS 1024
#define OP_BUFFER_MAX_SIZE 512

#define TGL_OFFSET_FILL    0x1
#define TGL_OFFSET_LINE    0x2
#define TGL_OFFSET_POINT   0x4

#define VERTEX_ARRAY   0x0001
#define COLOR_ARRAY    0x0002
#define NORMAL_ARRAY   0x0004
#define TEXCOORD_ARRAY 0x0008

#define CLIP_XMIN   (1<<0)
#define CLIP_XMAX   (1<<1)
#define CLIP_YMIN   (1<<2)
#define CLIP_YMAX   (1<<3)
#define CLIP_ZMIN   (1<<4)
#define CLIP_ZMAX   (1<<5)

// bitmap.cpp
#define TGL_PIXEL_ENUM GL_UNSIGNED_BYTE
#define TGL_PIXEL_TYPE GLuint

typedef struct GLSpecBuf
{
	int shininess_i;
	int last_used;
	float buf[SPECULAR_BUFFER_SIZE+1];
	struct GLSpecBuf *next;
} GLSpecBuf;

typedef struct GLLight
{
	V4 ambient;
	V4 diffuse;
	V4 specular;
	V4 position;	
	V3 spot_direction;
	float spot_exponent;
	float spot_cutoff;
	float attenuation[3];
	/* precomputed values */
	float cos_spot_cutoff;
	V3 norm_spot_direction;
	V3 norm_position;
	/* we use a linked list to know which are the enabled lights */
	int enabled;
	struct GLLight *next,*prev;
} GLLight;

typedef struct GLMaterial
{
	V4 emission;
	V4 ambient;
	V4 diffuse;
	V4 specular;
	float shininess;

	/* computed values */
	int shininess_i;
	int do_specular;  
} GLMaterial;

typedef struct GLViewport
{
	int xmin,ymin,xsize,ysize;
	V3 scale;
	V3 trans;
	int updated;
} GLViewport;

typedef struct GLVertex
{
	int edge_flag;
	V3 normal;
	V4 coord;
	V4 tex_coord;
	V4 color;

	/* computed values */
	V4 ec;                /* eye coordinates */
	V4 pc;                /* coordinates in the normalized volume */
	int clip_code;        /* clip code */
	ZBufferPoint zp;      /* integer coordinates for the rasterization */
} GLVertex;

typedef struct GLImage
{
	void *pixmap;
	int xsize,ysize;

	// anchor 2017.02.26
	unsigned short shift[2];
	unsigned int uvmask;
} GLImage;

/* textures */

#define TEXTURE_HASH_TABLE_SIZE 256

typedef struct GLTexture
{
	GLImage images[MAX_TEXTURE_LEVELS];
	int handle;
	struct GLTexture *next,*prev;
} GLTexture;

/* shared state */

typedef struct GLSharedState
{
	GLTexture **texture_hash_table;
} GLSharedState;

struct GLContext;

typedef void (*gl_draw_triangle_func)(struct GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);

typedef struct 
{
	float x, y, z;
} GLRasterPos;

typedef struct
{
	float *p;
	int size;
	int stride;
} GLArray;

/* display context */

typedef struct GLContext
{
    /* Z buffer */
    ZBuffer *zb;

    /* shared state */
    GLSharedState shared_state;

    /* viewport */
    GLViewport viewport;

    /* lights */
    struct
	{
        GLLight lights[MAX_LIGHTS];
        GLLight *first;
        struct 
		{
            V4 ambient;
            int local;
            int two_side;
        } model;
        int enabled;
    } light;

    /* materials */
    struct 
	{
        GLMaterial materials[2];
        struct
		{
            int enabled;
            int current_mode;
            int current_type;
        } color;
    } material;

    /* textures */
    struct 
	{
        GLTexture *current;
        int enabled_2d;
    } texture;

    int print_flag; // debug flag

    /* matrix */
    struct 
	{
        int mode;
        M4 *stack[3];
        M4 *stack_ptr[3];
        int stack_depth_max[3];

        M4 model_view_inv;
        M4 model_projection;
        int model_projection_updated;
        int model_projection_no_w_transform;
        int apply_texture;
    } matrix;

    /* current state */
    int polygon_mode_back;
    int polygon_mode_front;

    int current_front_face;
    int current_shade_model;
    int current_cull_face;
    int cull_face_enabled;
    int normalize_enabled;
    gl_draw_triangle_func draw_triangle_front, draw_triangle_back;

    /* clear */
    struct
	{
        float depth;
        V4 color;
    } clear;

    /* glBegin / glEnd */
    int in_begin;
    int begin_type;
    int vertex_n, vertex_cnt;
    int vertex_max;
    GLVertex *vertex;

    /* current vertex state */
    struct
	{
        V4 color;
        unsigned int longcolor[3]; /* precomputed integer color */
        V4 normal;
        V4 tex_coord;
        int edge_flag;
    } current;

    /* opengl 1.1 arrays  */
    struct
	{
        GLArray vertex,
                normal,
                color,
                tex_coord;
    } array;

    int client_states;

    /* opengl 1.1 polygon offset */
    struct
	{
        float factor;
        float units;
        int states;
    } offset;

    /* specular buffer. could probably be shared between contexts, but that wouldn't be 100% thread safe */
    GLSpecBuf *specbuf_first;
    int specbuf_used_counter;
    int specbuf_num_buffers;

    /* opaque structure for user's use */
    void *opaque;

    /* resize viewport function */
    int (*gl_resize_viewport)(struct GLContext *c, int *xsize, int *ysize);

    /* depth test */
    int depth_test;

    struct
	{
        int dfactor;
        int sfactor;
        int enabled;
    } blend;

    struct
	{
        int func;
        int ref;
    } alpha;

    struct
	{
        int op;
    } logic;

    // TODO: glPushAttrib
    GLRasterPos raster_pos;

} GLContext;

extern GLContext *gl_ctx;

// tgl_clip.cpp

void gl_transform_to_viewport(GLContext *c, GLVertex *v);
void gl_draw_triangle(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_line(GLContext *c, GLVertex *p0, GLVertex *p1);
void gl_draw_point(GLContext *c, GLVertex *p0);

void gl_draw_triangle_point(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_line(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);
void gl_draw_triangle_fill(GLContext *c, GLVertex *p0, GLVertex *p1, GLVertex *p2);

// tgl_matrix.cpp

void gl_print_matrix(const float *m);

// tgl_light.cpp

void gl_enable_disable_light(GLContext *c, int light, int v);
void gl_shade_vertex(GLContext *c, GLVertex *v);

// tgl_texture.cpp

void gl_init_textures(GLContext *c);
//void glEndTextures(GLContext *c);
GLTexture *alloc_texture(GLContext *c,int h);
void free_texture(GLContext *c,int h);
GLTexture *find_texture(GLContext *c,int h);

// tgl_image_util.cpp

void gl_convertRGB24_to_RGB16(unsigned short *pixmap,unsigned char *rgb,int xsize,int ysize);
void gl_convertRGB24_to_ARGB32(unsigned int *pixmap, unsigned char *rgb,int xsize, int ysize);
void gl_convertARGB32_to_RGB24(unsigned char *rgb, unsigned char *rgba, int xsize, int ysize);
void gl_convertRGBA32_to_RGB24(unsigned char *rgb, unsigned char *rgba, int xsize, int ysize);
void gl_resizeImage(unsigned char *dest,int xsize_dest,int ysize_dest,unsigned char *src,int xsize_src,int ysize_src);
void gl_resizeImageNoInterpolate(unsigned char *dest,int xsize_dest,int ysize_dest,unsigned char *src,int xsize_src,int ysize_src);

// tgl_init.cpp

GLContext *gl_get_context();

// tgl_specbuf.cpp

GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, const float shininess);

// memory.cpp

void gl_free(void *p);
void *gl_malloc(int size);
void *gl_zalloc(int size);

// msghandling.cpp

typedef void (* logFuncPtr) (const char*, va_list);
extern void tgl_set_log_func(logFuncPtr ptr);
extern void tgl_warning(const char *text, ...);
extern void tgl_info(const char *text, ...);
extern void tgl_fatal_error(const char *format, ...);

// misc
void glDebug(int mode);
void glInit(void *zbuffer);
void glClose(); 

// the only supported glu function

void gluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar ); 

/* this clip epsilon is needed to avoid some rounding errors after several clipping stages */

#define CLIP_EPSILON (1E-5)

static int gl_clipcode(float x,float y,float z,float w1)
{
	float w = w1 * (float) (1.f + CLIP_EPSILON);
	return (x<-w) | ((x>w)<<1) | ((y<-w)<<2) | ((y>w)<<3) | ((z<-w)<<4) | ((z>w)<<5);
}

#endif // __TGL_H_
