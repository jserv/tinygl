#ifndef RETVAL
#define RETVAL
#endif

#if TGL_HAS(ERROR_CHECK)
#ifndef ERROR_FLAG
#if TGL_HAS(STRICT_OOM_CHECKS)
if (c->error_flag == GL_OUT_OF_MEMORY)
    return RETVAL;
#endif
#elif ERROR_FLAG != GL_OUT_OF_MEMORY
{
    c->error_flag = ERROR_FLAG;
    return RETVAL;
}
#else
{
    c->error_flag = GL_OUT_OF_MEMORY;
    return RETVAL;
}
#endif
#endif

#undef RETVAL
#undef ERROR_FLAG
