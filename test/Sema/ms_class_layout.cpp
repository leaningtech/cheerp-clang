// RUN: %clang_cc1 -emit-llvm-only -triple i686-pc-win32 -fdump-record-layouts %s 2>/dev/null \
// RUN:            | FileCheck %s

#pragma pack(push, 8)

class B {
public:
  virtual void b(){}
  int b_field;
protected:
private:
};

class A : public B {
public:
  int a_field;
  virtual void a(){}
  char one;
protected:
private:
};

class D {
public:
  virtual void b(){}
  double a;
};

class C : public virtual A, 
          public D, public B {
public:
  double c1_field;
  int c2_field;
  double c3_field;
  int c4_field;
  virtual void foo(){}
  virtual void bar(){}
protected:
private:
};

struct BaseStruct
{
    BaseStruct(){}
    double v0;
    float v1;
    C fg;
};

struct DerivedStruct : public BaseStruct {
  int x;
};

struct G
{
    int g_field;
};

struct H : public G, 
           public virtual D
{
};

struct I : public virtual D
{
  virtual ~I(){}
  double q;
};

struct K
{
  int k;
};

struct L
{
  int l;
};

struct M : public virtual K
{
  int m;
};

struct N : public L, public M
{
  virtual void f(){}
};

struct O : public H, public G {
  virtual void fo(){}
};

struct P : public M, public virtual L {
  int p;
};

struct R {};

class IA {
public:
  virtual ~IA(){}
  virtual void ia() = 0;
};

class ICh : public virtual IA {
public:
  virtual ~ICh(){}
  virtual void ia(){}
  virtual void iCh(){}
};

struct f {
  virtual int asd() {return -90;}
};

struct s : public virtual f {
  virtual ~s(){}
  int r;
  virtual int asd() {return -9;}
};

struct sd : virtual s, virtual ICh {
  virtual ~sd(){}
  int q;
  char y;
  virtual int asd() {return -1;}
};
struct AV { 
  virtual void foo(); 
};
struct BV : AV { 
};
struct CV : virtual BV { 
  CV(); 
  virtual void foo(); 
};
struct DV : BV {
};
struct EV : CV, DV {
};
#pragma pack(pop)

// This needs only for building layouts. 
// Without this clang doesn`t dump record layouts.
int main() {
  // This avoid "Can't yet mangle constructors!" for MS ABI.
  C* c;
  c->foo();
  DerivedStruct* v;
  H* g;
  BaseStruct* u;
  I* i;
  N* n;
  O* o;
  P* p;
  R* r;
  sd *h;
  EV *j;
  return 0;
}

// CHECK:       0 | class D
// CHECK-NEXT:  0 |   (D vftable pointer)
// CHECK-NEXT:  8 |   double a

// CHECK-NEXT: sizeof=16, align=8
// CHECK-NEXT: nvsize=16, nvalign=8

// CHECK: %"class.\01?D@@" = type { i32 (...)**, double }

// CHECK:       0 | class B
// CHECK-NEXT:  0 |   (B vftable pointer)
// CHECK-NEXT:  4 |   int b_field

// CHECK-NEXT: sizeof=8, align=4
// CHECK-NEXT: nvsize=8, nvalign=4

// CHECK: %"class.\01?B@@" = type { i32 (...)**, i32 }

// CHECK:       0 | class A
// CHECK-NEXT:  0 |   class B (primary base)
// CHECK-NEXT:  0 |     (B vftable pointer)
// CHECK-NEXT:  4 |     int b_field
// CHECK-NEXT:  8 |   int a_field
// CHECK-NEXT: 12 |   char one

// CHECK-NEXT: sizeof=16, align=4
// CHECK-NEXT: nvsize=16, nvalign=4

