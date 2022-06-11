#include "zgl.h"

/*
 * image conversion
 */

void gl_convertRGB_to_5R6G5B(GLushort *pixmap,
                             GLubyte *rgb,
                             GLint xsize,
                             GLint ysize)
{
    GLubyte *p = rgb;
    GLint n = xsize * ysize;
    for (GLint i = 0; i < n; i++) {
        pixmap[i] =
            ((p[0] & 0xF8) << 8) | ((p[1] & 0xFC) << 3) | ((p[2] & 0xF8) >> 3);
        p += 3;
    }
}

/*
 * This actually converts to ARGB.
 * This is the format of the entire engine.
 */
void gl_convertRGB_to_8A8R8G8B(GLuint *pixmap,
                               GLubyte *rgb,
                               GLint xsize,
                               GLint ysize)
{
    GLubyte *p = rgb;
    GLint n = xsize * ysize;
    for (GLint i = 0; i < n; i++) {
        pixmap[i] = (((GLuint) p[0]) << 16) | (((GLuint) p[1]) << 8) |
                    (((GLuint) p[2]));
        p += 3;
    }
}

/*
 * linear GLinterpolation with xf,yf normalized to 2^16
 */

#define INTERP_NORM_BITS 16
#define INTERP_NORM (1 << INTERP_NORM_BITS)

static GLint GLinterpolate_imutil(GLint v00,
                                  GLint v01,
                                  GLint v10,
                                  GLint xf,
                                  GLint yf)
{
    return v00 + (((v01 - v00) * xf + (v10 - v00) * yf) >> INTERP_NORM_BITS);
}

/*
 * TODO: more accurate resampling
 */

void gl_resizeImage(GLubyte *dest,
                    GLint xsize_dest,
                    GLint ysize_dest,
                    GLubyte *src,
                    GLint xsize_src,
                    GLint ysize_src)
{
    GLubyte *pix = dest, *pix_src = src;

    GLfloat x1inc = (GLfloat) (xsize_src - 1) / (GLfloat) (xsize_dest - 1);
    GLfloat y1inc = (GLfloat) (ysize_src - 1) / (GLfloat) (ysize_dest - 1);

    GLfloat y1 = 0;
    for (GLint y = 0; y < ysize_dest; y++) {
        GLfloat x1 = 0;
        for (GLint x = 0; x < xsize_dest; x++) {
            GLint xi = (GLint) x1, yi = (GLint) y1;
            GLint xf = (GLint) ((x1 - floor(x1)) * INTERP_NORM);
            GLint yf = (GLint) ((y1 - floor(y1)) * INTERP_NORM);

            if ((xf + yf) <= INTERP_NORM) {
                for (GLint j = 0; j < 3; j++) {
                    pix[j] = GLinterpolate_imutil(
                        pix_src[(yi * xsize_src + xi) * 3 + j],
                        pix_src[(yi * xsize_src + xi + 1) * 3 + j],
                        pix_src[((yi + 1) * xsize_src + xi) * 3 + j], xf, yf);
                }
            } else {
                xf = INTERP_NORM - xf;
                yf = INTERP_NORM - yf;
                for (GLint j = 0; j < 3; j++) {
                    pix[j] = GLinterpolate_imutil(
                        pix_src[((yi + 1) * xsize_src + xi + 1) * 3 + j],
                        pix_src[((yi + 1) * xsize_src + xi) * 3 + j],
                        pix_src[(yi * xsize_src + xi + 1) * 3 + j], xf, yf);
                }
            }

            pix += 3;
            x1 += x1inc;
        }
        y1 += y1inc;
    }
}

#define FRAC_BITS 16

/* resizing with no GLinterlating nor nearest pixel */

void gl_resizeImageNoInterpolate(GLubyte *dest,
                                 GLint xsize_dest,
                                 GLint ysize_dest,
                                 GLubyte *src,
                                 GLint xsize_src,
                                 GLint ysize_src)
{
    GLubyte *pix = dest, *pix_src = src;

    GLint x1inc =
        (GLint) ((GLfloat) ((xsize_src) << FRAC_BITS) / (GLfloat) (xsize_dest));
    GLint y1inc =
        (GLint) ((GLfloat) ((ysize_src) << FRAC_BITS) / (GLfloat) (ysize_dest));

    GLint y1 = 0;
    for (GLint y = 0; y < ysize_dest; y++) {
        GLint x1 = 0;
        for (GLint x = 0; x < xsize_dest; x++) {
            GLint xi = x1 >> FRAC_BITS, yi = y1 >> FRAC_BITS;
            GLubyte *pix1 = pix_src + (yi * xsize_src + xi) * 3;

            pix[0] = pix1[0];
            pix[1] = pix1[1];
            pix[2] = pix1[2];

            pix += 3;
            x1 += x1inc;
        }
        y1 += y1inc;
    }
}
