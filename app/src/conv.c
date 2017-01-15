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

float ICACHE_FLASH_ATTR strtofloat(const char* num)
{
     if (!num || !*num)
         return 0;
     float integerPart = 0;
     float fractionPart = 0;
     int divisorForFraction = 1;
     int sign = 1;
     int inFraction = 0;
     /*Take care of +/- sign*/
     if (*num == '-')
     {
         ++num;
         sign = -1;
     }
     else if (*num == '+')
     {
         ++num;
     }
     while (*num != '\0')
     {
         if (*num >= '0' && *num <= '9')
         {
             if (inFraction)
             {
                 /*See how are we converting a character to integer*/
                 fractionPart = fractionPart*10 + (*num - '0');
                 divisorForFraction *= 10;
             }
             else
             {
                 integerPart = integerPart*10 + (*num - '0');
             }
         }
         else if (*num == '.')
         {
             if (inFraction)
                 return sign * (integerPart + fractionPart/divisorForFraction);
             else
                 inFraction = 1;
         }
         else
         {
             return sign * (integerPart + fractionPart/divisorForFraction);
         }
         ++num;
     }
     return sign * (integerPart + fractionPart/divisorForFraction);
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

