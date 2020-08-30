// tgl_specbuf.h

#ifndef __TGL_SPECBUF_H_
#define __TGL_SPECBUF_H_

#include "tgl.h"

/* Max # of specular light pow buffers */
#define MAX_SPECULAR_BUFFERS 8
/* # of entries in specular buffer */
#define SPECULAR_BUFFER_SIZE 1024
/* specular buffer granularity */
#define SPECULAR_BUFFER_RESOLUTION 1024

GLSpecBuf *specbuf_get_buffer(GLContext *c, const int shininess_i, const float shininess);
void specbuf_cleanup(GLContext *c); /* free all memory used */

#endif // __TGL_SPECBUF_H_

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
