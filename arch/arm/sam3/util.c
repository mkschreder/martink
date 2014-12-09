#include <stdlib.h>
#include <math.h>
#include <stdio.h>
/*
void raise(){
	while(1); 
}

int __isnanf(float x){
	return isnan(x); 
}
*/
float sqrtf(float x){
	return 0; //return sqrt(x); 
}

#undef atan2

#define abs(x) ((x<0)?-x:x)
#define max(a, b) ((a >= b)?a:b) 
#define min(a, b) ((a <= b)?a:b) 

double atan2(double y, double x)
{
  float t0, t1, t2, t3, t4;

  t3 = abs(x);
  t1 = abs(y);
  t0 = max(t3, t1);
  t1 = min(t3, t1);
  t3 = (float)1.0f / t0;
  t3 = t1 * t3;

  t4 = t3 * t3;
  t0 =         - (float)0.013480470;
  t0 = t0 * t4 + (float)0.057477314;
  t0 = t0 * t4 - (float)0.121239071;
  t0 = t0 * t4 + (float)0.195635925;
  t0 = t0 * t4 - (float)0.332994597;
  t0 = t0 * t4 + (float)0.999995630;
  t3 = t0 * t3;

  t3 = (abs(y) > abs(x)) ? (float)1.570796327 - t3 : t3;
  t3 = (x < 0) ?  (float)3.141592654 - t3 : t3;
  t3 = (y < 0) ? -t3 : t3;

  return t3;
}
/*
void __assert_fail(void){
	
}

void srand(unsigned s){
	
}

int __errno(void){
	return 0; 
}

int __assert_func(void){
	return 1; 
}
*/
/*
#include <reent.h>
#include <unistd.h>

extern void *_sbrk_r (struct _reent *, ptrdiff_t);
void *_sbrk (ptrdiff_t d){
	return 0; 
}*/
