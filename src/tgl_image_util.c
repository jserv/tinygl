// tgl_image_util.cpp

#include "tgl.h"

void gl_convertRGB24_to_RGB16(unsigned short *pixmap, unsigned char *rgb, int xsize, int ysize)
{ // R5G6B5
	int i,n;
	unsigned char *p;

	p = rgb;
	n = xsize*ysize;
	for(i=0;i<n;i++)
	{
		pixmap[i] = ((p[0]&0xF8)<<8) | ((p[1]&0xFC)<<3) | ((p[2]&0xF8)>>3); 
		p += 3;
	}
}

void gl_convertRGB24_to_ARGB32(unsigned int *pixmap, unsigned char *rgb, int xsize, int ysize)
{
	int i,n;
	unsigned char *p;

	p = rgb;
	n = xsize*ysize;
	for(i=0;i<n;i++)
	{
		pixmap[i] = (((unsigned int)p[0])<<16) | (((unsigned int)p[1])<<8) | (((unsigned int)p[2])); 
		p += 3;
	}
}

void gl_convertARGB32_to_RGB24(unsigned char *rgb, unsigned char *rgba, int xsize, int ysize)
{
	int i,n = xsize*ysize;
	for(i=0;i<n;i++)
	{
		rgb[0] = (unsigned char)(rgba[1] * (rgba[0]/255.f));
		rgb[1] = (unsigned char)(rgba[2] * (rgba[0]/255.f));
		rgb[2] = (unsigned char)(rgba[3] * (rgba[0]/255.f));
		rgb += 3;
		rgba += 4;
	}
}

void gl_convertRGBA32_to_RGB24(unsigned char *rgb, unsigned char *rgba, int xsize, int ysize)
{
	int i,n = xsize*ysize;
	for(i=0;i<n;i++)
	{
		rgb[0] = (unsigned char)(rgba[0] * (rgba[3]/255.f));
		rgb[1] = (unsigned char)(rgba[1] * (rgba[3]/255.f));
		rgb[2] = (unsigned char)(rgba[2] * (rgba[3]/255.f));
		rgb += 3;
		rgba += 4;
	}
}

/*
* linear interpolation with xf,yf normalized to 2^16
*/

#define INTERP_NORM_BITS  16
#define INTERP_NORM       (1 << INTERP_NORM_BITS)

int image_util_interpolate(int v00, int v01, int v10, int xf, int yf)
{
	return ( v00+(((v01-v00)*xf + (v10-v00)*yf) >> INTERP_NORM_BITS) );
}

/* 
* TODO: more accurate resampling 
*/

void gl_resizeImage(unsigned char *dest, int xsize_dest, int ysize_dest,
					unsigned char *src, int xsize_src, int ysize_src)
{
	unsigned char *pix,*pix_src;
	float x1,y1,x1inc,y1inc;
	int xi,yi,j,xf,yf,x,y;

	pix = dest;
	pix_src = src;

	x1inc = (float) (xsize_src - 1) / (float) (xsize_dest - 1);
	y1inc = (float) (ysize_src - 1) / (float) (ysize_dest - 1);

	y1 = 0;
	for(y=0;y<ysize_dest;y++)
	{
		x1 = 0;
		for(x=0;x<xsize_dest;x++)
		{
			xi = (int) x1;
			yi = (int) y1;
			xf = (int) ((x1 - floor(x1)) * INTERP_NORM);
			yf = (int) ((y1 - floor(y1)) * INTERP_NORM);

			if ( (xf+yf) <= INTERP_NORM )
			{
				for(j=0;j<3;j++)
				{
					pix[j]=image_util_interpolate(pix_src[(yi*xsize_src+xi)*3+j],
						pix_src[(yi*xsize_src+xi+1)*3+j],
						pix_src[((yi+1)*xsize_src+xi)*3+j],
						xf,yf);
				}
			}
			else
			{
				xf = INTERP_NORM - xf;
				yf = INTERP_NORM - yf;
				for(j=0;j<3;j++)
				{
					pix[j]=image_util_interpolate(pix_src[((yi+1)*xsize_src+xi+1)*3+j],
						pix_src[((yi+1)*xsize_src+xi)*3+j],
						pix_src[(yi*xsize_src+xi+1)*3+j],
						xf,yf);
				}
			}

			pix += 3;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}

#define FRAC_BITS 16

/* resizing with no interlating nor nearest pixel */

void gl_resizeImageNoInterpolate(unsigned char *dest, int xsize_dest, int ysize_dest,
								 unsigned char *src, int xsize_src, int ysize_src)
{
	unsigned char *pix,*pix_src,*pix1;
	int x1,y1,x1inc,y1inc;
	int xi,yi,x,y;

	pix = dest;
	pix_src = src;

	x1inc = (int)((float) ((xsize_src)<<FRAC_BITS) / (float) (xsize_dest));
	y1inc = (int)((float) ((ysize_src)<<FRAC_BITS) / (float) (ysize_dest));

	y1 = 0;
	for(y=0;y<ysize_dest;y++)
	{
		x1 = 0;
		for(x=0;x<xsize_dest;x++)
		{
			xi = x1 >> FRAC_BITS;
			yi = y1 >> FRAC_BITS;
			pix1 = pix_src+(yi*xsize_src+xi)*3;

			pix[0] = pix1[0];
			pix[1] = pix1[1];
			pix[2] = pix1[2];

			pix += 3;
			x1 += x1inc;
		}
		y1 += y1inc;
	}
}
