#ifndef INCLUDE_CONV_H_
#define INCLUDE_CONV_H_

#include "typedefs.h"

int strtoint(const char *p);
float strtofloat(const char* num);

float kelvinToTemp(float kelvin);
int floatToIntRound(float f);
int alignTo8(int value);
void swap(uchar *a, uchar *b);
void swapInt(int *a, int *b);
int modfInt(float f, uchar fractDigits, int *intpart);

#endif /* INCLUDE_CONV_H_ */
