/*
 * Template-based triangle rasterizer with compile-time feature selection.
 *
 * This file generates specialized triangle rasterization functions for all
 * combinations of depth_test and depth_write flags, eliminating per-pixel
 * runtime branching in the hot rasterization loops.
 *
 * Each base function type generates 4 variants:
 *   _DT0_DW0: depth_test=off, depth_write=off
 *   _DT0_DW1: depth_test=off, depth_write=on
 *   _DT1_DW0: depth_test=on, depth_write=off
 *   _DT1_DW1: depth_test=on, depth_write=on
 */

#include <stdlib.h>
#include "msghandling.h"
#include "zbuffer.h"
#include "ztriangle_variants.h"

#if (TGL_FEATURE_RENDER_BITS != 32) && (TGL_FEATURE_RENDER_BITS != 16)
#error "Incorrect render bits"
#endif

/* Polygon stipple support */
#if TGL_HAS(POLYGON_STIPPLE)
#define TGL_STIPPLEVARS                             \
    GLubyte *zbstipplepattern = zb->stipplepattern; \
    GLubyte zbdostipple = zb->dostipple;
#define THE_X ((GLint) (pp - pp1))
#define XSTIP(_a) ((THE_X + _a) & TGL_POLYGON_STIPPLE_MASK_X)
#define YSTIP (the_y & TGL_POLYGON_STIPPLE_MASK_Y)
#define STIPBIT(_a)                                                       \
    (zbstipplepattern                                                     \
         [(XSTIP(_a) | (YSTIP << TGL_POLYGON_STIPPLE_POW2_WIDTH)) >> 3] & \
     (1 << (XSTIP(_a) & 7)))
#define STIPTEST(_a) &&(!(zbdostipple && !STIPBIT(_a)))
#else
#define TGL_STIPPLEVARS
#define STIPTEST(_a)
#endif

/* No-draw color support */
#if TGL_HAS(NO_DRAW_COLOR)
#define NODRAWTEST(c) &&((c & TGL_COLOR_MASK) != TGL_NO_DRAW_COLOR)
#else
#define NODRAWTEST(c)
#endif

/* Texture setup */
void ZB_setTexture(ZBuffer *zb, PIXEL *texture)
{
    zb->current_texture = texture;
}

/*
 * ============================================================================
 * Flat shaded triangle - with blending
 * ============================================================================
 */

