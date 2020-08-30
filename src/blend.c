// tgl_blend.cpp

#include "tgl.h"

void glAlphaFunc(GLenum func, GLclampf ref)
{
    GLContext *c = gl_get_context();
    c->alpha.func = (int) func;
    c->alpha.ref = (int) ref;
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    GLContext *c = gl_get_context();
    c->blend.sfactor = sfactor;
    c->blend.dfactor = dfactor;
}

void glLogicOp(GLenum opcode)
{
    GLContext *c = gl_get_context();
    c->logic.op = opcode;
}

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