// CHECK:       0 | class C
// CHECK-NEXT:  0 |   class D (primary base)
// CHECK-NEXT:  0 |     (D vftable pointer)
// CHECK-NEXT:  8 |     double a
// CHECK-NEXT: 16 |   class B (base)
// CHECK-NEXT: 16 |     (B vftable pointer)
// CHECK-NEXT: 20 |     int b_field
// CHECK-NEXT: 24 |   (C vbtable pointer)
// CHECK-NEXT: 32 |   double c1_field
// CHECK-NEXT: 40 |   int c2_field
// CHECK-NEXT: 48 |   double c3_field
// CHECK-NEXT: 56 |   int c4_field
// CHECK-NEXT: 64 |   class A (virtual base)
// CHECK-NEXT: 64 |     class B (primary base)
// CHECK-NEXT: 64 |       (B vftable pointer)
// CHECK-NEXT: 68 |       int b_field
// CHECK-NEXT: 72 |     int a_field
// CHECK-NEXT: 76 |     char one

// CHECK-NEXT: sizeof=80, align=8
// CHECK-NEXT: nvsize=64, nvalign=8

// CHECK: %"class.\01?A@@" = type { %"class.\01?B@@", i32, i8 }

// CHECK: %"class.\01?C@@" = type { %"class.\01?D@@", %"class.\01?B@@", i32*, double, i32, double, i32, [4 x i8], %"class.\01?A@@" }
// CHECK: %"class.\01?C@@.base" = type { %"class.\01?D@@", %"class.\01?B@@", i32*, double, i32, double, i32 }

// CHECK:       0 | struct BaseStruct
// CHECK-NEXT:  0 |   double v0
// CHECK-NEXT:  8 |   float v1
// CHECK-NEXT: 16 |   class C fg
// CHECK-NEXT: 16 |     class D (primary base)
// CHECK-NEXT: 16 |       (D vftable pointer)
// CHECK-NEXT: 24 |       double a
// CHECK-NEXT: 32 |     class B (base)
// CHECK-NEXT: 32 |       (B vftable pointer)
// CHECK-NEXT: 36 |       int b_field
// CHECK-NEXT: 40 |     (C vbtable pointer)
// CHECK-NEXT: 48 |     double c1_field
// CHECK-NEXT: 56 |     int c2_field
// CHECK-NEXT: 64 |     double c3_field
// CHECK-NEXT: 72 |     int c4_field
// CHECK-NEXT: 80 |     class A (virtual base)
// CHECK-NEXT: 80 |       class B (primary base)
// CHECK-NEXT: 80 |         (B vftable pointer)
// CHECK-NEXT: 84 |         int b_field
// CHECK-NEXT: 88 |       int a_field
// CHECK-NEXT: 92 |       char one

// CHECK-NEXT: sizeof=80, align=8
// CHECK-NEXT: nvsize=64, nvalign=8

// CHECK: sizeof=96, align=8
// CHECK-NEXT: nvsize=96, nvalign=8

// CHECK: %"struct.\01?BaseStruct@@" = type { double, float, %"class.\01?C@@" }

// CHECK:       0 | struct DerivedStruct
// CHECK-NEXT:  0 |   struct BaseStruct (base)
// CHECK-NEXT:  0 |     double v0
// CHECK-NEXT:  8 |     float v1
// CHECK-NEXT: 16 |     class C fg
// CHECK-NEXT: 16 |       class D (primary base)
// CHECK-NEXT: 16 |         (D vftable pointer)
// CHECK-NEXT: 24 |         double a
// CHECK-NEXT: 32 |       class B (base)
// CHECK-NEXT: 32 |         (B vftable pointer)
// CHECK-NEXT: 36 |         int b_field
// CHECK-NEXT: 40 |       (C vbtable pointer)
// CHECK-NEXT: 48 |       double c1_field
// CHECK-NEXT: 56 |       int c2_field
// CHECK-NEXT: 64 |       double c3_field
// CHECK-NEXT: 72 |       int c4_field
// CHECK-NEXT: 80 |       class A (virtual base)
// CHECK-NEXT: 80 |         class B (primary base)
// CHECK-NEXT: 80 |           (B vftable pointer)
// CHECK-NEXT: 84 |           int b_field
// CHECK-NEXT: 88 |         int a_field
// CHECK-NEXT: 92 |         char one
// CHECK-NEXT: sizeof=80, align=8
// CHECK-NEXT: nvsize=64, nvalign=8

