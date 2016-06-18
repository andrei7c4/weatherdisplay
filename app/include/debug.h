#ifndef INCLUDE_DEBUG_H_
#define INCLUDE_DEBUG_H_

#include <stdarg.h>
int ets_vsnprintf(char *buffer, size_t sizeOfBuffer,  const char *format, va_list argptr);

//#define NDEBUG

#ifdef NDEBUG
#define debug(format, args...) ((void)0)
#else
void debug(char *format, ...);
#endif



#endif /* INCLUDE_DEBUG_H_ */
