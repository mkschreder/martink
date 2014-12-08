#pragma once 

#define wrap_pi(x) (x < -M_PI ? x+M_PI*2 : (x > M_PI ? x - M_PI*2: x))

inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


inline long constrain(long x, long a, long b){
	if(x < a) return a; 
	if(x > b) return b; 
	return x; 
}