// CHECK: 96 |   int x
// CHECK-NEXT: sizeof=104, align=8
// CHECK-NEXT: nvsize=104, nvalign=8

// CHECK: %"struct.\01?DerivedStruct@@" = type { %"struct.\01?BaseStruct@@", i32 }


// CHECK:      0 | struct G
// CHECK-NEXT: 0 |   int g_field
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4

// CHECK:       0 | struct H
// CHECK-NEXT:  0 |   struct G (base)
// CHECK-NEXT:  0 |     int g_field
// CHECK-NEXT:  4 |   (H vbtable pointer)
// CHECK-NEXT:  8 |   class D (virtual base)
// CHECK-NEXT:  8 |     (D vftable pointer)
// CHECK-NEXT: 16 |     double a
// CHECK-NEXT: sizeof=24, align=8
// CHECK-NEXT: nvsize=8, nvalign=8

// CHECK: %"struct.\01?H@@" = type { %"struct.\01?G@@", i32*, %"class.\01?D@@" }

// CHECK:       0 | struct I
// CHECK-NEXT:  0 |   (I vftable pointer)
// CHECK-NEXT:  8 |   (I vbtable pointer)
// CHECK-NEXT: 16 |   double q
// CHECK-NEXT: 24 |   class D (virtual base)
// CHECK-NEXT: 24 |     (D vftable pointer)
// CHECK-NEXT: 32 |     double a
// CHECK-NEXT: sizeof=40, align=8
// CHECK-NEXT: nvsize=24, nvalign=8

// CHECK: %"struct.\01?I@@" = type { i32 (...)**, [4 x i8], i32*, double, %"class.\01?D@@" }
// CHECK: %"struct.\01?I@@.base" = type { i32 (...)**, [4 x i8], i32*, double }

// CHECK:       0 | struct L
// CHECK-NEXT:  0 |   int l
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4

// CHECK:       0 | struct K
// CHECK-NEXT:  0 |   int k
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4

// CHECK:       0 | struct M
// CHECK-NEXT:  0 |   (M vbtable pointer)
// CHECK-NEXT:  4 |   int m
// CHECK-NEXT:  8 |   struct K (virtual base)
// CHECK-NEXT:  8 |     int k
// CHECK-NEXT: sizeof=12, align=4

//CHECK: %"struct.\01?M@@" = type { i32*, i32, %"struct.\01?K@@" }
//CHECK: %"struct.\01?M@@.base" = type { i32*, i32 }

// CHECK:       0 | struct N
// CHECK-NEXT:  0 |   (N vftable pointer)
// CHECK-NEXT:  4 |   struct L (base)
// CHECK-NEXT:  4 |     int l
// CHECK-NEXT:  8 |   struct M (base)
// CHECK-NEXT:  8 |     (M vbtable pointer)
// CHECK-NEXT: 12 |     int m
// CHECK-NEXT: 16 |   struct K (virtual base)
// CHECK-NEXT: 16 |     int k
// CHECK-NEXT: sizeof=20, align=4
// CHECK-NEXT: nvsize=16, nvalign=4

//CHECK: %"struct.\01?N@@" = type { i32 (...)**, %"struct.\01?L@@", %"struct.\01?M@@.base", %"struct.\01?K@@" }

// CHECK:       0 | struct O
// CHECK-NEXT:  0 |   (O vftable pointer)
// CHECK-NEXT:  8 |   struct H (base)
// CHECK-NEXT:  8 |     struct G (base)
// CHECK-NEXT:  8 |       int g_field
// CHECK-NEXT: 12 |     (H vbtable pointer)
// CHECK-NEXT: 16 |   struct G (base)
// CHECK-NEXT: 16 |     int g_field
// CHECK-NEXT: 24 |   class D (virtual base)
// CHECK-NEXT: 24 |     (D vftable pointer)
// CHECK-NEXT: 32 |     double a
// CHECK-NEXT:    | [sizeof=40, align=8
// CHECK-NEXT:    |  nvsize=24, nvalign=8]

