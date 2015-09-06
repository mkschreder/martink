

#ifndef _UTL_H
#define _UTL_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define UTL_STRING_CONVERSION_DATA_LEN 6

extern char m2_utl_string_conversion_data[UTL_STRING_CONVERSION_DATA_LEN];


char *m2_utl_u8dp(char *dest, uint8_t v);
char *m2_utl_u8d(uint8_t v, uint8_t d);
char *m2_utl_s8d(int8_t v, uint8_t d, uint8_t is_plus); 

#ifdef __cplusplus
}
#endif


#endif
