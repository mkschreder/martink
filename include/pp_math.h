#pragma once

#define __LOG2A(s) ((((s) &0xffffffff00000000) ? (32 +__LOG2B((s) >>32)): (__LOG2B(s))))
#define __LOG2B(s) ((((s) &0xffff0000)         ? (16 +__LOG2C((s) >>16)): (__LOG2C(s))))
#define __LOG2C(s) ((((s) &0xff00)             ? (8  +__LOG2D((s) >>8)) : (__LOG2D(s))))
#define __LOG2D(s) ((((s) &0xf0)               ? (4  +__LOG2E((s) >>4)) : (__LOG2E(s))))
#define __LOG2E(s) ((((s) &0xc)                ? (2  +__LOG2F((s) >>2)) : (__LOG2F(s))))
#define __LOG2F(s) ((((s) &0x2)                ? (1)                  : (0)))

#define LOG2_UINT64 __LOG2A
#define LOG2_UINT32 __LOG2B
#define LOG2_UINT16 __LOG2C
#define LOG2_UINT8  __LOG2D

#define NEXT_POW2(i) ((typeof(i))(1UL << (1UL + LOG2_UINT64((i) - 1UL))))
