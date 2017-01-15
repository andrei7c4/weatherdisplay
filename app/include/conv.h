#ifndef INCLUDE_CONV_H_
#define INCLUDE_CONV_H_

int strtoint(const char *p);
float strtofloat(const char* num);

float kelvinToTemp(float kelvin);
int floatToIntRound(float f);
int alignTo8(int value);

#endif /* INCLUDE_CONV_H_ */
