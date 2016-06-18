#include <os_type.h>
#include <osapi.h>
#include "debug.h"
#include "config.h"

#ifndef NDEBUG

#define BUF_SIZE 128
void ICACHE_FLASH_ATTR debug(char* format, ...)
{
	if (!config.debug)
		return;

	char buffer[BUF_SIZE];
	va_list al;
	va_start(al, format);
	ets_vsnprintf(buffer, BUF_SIZE-1, format, al);
	va_end(al);
	os_printf(buffer);
}

#endif	// NDEBUG
