/*
fftradix4 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <avr/pgmspace.h>
#include <math.h> //include libm

#include "fftradix4.h"

#define NWAVE 256 //full length of Sinewave[]
const int16_t Sinewave[NWAVE] PROGMEM = {
      +0,      +6,     +13,     +19,     +25,     +31,     +37,     +44,     +50,     +56,     +62,     +68,     +74,     +80,     +86,     +92,
     +98,    +103,    +109,    +115,    +120,    +126,    +131,    +136,    +142,    +147,    +152,    +157,    +162,    +167,    +171,    +176,
    +180,    +185,    +189,    +193,    +197,    +201,    +205,    +208,    +212,    +215,    +219,    +222,    +225,    +228,    +231,    +233,
    +236,    +238,    +240,    +242,    +244,    +246,    +247,    +249,    +250,    +251,    +252,    +253,    +254,    +254,    +255,    +255,
    +255,    +255,    +255,    +254,    +254,    +253,    +252,    +251,    +250,    +249,    +247,    +246,    +244,    +242,    +240,    +238,
    +236,    +233,    +231,    +228,    +225,    +222,    +219,    +215,    +212,    +208,    +205,    +201,    +197,    +193,    +189,    +185,
    +180,    +176,    +171,    +167,    +162,    +157,    +152,    +147,    +142,    +136,    +131,    +126,    +120,    +115,    +109,    +103,
     +98,     +92,     +86,     +80,     +74,     +68,     +62,     +56,     +50,     +44,     +37,     +31,     +25,     +19,     +13,      +6,
      +0,      -6,     -13,     -19,     -25,     -31,     -38,     -44,     -50,     -56,     -62,     -68,     -74,     -80,     -86,     -92,
     -98,    -104,    -109,    -115,    -121,    -126,    -132,    -137,    -142,    -147,    -152,    -157,    -162,    -167,    -172,    -177,
    -181,    -185,    -190,    -194,    -198,    -202,    -206,    -209,    -213,    -216,    -220,    -223,    -226,    -229,    -231,    -234,
    -237,    -239,    -241,    -243,    -245,    -247,    -248,    -250,    -251,    -252,    -253,    -254,    -255,    -255,    -256,    -256,
    -256,    -256,    -256,    -255,    -255,    -254,    -253,    -252,    -251,    -250,    -248,    -247,    -245,    -243,    -241,    -239,
    -237,    -234,    -231,    -229,    -226,    -223,    -220,    -216,    -213,    -209,    -206,    -202,    -198,    -194,    -190,    -185,
    -181,    -177,    -172,    -167,    -162,    -157,    -152,    -147,    -142,    -137,    -132,    -126,    -121,    -115,    -109,    -104,
     -98,     -92,     -86,     -80,     -74,     -68,     -62,     -56,     -50,     -44,     -38,     -31,     -25,     -19,     -13,      -6
};

#define mult_shf_s16x16( a, b)    \
({                        \
int prod, val1=a, val2=b; \
__asm__ __volatile__ (    \
"muls %B1, %B2	\n\t"     \
"mov %B0, r0    \n\t"	  \
"mul %A1, %A2   \n\t"	  \
"mov %A0, r1    \n\t"     \
"mulsu %B1, %A2	\n\t"     \
"add %A0, r0    \n\t"     \
"adc %B0, r1    \n\t"     \
"mulsu %B2, %A1	\n\t"     \
"add %A0, r0    \n\t"     \
"adc %B0, r1    \n\t"     \
"clr r1         \n\t"     \
: "=&d" (prod)            \
: "a" (val1), "a" (val2)  \
);                        \
prod;                     \
})

static inline void mult_shf_I(int c, int s, int x, int y, int *u, int *v) __attribute__((always_inline));
static inline void mult_shf_I(int c, int s, int x, int y, int *u, int *v) {
	*u = (mult_shf_s16x16(x, c) - mult_shf_s16x16(y, s));
	*v = (mult_shf_s16x16(y, c) + mult_shf_s16x16(x, s));
}

static inline void sum_dif_I(int a, int b, int *s, int *d) __attribute__((always_inline));
static inline void sum_dif_I(int a, int b, int *s, int *d) {
	*s = (a+b);
	*d = (a-b);
}

void rev_bin(int *fr, int fft_n) {
    int m, mr, nn, l;
    int tr;

    mr = 0;
    nn = fft_n - 1;

    for (m=1; m<=nn; ++m) {
		l = fft_n;
		do {
			l >>= 1;
		} while (mr+l > nn);

		mr = (mr & (l-1)) + l;

		if (mr <= m)
			continue;
		tr = fr[m];
		fr[m] = fr[mr];
		fr[mr] = tr;
	}
}

void fft8_dit_core_p1(int *fr, int *fi) {
    int plus1a, plus2a, plus3a, plus4a, plus1b, plus2b;
    int mins1a, mins2a, mins3a, mins4a, mins1b, mins2b, mM1a, mM2a;

    sum_dif_I(fr[0], fr[1], &plus1a, &mins1a);
    sum_dif_I(fr[2], fr[3], &plus2a, &mins2a);
    sum_dif_I(fr[4], fr[5], &plus3a, &mins3a);
    sum_dif_I(fr[6], fr[7], &plus4a, &mins4a);

    sum_dif_I(plus1a, plus2a, &plus1b, &mins1b);
    sum_dif_I(plus3a, plus4a, &plus2b, &mins2b);

    sum_dif_I(plus1b, plus2b, &fr[0], &fr[4]);
    sum_dif_I(mins3a, mins4a, &mM1a, &mM2a);

    int prib1a, prib2a, prib3a, prib4a, prib1b, prib2b;
    int otnt1a, otnt2a, otnt3a, otnt4a, otnt1b, otnt2b, oT1a, oT2a;

    sum_dif_I(fi[0], fi[1], &prib1a, &otnt1a);
    sum_dif_I(fi[2], fi[3], &prib2a, &otnt2a);
    sum_dif_I(fi[4], fi[5], &prib3a, &otnt3a);
    sum_dif_I(fi[6], fi[7], &prib4a, &otnt4a);

    sum_dif_I(prib1a, prib2a, &prib1b, &otnt1b);
    sum_dif_I(prib3a, prib4a, &prib2b, &otnt2b);

    sum_dif_I(prib1b, prib2b, &fi[0], &fi[4]);
    sum_dif_I(otnt3a, otnt4a, &oT1a, &oT2a);

    mM2a = mult_shf_s16x16(mM2a, 181);
    sum_dif_I(mins1a,   mM2a, &plus1a, &plus2a);

    prib2b =    mult_shf_s16x16(oT1a, 181);
    sum_dif_I(otnt2a, prib2b, &mins3a, &plus3a);

    sum_dif_I(plus1a, mins3a, &fr[7], &fr[1]);
    sum_dif_I(mins1b, otnt2b, &fr[6], &fr[2]);
    sum_dif_I(plus2a, plus3a, &fr[3], &fr[5]);

    oT2a = mult_shf_s16x16(oT2a, 181);
    sum_dif_I( otnt1a, oT2a, &plus1a, &plus2a);

    plus2b =    mult_shf_s16x16(mM1a, 181);
    sum_dif_I(-mins2a, plus2b, &plus3a, &mins3a);

    sum_dif_I(plus1a, mins3a, &fi[7], &fi[1]);
    sum_dif_I(otnt1b,-mins2b, &fi[6], &fi[2]);
    sum_dif_I(plus2a, plus3a, &fi[3], &fi[5]);
}

void fft_radix4_I(int *fr, int *fi, int ldn) {
    const int n = (1UL<<ldn);
    int ldm = 0, rdx = 2;

    ldm = (ldn&1);
    if ( ldm!=0 ) {
        for (int i0=0; i0<n; i0+=8) {
            fft8_dit_core_p1(fr+i0, fi+i0);
        }
    } else {
		for (int i0 = 0; i0 < n; i0 += 4) {
			int xr,yr,ur,vr, xi,yi,ui,vi;

			int i1 = i0 + 1;
			int i2 = i1 + 1;
			int i3 = i2 + 1;

			sum_dif_I(fr[i0], fr[i1], &xr, &ur);
			sum_dif_I(fr[i2], fr[i3], &yr, &vi);
			sum_dif_I(fi[i0], fi[i1], &xi, &ui);
			sum_dif_I(fi[i3], fi[i2], &yi, &vr);

			sum_dif_I(ui, vi, &fi[i1], &fi[i3]);
			sum_dif_I(xi, yi, &fi[i0], &fi[i2]);
			sum_dif_I(ur, vr, &fr[i1], &fr[i3]);
			sum_dif_I(xr, yr, &fr[i0], &fr[i2]);
		}
    }
    for (ldm += 2 * rdx; ldm <= ldn; ldm += rdx) {
        int m = (1UL<<ldm);
        int m4 = (m>>rdx);

        int phI0 =  NWAVE / m;
        int phI  = 0;

        for (int j = 0; j < m4; j++) {
			int c,s,c2,s2,c3,s3;

			s  = pgm_read_word(&Sinewave[   phI]);
			s2 = pgm_read_word(&Sinewave[ 2*phI]);
			s3 = pgm_read_word(&Sinewave[ 3*phI]);

			c  = pgm_read_word(&Sinewave[   phI + NWAVE/4]);
			c2 = pgm_read_word(&Sinewave[ 2*phI + NWAVE/4]);
			c3 = pgm_read_word(&Sinewave[ 3*phI + NWAVE/4]);

			for (int r = 0; r < n; r += m) {
                int i0 = j + r;
                int i1 = i0 + m4;
                int i2 = i1 + m4;
                int i3 = i2 + m4;

                int xr,yr,ur,vr,xi,yi,ui,vi;

				mult_shf_I( c2, s2, fr[i1], fi[i1], &xr, &xi);
				mult_shf_I(  c,  s, fr[i2], fi[i2], &yr, &vr);
				mult_shf_I( c3, s3, fr[i3], fi[i3], &vi, &yi);

				int t = yi - vr;
                yi += vr;
                vr = t;

                ur = fr[i0] - xr;
                xr += fr[i0];

                sum_dif_I(ur, vr, &fr[i1], &fr[i3]);

                t = yr - vi;
                yr += vi;
                vi = t;

                ui = fi[i0] - xi;
                xi += fi[i0];

                sum_dif_I(ui, vi, &fi[i1], &fi[i3]);
                sum_dif_I(xr, yr, &fr[i0], &fr[i2]);
                sum_dif_I(xi, yi, &fi[i0], &fi[i2]);
			}
			phI += phI0;
        }
    }
}

