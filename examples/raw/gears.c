/* 3D gear wheels written by Brian Paul. */

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TGL/gl.h>
#include "zbuffer.h"

#define STBIW_ASSERT(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static int override_drawmodes = 0;
static GLubyte stipplepattern[128] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA,
    0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55,
    0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA,
    0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55,
    0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA,
    0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55,
    0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA,
    0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55,
    0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
};

/*
 * Draw a gear wheel.  You'll probably want to call this function when
 * building a display list since we do a lot of trig here.
 *
 * Input:  inner_radius - radius of hole at center
 *         outer_radius - radius at center of teeth
 *         width - width of gear
 *         teeth - number of teeth
 *         tooth_depth - depth of tooth
 */
static void gear(GLfloat inner_radius,
                 GLfloat outer_radius,
                 GLfloat width,
                 GLint teeth,
                 GLfloat tooth_depth)
{
    GLfloat r0, r1, r2;
    GLfloat angle, da;
    GLfloat u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    r2 = outer_radius + tooth_depth / 2.0;

    da = 2.0 * M_PI / teeth / 4.0;

    glNormal3f(0.0, 0.0, 1.0);

    /* draw front face */
    if (override_drawmodes == 1)
        glBegin(GL_LINES);
    else if (override_drawmodes == 2)
        glBegin(GL_POINTS);
    else
        glBegin(GL_QUAD_STRIP);
    for (GLint i = 0; i <= teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
                   width * 0.5);
    }
    glEnd();

    /* draw front sides of teeth */
    if (override_drawmodes == 1)
        glBegin(GL_LINES);
    else if (override_drawmodes == 2)
        glBegin(GL_POINTS);
    else
        glBegin(GL_QUADS);
    da = 2.0 * M_PI / teeth / 4.0;
    for (GLint i = 0; i < teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;

        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),
                   width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
                   width * 0.5);
    }
    glEnd();

    glNormal3f(0.0, 0.0, -1.0);

    /* draw back face */
    if (override_drawmodes == 1)
        glBegin(GL_LINES);
    else if (override_drawmodes == 2)
        glBegin(GL_POINTS);
    else
        glBegin(GL_QUAD_STRIP);
    for (GLint i = 0; i <= teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
                   -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    }
    glEnd();

    /* draw back sides of teeth */
    if (override_drawmodes == 1)
        glBegin(GL_LINES);
    else if (override_drawmodes == 2)
        glBegin(GL_POINTS);
    else
        glBegin(GL_QUADS);
    da = 2.0 * M_PI / teeth / 4.0;
    for (GLint i = 0; i < teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;

        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
                   -width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),
                   -width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    }
    glEnd();

    /* draw outward faces of teeth */
    if (override_drawmodes == 1)
        glBegin(GL_LINES);
    else if (override_drawmodes == 2)
        glBegin(GL_POINTS);
    else
        glBegin(GL_QUAD_STRIP);
    for (GLint i = 0; i < teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;

        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
        u = r2 * cos(angle + da) - r1 * cos(angle);
        v = r2 * sin(angle + da) - r1 * sin(angle);
        len = sqrt(u * u + v * v);
        u /= len;
        v /= len;
        glNormal3f(v, -u, 0.0);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
        glNormal3f(cos(angle), sin(angle), 0.0);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),
                   width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da),
                   -width * 0.5);
        u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
        v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
        glNormal3f(v, -u, 0.0);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
                   width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da),
                   -width * 0.5);
        glNormal3f(cos(angle), sin(angle), 0.0);
    }

    glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
    glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

    glEnd();

    /* draw inside radius cylinder */
    if (override_drawmodes == 1)
        glBegin(GL_LINES);
    else if (override_drawmodes == 2)
        glBegin(GL_POINTS);
    else
        glBegin(GL_QUAD_STRIP);
    for (GLint i = 0; i <= teeth; i++) {
        angle = i * 2.0 * M_PI / teeth;
        glNormal3f(-cos(angle), -sin(angle), 0.0);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    }
    glEnd();
}

static GLfloat view_rotx = 20.0, view_roty = 30.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

static void draw()
{
    angle += 2.0;
    glPushMatrix();
    glRotatef(view_rotx, 1.0, 0.0, 0.0);
    glRotatef(view_roty, 0.0, 1.0, 0.0);

    glPushMatrix();
    glTranslatef(-3.0, -2.0, 0.0);
    glRotatef(angle, 0.0, 0.0, 1.0);
    glCallList(gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.1, -2.0, 0.0);
    glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
    glCallList(gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.1, 4.2, 0.0);
    glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
    glCallList(gear3);
    glPopMatrix();

    glPopMatrix();
}

