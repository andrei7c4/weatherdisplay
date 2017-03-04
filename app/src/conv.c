#include <os_type.h>
#include <osapi.h>
#include "config.h"

int ICACHE_FLASH_ATTR strtoint(const char *p)
{
	int sign = 1;
	if (*p == '-'){
		sign = -1;
		p++;
	}
    int k = 0;
    while (*p) {
        k = (k<<3)+(k<<1)+(*p)-'0';
        p++;
     }
     return k*sign;
}

float ICACHE_FLASH_ATTR strtofloat(const char* s)
{
  float rez = 0, fact = 1;
  int point_seen = 0;
  if (*s == '-'){
	s++;
	fact = -1;
  };
  for (; *s; s++){
	if (*s == '.'){
	  point_seen = 1;
	  continue;
	};
	int d = *s - '0';
	if (d >= 0 && d <= 9){
	  if (point_seen) fact /= 10.0f;
	  rez = rez * 10.0f + (float)d;
	};
  };
  return rez * fact;
}


int ICACHE_FLASH_ATTR floatToIntRound(float f)
{
	return f >= 0 ? (int)(f+0.5) : (int)(f-0.5);
}

float ICACHE_FLASH_ATTR kelvinToTemp(float kelvin)
{
	const float zeroKelvin = 273.15;
	if (config.fahrenheit)
	{
		return (kelvin-zeroKelvin) * 1.8 + 32.0;
	}
	return kelvin-zeroKelvin;
}

int ICACHE_FLASH_ATTR alignTo8(int value)
{
	int mod = value%8;
	if (mod)
	{
		if (mod <= 4)
		{
			value -= mod;
		}
		else
		{
			value += (8-mod);
		}
	}
	return value;
}

