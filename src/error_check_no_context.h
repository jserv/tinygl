#ifndef RETVAL
#define RETVAL
#endif

#if TGL_HAS(ERROR_CHECK)
#if TGL_HAS(STRICT_OOM_CHECKS)
GLContext *c = gl_get_context();
if (c->error_flag == GL_OUT_OF_MEMORY)
    return RETVAL;
#elif defined(NEED_CONTEXT)
GLContext *c = gl_get_context();
#endif
#endif

#undef RETVAL
#undef NEED_CONTEXT