static void init_scene()
{
    static GLfloat pos[4] = {5, 5, 10, 0.0};  // Light at infinity.

    static GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
    static GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
    static GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
    static GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};
    static GLfloat shininess = 5;
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);
    glEnable(GL_CULL_FACE);

    glEnable(GL_LIGHT0);

    glEnable(GL_POLYGON_STIPPLE);
    glPolygonStipple(stipplepattern);
    glPointSize(10.0f);
    glTextSize(GL_TEXT_SIZE24x24);

    /* make the gears */
    gear1 = glGenLists(1);
    glNewList(gear1, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
    glColor3fv(blue);
    gear(1.0, 4.0, 1.0, 20, 0.7);  // The largest gear.
    glEndList();

    gear2 = glGenLists(1);
    glNewList(gear2, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glColor3fv(red);
    /* The small gear with the smaller hole, to the right. */
    gear(0.5, 2.0, 2.0, 10, 0.7);
    glEndList();

    gear3 = glGenLists(1);
    glNewList(gear3, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glColor3fv(green);
    /* The small gear above with the large hole. */
    gear(1.3, 2.0, 0.5, 10, 0.7);
    glEndList();
}

int main(int argc, char **argv)
{
    int winSizeX = 640, winSizeY = 480;
    unsigned int flat = 0;
    unsigned int setenspec = 1;
    unsigned int dotext = 1;
    unsigned int blending = 0;

    if (argc > 1) {
        char *larg = "";
        for (int i = 1; i < argc; i++) {
            if (!strcmp(larg, "-w"))
                winSizeX = atoi(argv[i]);
            if (!strcmp(larg, "-h"))
                winSizeY = atoi(argv[i]);
            if (!strcmp(argv[i], "-flat"))
                flat = 1;
            if (!strcmp(argv[i], "-smooth"))
                flat = 0;
            if (!strcmp(argv[i], "-blend"))
                blending = 1;
            if (!strcmp(argv[i], "-nospecular"))
                setenspec = 0;
            if (!strcmp(argv[i], "-lines"))
                override_drawmodes = 1;
            if (!strcmp(argv[i], "-points"))
                override_drawmodes = 2;
            if (!strcmp(argv[i], "-notext"))
                dotext = 0;
            larg = argv[i];
        }
    }

    fflush(stdout);
    PIXEL *imbuf = calloc(1, sizeof(PIXEL) * winSizeX * winSizeY);

    // initialize TinyGL
    ZBuffer *frameBuffer = ZB_open(winSizeX, winSizeY,
#if TGL_FEATURE_RENDER_BITS == 32
                                   ZB_MODE_RGBA,
#else
                                   ZB_MODE_5R6G5B,
#endif
                                   0);
    if (!frameBuffer) {
        printf("\nZB_open failed!");
        exit(1);
    }
    glInit(frameBuffer);

    // Print version info
    printf("Version string:\n%s", glGetString(GL_VERSION));
    printf("Vendor string:\n%s", glGetString(GL_VENDOR));
    printf("Renderer string:\n%s", glGetString(GL_RENDERER));
    printf("Extensions string:\n%s", glGetString(GL_EXTENSIONS));
    printf("\n");

    // initialize GL:
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glViewport(0, 0, winSizeX, winSizeY);
    glShadeModel(flat ? GL_FLAT : GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glBlendEquation(GL_FUNC_ADD);
    if (blending) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR);
        glBlendEquation(GL_FUNC_ADD);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
    GLfloat h = (GLfloat) winSizeY / (GLfloat) winSizeX;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -45.0);

    init_scene();
    if (setenspec)
        glSetEnableSpecular(GL_TRUE);
    else
        glSetEnableSpecular(GL_FALSE);
    unsigned int frames = 0;

    int isRunning = 1;
    while (isRunning) {
        ++frames;

        // draw scene:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw();
        if (dotext) {
            glDrawText((GLubyte *) "RED text", 0, 0, 0xFF0000);
            glDrawText((GLubyte *) "GREEN text", 0, 24, 0x00FF00);
            glDrawText((GLubyte *) "BLUE text", 0, 48, 0xFF);
        }

        // swap buffers:
        // Quickly convert all pixels to the correct format
        ZB_copyFrameBuffer(frameBuffer, imbuf, winSizeX * sizeof(PIXEL));
        if (frames > 0)
            break;
    }

    {
        uint8_t *pbuf = malloc(3 * winSizeX * winSizeY);
        for (int i = 0; i < winSizeX * winSizeY; i++) {
            pbuf[3 * i + 0] = GET_RED(imbuf[i]);
            pbuf[3 * i + 1] = GET_GREEN(imbuf[i]);
            pbuf[3 * i + 2] = GET_BLUE(imbuf[i]);
        }
        stbi_write_png("render.png", winSizeX, winSizeY, 3, pbuf, 0);
        free(imbuf);
        free(pbuf);
    }

    // cleanup:
    glDeleteList(gear1);
    glDeleteList(gear2);
    glDeleteList(gear3);

    ZB_close(frameBuffer);
    glClose();

    return 0;
}
