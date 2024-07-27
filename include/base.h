#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef uint8_t byte;

#define countof(a) (sizeof(a)/sizeof(a[0]))
#define STR_VALUE(arg) #arg

#pragma region Enum Macros
#define __FE_VA(_31,_30,_29,_28,_27,_26,_25,_24,_23,_22,_21,_20,_19,_18,_17,_16,_15,_14,_13,_12,_11,_10,_9,_8,_7,_6,_5,_4,_3,_2,_1,N,...)N
#define _FE_VA(...)__FE_VA(__VA_ARGS__ __VA_OPT__(,)31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define _FE0(A,...)
#define _FE1(A,E,...)A##E,
#define _FE2(A,E,...)A##E,_FE1(A,__VA_ARGS__)
#define _FE3(A,E,...)A##E,_FE2(A,__VA_ARGS__)
#define _FE4(A,E,...)A##E,_FE3(A,__VA_ARGS__)
#define _FE5(A,E,...)A##E,_FE4(A,__VA_ARGS__)
#define _FE6(A,E,...)A##E,_FE5(A,__VA_ARGS__)
#define _FE7(A,E,...)A##E,_FE6(A,__VA_ARGS__)
#define _FE8(A,E,...)A##E,_FE7(A,__VA_ARGS__)
#define _FE9(A,E,...)A##E,_FE8(A,__VA_ARGS__)
#define _FE10(A,E,...)A##E,_FE9(A,__VA_ARGS__)
#define _FE11(A,E,...)A##E,_FE10(A,__VA_ARGS__)
#define _FE12(A,E,...)A##E,_FE11(A,__VA_ARGS__)
#define _FE13(A,E,...)A##E,_FE12(A,__VA_ARGS__)
#define _FE14(A,E,...)A##E,_FE13(A,__VA_ARGS__)
#define _FE15(A,E,...)A##E,_FE14(A,__VA_ARGS__)
#define _FE16(A,E,...)A##E,_FE15(A,__VA_ARGS__)
#define _FE17(A,E,...)A##E,_FE16(A,__VA_ARGS__)
#define _FE18(A,E,...)A##E,_FE17(A,__VA_ARGS__)
#define _FE19(A,E,...)A##E,_FE18(A,__VA_ARGS__)
#define _FE20(A,E,...)A##E,_FE19(A,__VA_ARGS__)
#define _FE21(A,E,...)A##E,_FE20(A,__VA_ARGS__)
#define _FE22(A,E,...)A##E,_FE21(A,__VA_ARGS__)
#define _FE23(A,E,...)A##E,_FE22(A,__VA_ARGS__)
#define _FE24(A,E,...)A##E,_FE23(A,__VA_ARGS__)
#define _FE25(A,E,...)A##E,_FE24(A,__VA_ARGS__)
#define _FE26(A,E,...)A##E,_FE25(A,__VA_ARGS__)
#define _FE27(A,E,...)A##E,_FE26(A,__VA_ARGS__)
#define _FE28(A,E,...)A##E,_FE27(A,__VA_ARGS__)
#define _FE29(A,E,...)A##E,_FE28(A,__VA_ARGS__)
#define _FE30(A,E,...)A##E,_FE29(A,__VA_ARGS__)
#define _FE31(A,E,...)A##E,_FE30(A,__VA_ARGS__)
#define __FE(A,NARGS,...)_FE##NARGS(A,__VA_ARGS__)
#define _FE(A,NARGS,...)__FE(A,NARGS,__VA_ARGS__)
#define FE(A,...)_FE(A,_FE_VA(__VA_ARGS__),__VA_ARGS__)
#define ENUM_SLIM(name,...)enum{FE(name,__VA_ARGS__)};
#define ENUM(name,size,...)ENUM_SLIM(name,__VA_ARGS__)typedef size name;
#define _FEV0(...)
#define _FEV1(E,...)#E,
#define _FEV2(E,...)#E,_FEV1(__VA_ARGS__)
#define _FEV3(E,...)#E,_FEV2(__VA_ARGS__)
#define _FEV4(E,...)#E,_FEV3(__VA_ARGS__)
#define _FEV5(E,...)#E,_FEV4(__VA_ARGS__)
#define _FEV6(E,...)#E,_FEV5(__VA_ARGS__)
#define _FEV7(E,...)#E,_FEV6(__VA_ARGS__)
#define _FEV8(E,...)#E,_FEV7(__VA_ARGS__)
#define _FEV9(E,...)#E,_FEV8(__VA_ARGS__)
#define _FEV10(E,...)#E,_FEV9(__VA_ARGS__)
#define _FEV11(E,...)#E,_FEV10(__VA_ARGS__)
#define _FEV12(E,...)#E,_FEV11(__VA_ARGS__)
#define _FEV13(E,...)#E,_FEV12(__VA_ARGS__)
#define _FEV14(E,...)#E,_FEV13(__VA_ARGS__)
#define _FEV15(E,...)#E,_FEV14(__VA_ARGS__)
#define _FEV16(E,...)#E,_FEV15(__VA_ARGS__)
#define _FEV17(E,...)#E,_FEV16(__VA_ARGS__)
#define _FEV18(E,...)#E,_FEV17(__VA_ARGS__)
#define _FEV19(E,...)#E,_FEV18(__VA_ARGS__)
#define _FEV20(E,...)#E,_FEV19(__VA_ARGS__)
#define _FEV21(E,...)#E,_FEV20(__VA_ARGS__)
#define _FEV22(E,...)#E,_FEV21(__VA_ARGS__)
#define _FEV23(E,...)#E,_FEV22(__VA_ARGS__)
#define _FEV24(E,...)#E,_FEV23(__VA_ARGS__)
#define _FEV25(E,...)#E,_FEV24(__VA_ARGS__)
#define _FEV26(E,...)#E,_FEV25(__VA_ARGS__)
#define _FEV27(E,...)#E,_FEV26(__VA_ARGS__)
#define _FEV28(E,...)#E,_FEV27(__VA_ARGS__)
#define _FEV29(E,...)#E,_FEV28(__VA_ARGS__)
#define _FEV30(E,...)#E,_FEV29(__VA_ARGS__)
#define _FEV31(E,...)#E,_FEV30(__VA_ARGS__)
#define __FEV(NARGS,...)_FEV##NARGS(__VA_ARGS__)
#define _FEV(NARGS,...)__FEV(NARGS,__VA_ARGS__)
#define FEV(...)_FEV(_FE_VA(__VA_ARGS__),__VA_ARGS__)
#define ENUM_VERBOSE(name,size,...)ENUM(name,size,__VA_ARGS__)const static char* name ## __Names[] = {FEV(__VA_ARGS__)};
#pragma endregion