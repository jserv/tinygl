// tgl_math.h

#ifndef __TGL_MAT_H_
#define __TGL_MAT_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Matrix & Vertex */

typedef struct {
    float m[4][4];
} M4;

typedef struct {
    float m[3][3];
} M3;

typedef struct {
    float m[3][4];
} M34;

typedef struct {
    float X,Y,Z;
} V3;

typedef struct {
    float X,Y,Z,W;
} V4;

void gl_M4_Id(M4 *a);
int gl_M4_IsId(M4 *a);
void gl_M4_Move(M4 *a,M4 *b);
void gl_MoveV3(V3 *a,V3 *b);
void gl_MulM4V3(V3 *a,M4 *b,V3 *c);
void gl_MulM3V3(V3 *a,M4 *b,V3 *c);

void gl_M4_MulV4(V4 * a,M4 *b,V4 * c);
void gl_M4_InvOrtho(M4 *a,M4 b);
void gl_M4_Inv(M4 *a,M4 *b);
void gl_M4_Mul(M4 *c,M4 *a,M4 *b);
void gl_M4_MulLeft(M4 *c,M4 *a);
void gl_M4_Transpose(M4 *a,M4 *b);
void gl_M4_Rotate(M4 *c,float t,int u);
int  gl_V3_Norm(V3 *a);

V3 gl_V3_New(float x,float y,float z);
V4 gl_V4_New(float x,float y,float z,float w);

int gl_Matrix_Inv(float *r,float *m,int n);

#endif // __TGL_MAT_H_

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
