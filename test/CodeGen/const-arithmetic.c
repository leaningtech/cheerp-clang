// RUN: %clang_cc1 -triple x86_64-unknown-unknown -emit-llvm -o - %s

// XFAIL: @g1 = global [2 x i8*] [i8* getelementptr (i8* bitcast ([0 x %struct.anon]* @g0 to i8*), i64 -2), i8* getelementptr (i8* bitcast ([0 x %struct.anon]* @g0 to i8*), i64 -46)], align 16
// XFAIL: @g2 = global [2 x i8*] [i8* getelementptr (i8* bitcast ([0 x %struct.anon]* @g0 to i8*), i64 -2), i8* getelementptr (i8* bitcast ([0 x %struct.anon]* @g0 to i8*), i64 -46)], align 16

extern struct { unsigned char a, b; } g0[];
void *g1[] = {g0 + -1, g0 + -23 };
void *g2[] = {g0 - 1, g0 - 23 };