/* Variant DT0_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlat_DT0_DW0(ZBuffer *zb,
                                 ZBufferPoint *p0,
                                 ZBufferPoint *p1,
                                 ZBufferPoint *p2)
{
    GLuint color;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT()                                \
    {                                              \
        color = RGB_TO_PIXEL(p2->r, p2->g, p2->b); \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        /* DT=0: always pass depth test */              \
        if (1 STIPTEST(_a)) {                           \
            TGL_BLEND_FUNC(color, (pp[_a]))             \
            /* DW=0: no depth write */                  \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/* Variant DT0_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlat_DT0_DW1(ZBuffer *zb,
                                 ZBufferPoint *p0,
                                 ZBufferPoint *p1,
                                 ZBufferPoint *p2)
{
    GLuint color;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT()                                \
    {                                              \
        color = RGB_TO_PIXEL(p2->r, p2->g, p2->b); \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        /* DT=0: always pass depth test */              \
        if (1 STIPTEST(_a)) {                           \
            TGL_BLEND_FUNC(color, (pp[_a]))             \
            /* DW=1: always write depth */              \
            pz[_a] = zz;                                \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlat_DT1_DW0(ZBuffer *zb,
                                 ZBufferPoint *p0,
                                 ZBufferPoint *p1,
                                 ZBufferPoint *p2)
{
    GLuint color;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT()                                \
    {                                              \
        color = RGB_TO_PIXEL(p2->r, p2->g, p2->b); \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        /* DT=1: test depth */                          \
        if ((zz >= pz[_a]) STIPTEST(_a)) {              \
            TGL_BLEND_FUNC(color, (pp[_a]))             \
            /* DW=0: no depth write */                  \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlat_DT1_DW1(ZBuffer *zb,
                                 ZBufferPoint *p0,
                                 ZBufferPoint *p1,
                                 ZBufferPoint *p2)
{
    GLuint color;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT()                                \
    {                                              \
        color = RGB_TO_PIXEL(p2->r, p2->g, p2->b); \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        /* DT=1: test depth */                          \
        if ((zz >= pz[_a]) STIPTEST(_a)) {              \
            TGL_BLEND_FUNC(color, (pp[_a]))             \
            /* DW=1: always write depth */              \
            pz[_a] = zz;                                \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/*
 * ============================================================================
 * Flat shaded triangle - no blending
 * ============================================================================
 */

/* Variant DT0_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlatNOBLEND_DT0_DW0(ZBuffer *zb,
                                        ZBufferPoint *p0,
                                        ZBufferPoint *p1,
                                        ZBufferPoint *p2)
{
    PIXEL color = RGB_TO_PIXEL(p2->r, p2->g, p2->b);
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if (1 STIPTEST(_a)) {                           \
            pp[_a] = color;                             \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/* Variant DT0_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlatNOBLEND_DT0_DW1(ZBuffer *zb,
                                        ZBufferPoint *p0,
                                        ZBufferPoint *p1,
                                        ZBufferPoint *p2)
{
    PIXEL color = RGB_TO_PIXEL(p2->r, p2->g, p2->b);
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if (1 STIPTEST(_a)) {                           \
            pp[_a] = color;                             \
            pz[_a] = zz;                                \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlatNOBLEND_DT1_DW0(ZBuffer *zb,
                                        ZBufferPoint *p0,
                                        ZBufferPoint *p1,
                                        ZBufferPoint *p2)
{
    PIXEL color = RGB_TO_PIXEL(p2->r, p2->g, p2->b);
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if ((zz >= pz[_a]) STIPTEST(_a)) {              \
            pp[_a] = color;                             \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleFlatNOBLEND_DT1_DW1(ZBuffer *zb,
                                        ZBufferPoint *p0,
                                        ZBufferPoint *p1,
                                        ZBufferPoint *p2)
{
    PIXEL color = RGB_TO_PIXEL(p2->r, p2->g, p2->b);
    TGL_STIPPLEVARS

#define INTERP_Z

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if ((zz >= pz[_a]) STIPTEST(_a)) {              \
            pp[_a] = color;                             \
            pz[_a] = zz;                                \
        }                                               \
        z += dzdx;                                      \
    }

#include "ztriangle.h"
}

/*
 * ============================================================================
 * Smooth shaded triangle - with blending
 * ============================================================================
 */

#define SAR_RND_TO_ZERO(v, n) (v / (1 << n))

/* Variant DT0_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmooth_DT0_DW0(ZBuffer *zb,
                                   ZBufferPoint *p0,
                                   ZBufferPoint *p1,
                                   ZBufferPoint *p2)
{
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                    \
    {                                                    \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;  \
        if (1 STIPTEST(_a)) {                            \
            TGL_BLEND_FUNC_RGB(or1, og1, ob1, (pp[_a])); \
        }                                                \
        z += dzdx;                                       \
        og1 += dgdx;                                     \
        or1 += drdx;                                     \
        ob1 += dbdx;                                     \
    }

#include "ztriangle.h"
}

/* Variant DT0_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmooth_DT0_DW1(ZBuffer *zb,
                                   ZBufferPoint *p0,
                                   ZBufferPoint *p1,
                                   ZBufferPoint *p2)
{
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                    \
    {                                                    \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;  \
        if (1 STIPTEST(_a)) {                            \
            TGL_BLEND_FUNC_RGB(or1, og1, ob1, (pp[_a])); \
            pz[_a] = zz;                                 \
        }                                                \
        z += dzdx;                                       \
        og1 += dgdx;                                     \
        or1 += drdx;                                     \
        ob1 += dbdx;                                     \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmooth_DT1_DW0(ZBuffer *zb,
                                   ZBufferPoint *p0,
                                   ZBufferPoint *p1,
                                   ZBufferPoint *p2)
{
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                    \
    {                                                    \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;  \
        if ((zz >= pz[_a]) STIPTEST(_a)) {               \
            TGL_BLEND_FUNC_RGB(or1, og1, ob1, (pp[_a])); \
        }                                                \
        z += dzdx;                                       \
        og1 += dgdx;                                     \
        or1 += drdx;                                     \
        ob1 += dbdx;                                     \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmooth_DT1_DW1(ZBuffer *zb,
                                   ZBufferPoint *p0,
                                   ZBufferPoint *p1,
                                   ZBufferPoint *p2)
{
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                    \
    {                                                    \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;  \
        if ((zz >= pz[_a]) STIPTEST(_a)) {               \
            TGL_BLEND_FUNC_RGB(or1, og1, ob1, (pp[_a])); \
            pz[_a] = zz;                                 \
        }                                                \
        z += dzdx;                                       \
        og1 += dgdx;                                     \
        or1 += drdx;                                     \
        ob1 += dbdx;                                     \
    }

#include "ztriangle.h"
}

/*
 * ============================================================================
 * Smooth shaded triangle - no blending
 * ============================================================================
 */

/* Variant DT0_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmoothNOBLEND_DT0_DW0(ZBuffer *zb,
                                          ZBufferPoint *p0,
                                          ZBufferPoint *p1,
                                          ZBufferPoint *p2)
{
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if (1 STIPTEST(_a)) {                           \
            pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);       \
        }                                               \
        z += dzdx;                                      \
        og1 += dgdx;                                    \
        or1 += drdx;                                    \
        ob1 += dbdx;                                    \
    }

#include "ztriangle.h"
}

/* Variant DT0_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmoothNOBLEND_DT0_DW1(ZBuffer *zb,
                                          ZBufferPoint *p0,
                                          ZBufferPoint *p1,
                                          ZBufferPoint *p2)
{
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if (1 STIPTEST(_a)) {                           \
            pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);       \
            pz[_a] = zz;                                \
        }                                               \
        z += dzdx;                                      \
        og1 += dgdx;                                    \
        or1 += drdx;                                    \
        ob1 += dbdx;                                    \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmoothNOBLEND_DT1_DW0(ZBuffer *zb,
                                          ZBufferPoint *p0,
                                          ZBufferPoint *p1,
                                          ZBufferPoint *p2)
{
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if ((zz >= pz[_a]) STIPTEST(_a)) {              \
            pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);       \
        }                                               \
        z += dzdx;                                      \
        og1 += dgdx;                                    \
        or1 += drdx;                                    \
        ob1 += dbdx;                                    \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE

void ZB_fillTriangleSmoothNOBLEND_DT1_DW1(ZBuffer *zb,
                                          ZBufferPoint *p0,
                                          ZBufferPoint *p1,
                                          ZBufferPoint *p2)
{
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_RGB

#define DRAW_INIT() \
    {               \
    }

#define PUT_PIXEL(_a)                                   \
    {                                                   \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS; \
        if ((zz >= pz[_a]) STIPTEST(_a)) {              \
            pp[_a] = RGB_TO_PIXEL(or1, og1, ob1);       \
            pz[_a] = zz;                                \
        }                                               \
        z += dzdx;                                      \
        og1 += dgdx;                                    \
        or1 += drdx;                                    \
        ob1 += dbdx;                                    \
    }

#include "ztriangle.h"
}

/*
 * ============================================================================
 * Texture mapped triangle - common macros
 * ============================================================================
 */

#if TGL_HAS(LIT_TEXTURES)
#define OR1OG1OB1DECL             \
    register GLint or1, og1, ob1; \
    or1 = r1;                     \
    og1 = g1;                     \
    ob1 = b1;
#define OR1G1B1INCR \
    og1 += dgdx;    \
    or1 += drdx;    \
    ob1 += dbdx;
#else
#define OR1OG1OB1DECL
#define OR1G1B1INCR
#define or1 COLOR_MULT_MASK
#define og1 COLOR_MULT_MASK
#define ob1 COLOR_MULT_MASK
#endif

#define NB_INTERP 8

#define DRAW_LINE_TRI_TEXTURED(DEPTH_TEST, DEPTH_WRITE_OP)        \
    {                                                             \
        register GLushort *pz;                                    \
        register PIXEL *pp;                                       \
        register GLuint s, t, z;                                  \
        register GLint n;                                         \
        OR1OG1OB1DECL                                             \
        GLfloat sz, tz, fzl, zinv;                                \
        n = (x2 >> 16) - x1;                                      \
        fzl = (GLfloat) z1;                                       \
        zinv = (GLfloat) (1.0 / fzl);                             \
        pp = (PIXEL *) ((GLbyte *) pp1 + x1 * PSZB);              \
        pz = pz1 + x1;                                            \
        z = z1;                                                   \
        sz = sz1;                                                 \
        tz = tz1;                                                 \
        while (n >= (NB_INTERP - 1)) {                            \
            register GLint dsdx, dtdx;                            \
            {                                                     \
                GLfloat ss, tt;                                   \
                ss = (sz * zinv);                                 \
                tt = (tz * zinv);                                 \
                s = (GLint) ss;                                   \
                t = (GLint) tt;                                   \
                dsdx = (GLint) ((dszdx - ss * fdzdx) * zinv);     \
                dtdx = (GLint) ((dtzdx - tt * fdzdx) * zinv);     \
            }                                                     \
            fzl += fndzdx;                                        \
            /* Newton-Raphson iteration for 1/fzl */              \
            zinv = zinv * (2.0f - fzl * zinv);                    \
            zinv = zinv * (2.0f - fzl * zinv);                    \
            PUT_PIXEL_TEXTURED(0, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(1, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(2, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(3, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(4, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(5, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(6, DEPTH_TEST, DEPTH_WRITE_OP)     \
            PUT_PIXEL_TEXTURED(7, DEPTH_TEST, DEPTH_WRITE_OP)     \
            pz += NB_INTERP;                                      \
            pp += NB_INTERP;                                      \
            n -= NB_INTERP;                                       \
            sz += ndszdx;                                         \
            tz += ndtzdx;                                         \
        }                                                         \
        {                                                         \
            register GLint dsdx, dtdx;                            \
            {                                                     \
                GLfloat ss, tt;                                   \
                ss = (sz * zinv);                                 \
                tt = (tz * zinv);                                 \
                s = (GLint) ss;                                   \
                t = (GLint) tt;                                   \
                dsdx = (GLint) ((dszdx - ss * fdzdx) * zinv);     \
                dtdx = (GLint) ((dtzdx - tt * fdzdx) * zinv);     \
            }                                                     \
            while (n >= 0) {                                      \
                PUT_PIXEL_TEXTURED(0, DEPTH_TEST, DEPTH_WRITE_OP) \
                pz += 1;                                          \
                pp++;                                             \
                n -= 1;                                           \
            }                                                     \
        }                                                         \
    }

/*
 * ============================================================================
 * Texture mapped triangle - with blending
 * ============================================================================
 */

/* Variant DT0_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspective_DT0_DW0(ZBuffer *zb,
                                               ZBufferPoint *p0,
                                               ZBufferPoint *p1,
                                               ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                      \
    {                                                                         \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                       \
        if (1 STIPTEST(_a)) {                                                 \
            TGL_BLEND_FUNC(                                                   \
                RGB_MIX_FUNC(or1, og1, ob1, (TEXTURE_SAMPLE(texture, s, t))), \
                (pp[_a]));                                                    \
        }                                                                     \
        z += dzdx;                                                            \
        s += dsdx;                                                            \
        t += dtdx;                                                            \
        OR1G1B1INCR                                                           \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(0, /* no-op */;) \
    }

#include "ztriangle.h"
}

/* Variant DT0_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspective_DT0_DW1(ZBuffer *zb,
                                               ZBufferPoint *p0,
                                               ZBufferPoint *p1,
                                               ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                      \
    {                                                                         \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                       \
        if (1 STIPTEST(_a)) {                                                 \
            TGL_BLEND_FUNC(                                                   \
                RGB_MIX_FUNC(or1, og1, ob1, (TEXTURE_SAMPLE(texture, s, t))), \
                (pp[_a]));                                                    \
            pz[_a] = zz;                                                      \
        }                                                                     \
        z += dzdx;                                                            \
        s += dsdx;                                                            \
        t += dtdx;                                                            \
        OR1G1B1INCR                                                           \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(0, pz[_a] = zz;) \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspective_DT1_DW0(ZBuffer *zb,
                                               ZBufferPoint *p0,
                                               ZBufferPoint *p1,
                                               ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                      \
    {                                                                         \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                       \
        if ((zz >= pz[_a]) STIPTEST(_a)) {                                    \
            TGL_BLEND_FUNC(                                                   \
                RGB_MIX_FUNC(or1, og1, ob1, (TEXTURE_SAMPLE(texture, s, t))), \
                (pp[_a]));                                                    \
        }                                                                     \
        z += dzdx;                                                            \
        s += dsdx;                                                            \
        t += dtdx;                                                            \
        OR1G1B1INCR                                                           \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(1, /* no-op */;) \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspective_DT1_DW1(ZBuffer *zb,
                                               ZBufferPoint *p0,
                                               ZBufferPoint *p1,
                                               ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_BLEND_VARS
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                      \
    {                                                                         \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                       \
        if ((zz >= pz[_a]) STIPTEST(_a)) {                                    \
            TGL_BLEND_FUNC(                                                   \
                RGB_MIX_FUNC(or1, og1, ob1, (TEXTURE_SAMPLE(texture, s, t))), \
                (pp[_a]));                                                    \
            pz[_a] = zz;                                                      \
        }                                                                     \
        z += dzdx;                                                            \
        s += dsdx;                                                            \
        t += dtdx;                                                            \
        OR1G1B1INCR                                                           \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(1, pz[_a] = zz;) \
    }

#include "ztriangle.h"
}

/*
 * ============================================================================
 * Texture mapped triangle - no blending
 * ============================================================================
 */

/* Variant DT0_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspectiveNOBLEND_DT0_DW0(ZBuffer *zb,
                                                      ZBufferPoint *p0,
                                                      ZBufferPoint *p1,
                                                      ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                    \
    {                                                                       \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                     \
        if (1 STIPTEST(_a)) {                                               \
            pp[_a] =                                                        \
                RGB_MIX_FUNC(or1, og1, ob1, TEXTURE_SAMPLE(texture, s, t)); \
        }                                                                   \
        z += dzdx;                                                          \
        s += dsdx;                                                          \
        t += dtdx;                                                          \
        OR1G1B1INCR                                                         \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(0, /* no-op */;) \
    }

#include "ztriangle.h"
}

/* Variant DT0_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspectiveNOBLEND_DT0_DW1(ZBuffer *zb,
                                                      ZBufferPoint *p0,
                                                      ZBufferPoint *p1,
                                                      ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                    \
    {                                                                       \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                     \
        if (1 STIPTEST(_a)) {                                               \
            pp[_a] =                                                        \
                RGB_MIX_FUNC(or1, og1, ob1, TEXTURE_SAMPLE(texture, s, t)); \
            pz[_a] = zz;                                                    \
        }                                                                   \
        z += dzdx;                                                          \
        s += dsdx;                                                          \
        t += dtdx;                                                          \
        OR1G1B1INCR                                                         \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(0, pz[_a] = zz;) \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW0 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspectiveNOBLEND_DT1_DW0(ZBuffer *zb,
                                                      ZBufferPoint *p0,
                                                      ZBufferPoint *p1,
                                                      ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                    \
    {                                                                       \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                     \
        if ((zz >= pz[_a]) STIPTEST(_a)) {                                  \
            pp[_a] =                                                        \
                RGB_MIX_FUNC(or1, og1, ob1, TEXTURE_SAMPLE(texture, s, t)); \
        }                                                                   \
        z += dzdx;                                                          \
        s += dsdx;                                                          \
        t += dtdx;                                                          \
        OR1G1B1INCR                                                         \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(1, /* no-op */;) \
    }

#include "ztriangle.h"
}

/* Variant DT1_DW1 */
#undef INTERP_Z
#undef INTERP_RGB
#undef INTERP_ST
#undef INTERP_STZ
#undef DRAW_INIT
#undef PUT_PIXEL
#undef DRAW_LINE
#undef PUT_PIXEL_TEXTURED

void ZB_fillTriangleMappingPerspectiveNOBLEND_DT1_DW1(ZBuffer *zb,
                                                      ZBufferPoint *p0,
                                                      ZBufferPoint *p1,
                                                      ZBufferPoint *p2)
{
    PIXEL *texture;
    TGL_STIPPLEVARS

#define INTERP_Z
#define INTERP_STZ
#define INTERP_RGB

#define DRAW_INIT()                    \
    {                                  \
        texture = zb->current_texture; \
        fdzdx = (GLfloat) dzdx;        \
        fndzdx = NB_INTERP * fdzdx;    \
        ndszdx = NB_INTERP * dszdx;    \
        ndtzdx = NB_INTERP * dtzdx;    \
    }

#define PUT_PIXEL_TEXTURED(_a, _dt, _dw)                                    \
    {                                                                       \
        register GLuint zz = z >> ZB_POINT_Z_FRAC_BITS;                     \
        if ((zz >= pz[_a]) STIPTEST(_a)) {                                  \
            pp[_a] =                                                        \
                RGB_MIX_FUNC(or1, og1, ob1, TEXTURE_SAMPLE(texture, s, t)); \
            pz[_a] = zz;                                                    \
        }                                                                   \
        z += dzdx;                                                          \
        s += dsdx;                                                          \
        t += dtdx;                                                          \
        OR1G1B1INCR                                                         \
    }

#define DRAW_LINE()                             \
    {                                           \
        DRAW_LINE_TRI_TEXTURED(1, pz[_a] = zz;) \
    }

#include "ztriangle.h"
}

/*
 * ============================================================================
 * Legacy compatibility functions
 * ============================================================================
 *
 * These functions provide backward compatibility with existing code that
 * calls the original function names. They dispatch to the appropriate
 * specialized variant based on runtime state.
 */

/* Dirty rectangle helper macros */
#if TGL_HAS(DIRTY_RECTANGLE)
#define TGL_MIN3(a, b, c) \
    (((a) < (b)) ? (((a) < (c)) ? (a) : (c)) : (((b) < (c)) ? (b) : (c)))
#define TGL_MAX3(a, b, c) \
    (((a) > (b)) ? (((a) > (c)) ? (a) : (c)) : (((b) > (c)) ? (b) : (c)))

#define MARK_TRIANGLE_DIRTY(zb, p0, p1, p2)               \
    do {                                                  \
        GLint xmin = TGL_MIN3((p0)->x, (p1)->x, (p2)->x); \
        GLint xmax = TGL_MAX3((p0)->x, (p1)->x, (p2)->x); \
        GLint ymin = TGL_MIN3((p0)->y, (p1)->y, (p2)->y); \
        GLint ymax = TGL_MAX3((p0)->y, (p1)->y, (p2)->y); \
        ZB_markDirty((zb), xmin, ymin, xmax, ymax);       \
    } while (0)
#endif

void ZB_fillTriangleFlat(ZBuffer *zb,
                         ZBufferPoint *p0,
                         ZBufferPoint *p1,
                         ZBufferPoint *p2)
{
    GLint dt = zb->depth_test != 0;
    GLint dw = zb->depth_write != 0;
    int idx = (dt << 1) | dw;

#if TGL_HAS(DIRTY_RECTANGLE)
    MARK_TRIANGLE_DIRTY(zb, p0, p1, p2);
#endif

    switch (idx) {
    case 0:
        ZB_fillTriangleFlat_DT0_DW0(zb, p0, p1, p2);
        break;
    case 1:
        ZB_fillTriangleFlat_DT0_DW1(zb, p0, p1, p2);
        break;
    case 2:
        ZB_fillTriangleFlat_DT1_DW0(zb, p0, p1, p2);
        break;
    case 3:
        ZB_fillTriangleFlat_DT1_DW1(zb, p0, p1, p2);
        break;
    }
}

void ZB_fillTriangleFlatNOBLEND(ZBuffer *zb,
                                ZBufferPoint *p0,
                                ZBufferPoint *p1,
                                ZBufferPoint *p2)
{
    GLint dt = zb->depth_test != 0;
    GLint dw = zb->depth_write != 0;
    int idx = (dt << 1) | dw;

#if TGL_HAS(DIRTY_RECTANGLE)
    MARK_TRIANGLE_DIRTY(zb, p0, p1, p2);
#endif

    switch (idx) {
    case 0:
        ZB_fillTriangleFlatNOBLEND_DT0_DW0(zb, p0, p1, p2);
        break;
    case 1:
        ZB_fillTriangleFlatNOBLEND_DT0_DW1(zb, p0, p1, p2);
        break;
    case 2:
        ZB_fillTriangleFlatNOBLEND_DT1_DW0(zb, p0, p1, p2);
        break;
    case 3:
        ZB_fillTriangleFlatNOBLEND_DT1_DW1(zb, p0, p1, p2);
        break;
    }
}

void ZB_fillTriangleSmooth(ZBuffer *zb,
                           ZBufferPoint *p0,
                           ZBufferPoint *p1,
                           ZBufferPoint *p2)
{
    GLint dt = zb->depth_test != 0;
    GLint dw = zb->depth_write != 0;
    int idx = (dt << 1) | dw;

#if TGL_HAS(DIRTY_RECTANGLE)
    MARK_TRIANGLE_DIRTY(zb, p0, p1, p2);
#endif

    switch (idx) {
    case 0:
        ZB_fillTriangleSmooth_DT0_DW0(zb, p0, p1, p2);
        break;
    case 1:
        ZB_fillTriangleSmooth_DT0_DW1(zb, p0, p1, p2);
        break;
    case 2:
        ZB_fillTriangleSmooth_DT1_DW0(zb, p0, p1, p2);
        break;
    case 3:
        ZB_fillTriangleSmooth_DT1_DW1(zb, p0, p1, p2);
        break;
    }
}

void ZB_fillTriangleSmoothNOBLEND(ZBuffer *zb,
                                  ZBufferPoint *p0,
                                  ZBufferPoint *p1,
                                  ZBufferPoint *p2)
{
    GLint dt = zb->depth_test != 0;
    GLint dw = zb->depth_write != 0;
    int idx = (dt << 1) | dw;

#if TGL_HAS(DIRTY_RECTANGLE)
    MARK_TRIANGLE_DIRTY(zb, p0, p1, p2);
#endif

    switch (idx) {
    case 0:
        ZB_fillTriangleSmoothNOBLEND_DT0_DW0(zb, p0, p1, p2);
        break;
    case 1:
        ZB_fillTriangleSmoothNOBLEND_DT0_DW1(zb, p0, p1, p2);
        break;
    case 2:
        ZB_fillTriangleSmoothNOBLEND_DT1_DW0(zb, p0, p1, p2);
        break;
    case 3:
        ZB_fillTriangleSmoothNOBLEND_DT1_DW1(zb, p0, p1, p2);
        break;
    }
}

void ZB_fillTriangleMappingPerspective(ZBuffer *zb,
                                       ZBufferPoint *p0,
                                       ZBufferPoint *p1,
                                       ZBufferPoint *p2)
{
    GLint dt = zb->depth_test != 0;
    GLint dw = zb->depth_write != 0;
    int idx = (dt << 1) | dw;

#if TGL_HAS(DIRTY_RECTANGLE)
    MARK_TRIANGLE_DIRTY(zb, p0, p1, p2);
#endif

    switch (idx) {
    case 0:
        ZB_fillTriangleMappingPerspective_DT0_DW0(zb, p0, p1, p2);
        break;
    case 1:
        ZB_fillTriangleMappingPerspective_DT0_DW1(zb, p0, p1, p2);
        break;
    case 2:
        ZB_fillTriangleMappingPerspective_DT1_DW0(zb, p0, p1, p2);
        break;
    case 3:
        ZB_fillTriangleMappingPerspective_DT1_DW1(zb, p0, p1, p2);
        break;
    }
}

void ZB_fillTriangleMappingPerspectiveNOBLEND(ZBuffer *zb,
                                              ZBufferPoint *p0,
                                              ZBufferPoint *p1,
                                              ZBufferPoint *p2)
{
    GLint dt = zb->depth_test != 0;
    GLint dw = zb->depth_write != 0;
    int idx = (dt << 1) | dw;

#if TGL_HAS(DIRTY_RECTANGLE)
    MARK_TRIANGLE_DIRTY(zb, p0, p1, p2);
#endif

    switch (idx) {
    case 0:
        ZB_fillTriangleMappingPerspectiveNOBLEND_DT0_DW0(zb, p0, p1, p2);
        break;
    case 1:
        ZB_fillTriangleMappingPerspectiveNOBLEND_DT0_DW1(zb, p0, p1, p2);
        break;
    case 2:
        ZB_fillTriangleMappingPerspectiveNOBLEND_DT1_DW0(zb, p0, p1, p2);
        break;
    case 3:
        ZB_fillTriangleMappingPerspectiveNOBLEND_DT1_DW1(zb, p0, p1, p2);
        break;
    }
}

/*
 * ============================================================================
 * Dispatch table initialization
 * ============================================================================
 */

ZB_TriangleDispatch zb_triangle_dispatch = {
    /* flat with blend */
    .flat = {ZB_fillTriangleFlat_DT0_DW0, ZB_fillTriangleFlat_DT0_DW1,
             ZB_fillTriangleFlat_DT1_DW0, ZB_fillTriangleFlat_DT1_DW1},
    /* flat without blend */
    .flat_noblend = {ZB_fillTriangleFlatNOBLEND_DT0_DW0,
                     ZB_fillTriangleFlatNOBLEND_DT0_DW1,
                     ZB_fillTriangleFlatNOBLEND_DT1_DW0,
                     ZB_fillTriangleFlatNOBLEND_DT1_DW1},
    /* smooth with blend */
    .smooth = {ZB_fillTriangleSmooth_DT0_DW0, ZB_fillTriangleSmooth_DT0_DW1,
               ZB_fillTriangleSmooth_DT1_DW0, ZB_fillTriangleSmooth_DT1_DW1},
    /* smooth without blend */
    .smooth_noblend = {ZB_fillTriangleSmoothNOBLEND_DT0_DW0,
                       ZB_fillTriangleSmoothNOBLEND_DT0_DW1,
                       ZB_fillTriangleSmoothNOBLEND_DT1_DW0,
                       ZB_fillTriangleSmoothNOBLEND_DT1_DW1},
    /* textured with blend */
    .textured = {ZB_fillTriangleMappingPerspective_DT0_DW0,
                 ZB_fillTriangleMappingPerspective_DT0_DW1,
                 ZB_fillTriangleMappingPerspective_DT1_DW0,
                 ZB_fillTriangleMappingPerspective_DT1_DW1},
    /* textured without blend */
    .textured_noblend = {ZB_fillTriangleMappingPerspectiveNOBLEND_DT0_DW0,
                         ZB_fillTriangleMappingPerspectiveNOBLEND_DT0_DW1,
                         ZB_fillTriangleMappingPerspectiveNOBLEND_DT1_DW0,
                         ZB_fillTriangleMappingPerspectiveNOBLEND_DT1_DW1}};
