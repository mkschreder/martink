#pragma once

#define ECIES_DEGREE 163                      /* the degree of the field polynomial */
#define ECIES_MARGIN 3                                          /* don't touch this */
#define ECIES_NUMWORDS ((ECIES_DEGREE + ECIES_MARGIN + 31) / 32)

#define ECIES_OVERHEAD (8 * ECIES_NUMWORDS + 8)

#ifdef __cplusplus
extern "C" {
#endif

void ECIES_init(void); 
int ECIES_decryption(char *text, const char *msg, int len, const char *privkey);
void ECIES_encryption(char *msg, const char *text, int len, 
		     const char *Px, const char *Py); 

#ifdef __cplusplus
}
#endif
