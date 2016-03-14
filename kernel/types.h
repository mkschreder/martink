#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <string.h>

#define __init __attribute__ ((constructor))
#define __exit __attribute__ ((destructor))

#define __unused __attribute__((unused))

#define container_of(ptr, type, member) __extension__ ({                      \
        __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)(void*)( (char *)__mptr - offsetof(type,member) );})

typedef long ssize_t; 

typedef uint16_t gpio_pin_t;
typedef int32_t timestamp_t;