// CHECK: "struct.\01?O@@" = type { i32 (...)**, [4 x i8], %"struct.\01?H@@.base", %"struct.\01?G@@", %"class.\01?D@@" }
// CHECK: "struct.\01?O@@.base" = type { i32 (...)**, [4 x i8], %"struct.\01?H@@.base", %"struct.\01?G@@", [4 x i8] }


// CHECK:       0 | struct P
// CHECK-NEXT:  0 |   struct M (base)
// CHECK-NEXT:  0 |     (M vbtable pointer)
// CHECK-NEXT:  4 |     int m
// CHECK-NEXT:  8 |   int p
// CHECK-NEXT: 12 |   struct K (virtual base)
// CHECK-NEXT: 12 |     int k
// CHECK-NEXT: 16 |   struct L (virtual base)
// CHECK-NEXT: 16 |     int l
// CHECK-NEXT: sizeof=20, align=4
// CHECK-NEXT: nvsize=12, nvalign=4

//CHECK: %"struct.\01?P@@" = type { %"struct.\01?M@@.base", i32, %"struct.\01?K@@", %"struct.\01?L@@" }

// CHECK:       0 | struct R (empty)
// CHECK-NEXT:  sizeof=1, align=1
// CHECK-NEXT:  nvsize=0, nvalign=1

//CHECK: %"struct.\01?R@@" = type { i8 }

// CHECK:       0 | struct f
// CHECK-NEXT:  0 |   (f vftable pointer)
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4

// CHECK:       0 | struct s
// CHECK-NEXT:  0 |   (s vftable pointer)
// CHECK-NEXT:  4 |   (s vbtable pointer)
// CHECK-NEXT:  8 |   int r
// CHECK-NEXT: 12 |   (vtordisp for vbase f)
// CHECK-NEXT: 16 |   struct f (virtual base)
// CHECK-NEXT: 16 |     (f vftable pointer)
// CHECK-NEXT: sizeof=20, align=4
// CHECK-NEXT: nvsize=12, nvalign=4

// CHECK:       0 | class IA
// CHECK-NEXT:  0 |   (IA vftable pointer)
// CHECK-NEXT:  sizeof=4, align=4
// CHECK-NEXT:  nvsize=4, nvalign=4
	
// CHECK:       0 | class ICh
// CHECK-NEXT:  0 |   (ICh vftable pointer)
// CHECK-NEXT:  4 |   (ICh vbtable pointer)
// CHECK-NEXT:  8 |   (vtordisp for vbase IA)
// CHECK-NEXT: 12 |   class IA (virtual base)
// CHECK-NEXT: 12 |     (IA vftable pointer)
// CHECK-NEXT: sizeof=16, align=4
// CHECK-NEXT: nvsize=8, nvalign=4

// CHECK:       0 | struct sd
// CHECK-NEXT:  0 |   (sd vbtable pointer)
// CHECK-NEXT:  4 |   int q
// CHECK-NEXT:  8 |   char y
// CHECK-NEXT: 12 |   (vtordisp for vbase f)
// CHECK-NEXT: 16 |   struct f (virtual base)
// CHECK-NEXT: 16 |     (f vftable pointer)
// CHECK-NEXT: 20 |   struct s (virtual base)
// CHECK-NEXT: 20 |     (s vftable pointer)
// CHECK-NEXT: 24 |     (s vbtable pointer)
// CHECK-NEXT: 28 |     int r
// CHECK-NEXT: 32 |   (vtordisp for vbase IA)
// CHECK-NEXT: 36 |   class IA (virtual base)
// CHECK-NEXT: 36 |     (IA vftable pointer)
// CHECK-NEXT: 40 |   class ICh (virtual base)
// CHECK-NEXT: 40 |     (ICh vftable pointer)
// CHECK-NEXT: 44 |     (ICh vbtable pointer)
// CHECK-NEXT: sizeof=48, align=4
// CHECK-NEXT: nvsize=12, nvalign=4

