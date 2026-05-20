/*
 * Alpha blending demo - renders to alpha_demo.png
 *
 * Scene: A red background with three overlapping triangles at different
 * alpha levels, plus a textured quad with per-texel alpha.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TGL/gl.h>
#include "zbuffer.h"

#define STBIW_ASSERT(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 512
#define HEIGHT 512
#define TEX_SIZE 256

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Generate a checkerboard RGBA texture with varying alpha */
static void make_checker_texture(unsigned char *rgba)
{
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int idx = (y * TEX_SIZE + x) * 4;
            int check = ((x / 32) + (y / 32)) & 1;
            if (check) {
                /* White squares, alpha varies by row */
                rgba[idx + 0] = 255;
                rgba[idx + 1] = 255;
                rgba[idx + 2] = 255;
                rgba[idx + 3] = (unsigned char)(255 * y / TEX_SIZE);
            } else {
                /* Yellow squares, full alpha */
                rgba[idx + 0] = 255;
                rgba[idx + 1] = 200;
                rgba[idx + 2] = 0;
                rgba[idx + 3] = 200;
            }
        }
    }
}

int main(void)
{
    ZBuffer *zb;
    unsigned char *framebuf;

    /* Initialize */
    zb = ZB_open(WIDTH, HEIGHT, ZB_MODE_RGBA, NULL);
    if (!zb) {
        fprintf(stderr, "Failed to open ZBuffer\n");
        return 1;
    }
    glInit(zb);

    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    /* Clear to dark blue background */
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Enable alpha blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* --- Draw a large opaque red rectangle as background element --- */
    glShadeModel(GL_FLAT);
    glColor4f(0.8f, 0.2f, 0.1f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.9f, -0.9f, 0.0f);
    glVertex3f( 0.9f, -0.9f, 0.0f);
    glVertex3f( 0.9f, -0.2f, 0.0f);
    glVertex3f(-0.9f, -0.2f, 0.0f);
    glEnd();

    /* --- Three overlapping triangles with decreasing alpha --- */

    /* Green triangle, alpha=0.8 (mostly opaque) */
    glShadeModel(GL_FLAT);
    glColor4f(0.0f, 0.9f, 0.2f, 0.8f);
    glBegin(GL_TRIANGLES);
    glVertex3f(-0.7f, -0.8f, 0.0f);
    glVertex3f( 0.0f, -0.8f, 0.0f);
    glVertex3f(-0.35f, 0.0f, 0.0f);
    glEnd();

    /* Blue triangle, alpha=0.5 (half transparent) */
    glColor4f(0.2f, 0.3f, 1.0f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex3f(-0.3f, -0.8f, 0.0f);
    glVertex3f( 0.4f, -0.8f, 0.0f);
    glVertex3f( 0.05f, 0.0f, 0.0f);
    glEnd();

    /* Cyan triangle, alpha=0.25 (mostly transparent) */
    glColor4f(0.0f, 0.9f, 0.9f, 0.25f);
    glBegin(GL_TRIANGLES);
    glVertex3f( 0.1f, -0.8f, 0.0f);
    glVertex3f( 0.8f, -0.8f, 0.0f);
    glVertex3f( 0.45f, 0.0f, 0.0f);
    glEnd();

    /* --- Smooth-shaded triangle with alpha gradient --- */
    glShadeModel(GL_SMOOTH);
    glBegin(GL_TRIANGLES);
    /* Fully opaque magenta at bottom-left */
    glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
    glVertex3f(-0.8f, 0.1f, 0.0f);
    /* Fully opaque magenta at bottom-right */
    glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
    glVertex3f( 0.0f, 0.1f, 0.0f);
    /* Fully transparent magenta at top (fades out) */
    glColor4f(1.0f, 0.0f, 1.0f, 0.0f);
    glVertex3f(-0.4f, 0.9f, 0.0f);
    glEnd();

    /* --- Textured quad with per-texel alpha (checkerboard) --- */
    {
        static unsigned char tex_data[TEX_SIZE * TEX_SIZE * 4];
        GLuint tex;

        make_checker_texture(tex_data);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, TEX_SIZE, TEX_SIZE, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_FLAT);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.1f, 0.1f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.9f, 0.1f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.9f, 0.9f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.1f, 0.9f, 0.0f);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDeleteTextures(1, &tex);
    }

    /* --- Save to PNG --- */
    framebuf = malloc(WIDTH * HEIGHT * 3);
    if (!framebuf) {
        fprintf(stderr, "Out of memory\n");
        ZB_close(zb);
        glClose();
        return 1;
    }

    /* Convert ARGB framebuffer to RGB for PNG */
    {
        unsigned int *pbuf = (unsigned int *)zb->pbuf;
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                unsigned int pix = pbuf[y * WIDTH + x];
                int idx = (y * WIDTH + x) * 3;
                framebuf[idx + 0] = (pix >> 16) & 0xFF; /* R */
                framebuf[idx + 1] = (pix >> 8) & 0xFF;  /* G */
                framebuf[idx + 2] = pix & 0xFF;         /* B */
            }
        }
    }

    if (stbi_write_png("alpha_demo.png", WIDTH, HEIGHT, 3, framebuf,
                       WIDTH * 3)) {
        printf("Saved alpha_demo.png (%dx%d)\n", WIDTH, HEIGHT);
    } else {
        fprintf(stderr, "Failed to write PNG\n");
        free(framebuf);
        ZB_close(zb);
        glClose();
        return 1;
    }

    free(framebuf);
    ZB_close(zb);
    glClose();
    return 0;
}
