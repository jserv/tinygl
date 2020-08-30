// tgl_msg_handling.cpp

#include "tgl.h"

#ifdef NDEBUG
# define NO_DEBUG_OUTPUT
#endif

logFuncPtr _tgl_log_func_ptr = NULL;

void tgl_set_log_func(logFuncPtr ptr)
{
    _tgl_log_func_ptr = ptr;
}

char _gl_temp[256];

/* Use this function to output messages when something unexpected
   happens (which might be an indication of an error). *Don't* use it
   when there's internal errors in the code - these should be handled
   by asserts. */
void tgl_warning(const char *format, ...)
{
    va_list args;
    if (_tgl_log_func_ptr) {
	sprintf(_gl_temp,"TGL Warning: %s",format);
	va_start(args, format);
	_tgl_log_func_ptr(_gl_temp,args);
	va_end(args);
    }
}

/* This function should be used for debug output only. */
void tgl_info(const char *format, ...)
{
    va_list args;
    if (_tgl_log_func_ptr) {
	sprintf(_gl_temp,"TGL %s",format);
	va_start(args, format);
	_tgl_log_func_ptr(_gl_temp,args);
	va_end(args);
    }
}

void tgl_fatal_error(const char *format, ...)
{
    va_list args;
    if (_tgl_log_func_ptr) {
	sprintf(_gl_temp,"TGL Fatal error: %s",format);
	va_start(args, format);
	_tgl_log_func_ptr(_gl_temp,args);
	va_end(args);
    }
    exit(1);
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
