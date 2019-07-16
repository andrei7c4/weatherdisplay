#ifndef DEBUG_H
#define DEBUG_H

#include <osapi.h>
#include "config.h"

//#define NDEBUG

#ifndef NDEBUG
#define debug(...) \
            do { if (config.debugEn) os_printf(__VA_ARGS__); } while (0)
#else
#define debug(...)
#endif


#endif /* DEBUG_H */