// CHECK: %"struct.\01?f@@" = type { i32 (...)** }
// CHECK: %"struct.\01?s@@" = type { i32 (...)**, i32*, i32, i32, %"struct.\01?f@@" }
// CHECK: %"class.\01?IA@@" = type { i32 (...)** }
// CHECK: %"class.\01?ICh@@" = type { i32 (...)**, i32*, i32, %"class.\01?IA@@" }
// CHECK: %"struct.\01?sd@@" = type { i32*, i32, i8, i32, %"struct.\01?f@@", %"struct.\01?s@@.base", i32, %"class.\01?IA@@", %"class.\01?ICh@@.base" }

// CHECK:       0 | struct AV
// CHECK-NEXT:  0 |   (AV vftable pointer)
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4


// CHECK:       0 | struct BV
// CHECK-NEXT:  0 |   struct AV (primary base)
// CHECK-NEXT:  0 |     (AV vftable pointer)
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4


// CHECK:       0 | struct CV
// CHECK-NEXT:  0 |   (CV vbtable pointer)
// CHECK-NEXT:  4 |   (vtordisp for vbase BV)
// CHECK-NEXT:  8 |   struct BV (virtual base)
// CHECK-NEXT:  8 |     struct AV (primary base)
// CHECK-NEXT:  8 |       (AV vftable pointer)
// CHECK-NEXT: sizeof=12, align=4
// CHECK-NEXT: nvsize=4, nvalign=4

// CHECK: %"struct.\01?AV@@" = type { i32 (...)** }
// CHECK: %"struct.\01?BV@@" = type { %"struct.\01?AV@@" }
// CHECK: %"struct.\01?CV@@" = type { i32*, i32, %"struct.\01?BV@@" }
// CHECK: %"struct.\01?CV@@.base" = type { i32* }

// CHECK:       0 | struct DV
// CHECK-NEXT:  0 |   struct BV (primary base)
// CHECK-NEXT:  0 |     struct AV (primary base)
// CHECK-NEXT:  0 |       (AV vftable pointer)
// CHECK-NEXT: sizeof=4, align=4
// CHECK-NEXT: nvsize=4, nvalign=4

// CHECK: %"struct.\01?DV@@" = type { %"struct.\01?BV@@" }

// CHECK:       0 | struct EV
// CHECK-NEXT:  0 |   struct DV (primary base)
// CHECK-NEXT:  0 |     struct BV (primary base)
// CHECK-NEXT:  0 |       struct AV (primary base)
// CHECK-NEXT:  0 |         (AV vftable pointer)
// CHECK-NEXT:  4 |   struct CV (base)
// CHECK-NEXT:  4 |     (CV vbtable pointer)
// CHECK-NEXT:  8 |   (vtordisp for vbase BV)
// CHECK-NEXT: 12 |   struct BV (virtual base)
// CHECK-NEXT: 12 |     struct AV (primary base)
// CHECK-NEXT: 12 |       (AV vftable pointer)
// CHECK-NEXT: sizeof=16, align=4
// CHECK-NEXT: nvsize=8, nvalign=4

// CHECK: %"struct.\01?EV@@" = type { %"struct.\01?DV@@", %"struct.\01?CV@@.base", i32, %"struct.\01?BV@@" }
// CHECK: %"struct.\01?EV@@.base" = type { %"struct.\01?DV@@", %"struct.\01?CV@@.base" }

// Overriding a method means that all the vbases containing that
// method need a vtordisp.  Note: this code will cause an error in cl.exe.
namespace test1 {
  struct A { virtual void foo(); };
  struct B : A {};
  struct C : virtual A, virtual B { C(); virtual void foo(); };
  void test() { C *c; }

// CHECK:        0 | struct test1::C
// CHECK-NEXT:   0 |   (C vbtable pointer)
// CHECK-NEXT:   4 |   (vtordisp for vbase A)
// CHECK-NEXT:   8 |   struct test1::A (virtual base)
// CHECK-NEXT:   8 |     (A vftable pointer)
// CHECK-NEXT:  12 |   (vtordisp for vbase B)
// CHECK-NEXT:  16 |   struct test1::B (virtual base)
// CHECK-NEXT:  16 |     struct test1::A (primary base)
// CHECK-NEXT:  16 |       (A vftable pointer)
// CHECK-NEXT:  sizeof=20, align=4
// CHECK-NEXT:  nvsize=4, nvalign=4
}
