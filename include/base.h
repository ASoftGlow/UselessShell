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
#define _FE1(A,FN,E,...)FN(A,E),
#define _FE2(A,FN,E,...)FN(A,E),_FE1(A,FN,__VA_ARGS__)
#define _FE3(A,FN,E,...)FN(A,E),_FE2(A,FN,__VA_ARGS__)
#define _FE4(A,FN,E,...)FN(A,E),_FE3(A,FN,__VA_ARGS__)
#define _FE5(A,FN,E,...)FN(A,E),_FE4(A,FN,__VA_ARGS__)
#define _FE6(A,FN,E,...)FN(A,E),_FE5(A,FN,__VA_ARGS__)
#define _FE7(A,FN,E,...)FN(A,E),_FE6(A,FN,__VA_ARGS__)
#define _FE8(A,FN,E,...)FN(A,E),_FE7(A,FN,__VA_ARGS__)
#define _FE9(A,FN,E,...)FN(A,E),_FE8(A,FN,__VA_ARGS__)
#define _FE10(A,FN,E,...)FN(A,E),_FE9(A,FN,__VA_ARGS__)
#define _FE11(A,FN,E,...)FN(A,E),_FE10(A,FN,__VA_ARGS__)
#define _FE12(A,FN,E,...)FN(A,E),_FE11(A,FN,__VA_ARGS__)
#define _FE13(A,FN,E,...)FN(A,E),_FE12(A,FN,__VA_ARGS__)
#define _FE14(A,FN,E,...)FN(A,E),_FE13(A,FN,__VA_ARGS__)
#define _FE15(A,FN,E,...)FN(A,E),_FE14(A,FN,__VA_ARGS__)
#define _FE16(A,FN,E,...)FN(A,E),_FE15(A,FN,__VA_ARGS__)
#define _FE17(A,FN,E,...)FN(A,E),_FE16(A,FN,__VA_ARGS__)
#define _FE18(A,FN,E,...)FN(A,E),_FE17(A,FN,__VA_ARGS__)
#define _FE19(A,FN,E,...)FN(A,E),_FE18(A,FN,__VA_ARGS__)
#define _FE20(A,FN,E,...)FN(A,E),_FE19(A,FN,__VA_ARGS__)
#define _FE21(A,FN,E,...)FN(A,E),_FE20(A,FN,__VA_ARGS__)
#define _FE22(A,FN,E,...)FN(A,E),_FE21(A,FN,__VA_ARGS__)
#define _FE23(A,FN,E,...)FN(A,E),_FE22(A,FN,__VA_ARGS__)
#define _FE24(A,FN,E,...)FN(A,E),_FE23(A,FN,__VA_ARGS__)
#define _FE25(A,FN,E,...)FN(A,E),_FE24(A,FN,__VA_ARGS__)
#define _FE26(A,FN,E,...)FN(A,E),_FE25(A,FN,__VA_ARGS__)
#define _FE27(A,FN,E,...)FN(A,E),_FE26(A,FN,__VA_ARGS__)
#define _FE28(A,FN,E,...)FN(A,E),_FE27(A,FN,__VA_ARGS__)
#define _FE29(A,FN,E,...)FN(A,E),_FE28(A,FN,__VA_ARGS__)
#define _FE30(A,FN,E,...)FN(A,E),_FE29(A,FN,__VA_ARGS__)
#define _FE31(A,FN,E,...)FN(A,E),_FE30(A,FN,__VA_ARGS__)
#define __FE(A,FN,NARGS,...)_FE##NARGS(A,FN,__VA_ARGS__)
#define _FE(A,FN,NARGS,...)__FE(A,FN,NARGS,__VA_ARGS__)
#define FE(A,FN,...)_FE(A,FN,_FE_VA(__VA_ARGS__),__VA_ARGS__)
#define TOKEN_CONCAT(A,B) A##B
#define ENUM_SLIM(name,...)enum{FE(name,TOKEN_CONCAT,__VA_ARGS__)};
#define ENUM(name,size,...)ENUM_SLIM(name,__VA_ARGS__)typedef size name;
#define TOKEN_TO_STR(A,B) #B
#define ENUM_REFLECT(name,size,...)ENUM(name,size,__VA_ARGS__)const static char* name ## __Names[] = {FE(0,TOKEN_TO_STR,__VA_ARGS__)};
#pragma endregion