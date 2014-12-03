/*
fftradix4 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  + Anatoly Kuzmenko radix4 library
     http://coolarduino.wordpress.com/2012/03/24/radix-4-fft-integer-math/
*/


#ifndef FFTRADIX4_H_
#define FFTRADIX4_H_

#define FFT_SIZE 128
#define LOG_2_FFT 7 //log2 FFT_SIZE

//functions
extern void rev_bin(int *fr, int fft_n);
extern void fft_radix4_I(int *fr, int *fi, int ldn);

#endif
