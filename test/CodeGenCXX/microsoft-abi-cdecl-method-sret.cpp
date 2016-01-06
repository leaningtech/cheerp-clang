// RUN: %clang_cc1 -triple i386-pc-win32 -emit-llvm %s -o - | FileCheck %s

// PR15768

// A trivial 12 byte struct is returned indirectly.
struct S {
  S();
  int a, b, c;
};

struct C {
  S variadic_sret(const char *f, ...);
  S __cdecl cdecl_sret();
  S __cdecl byval_and_sret(S a);
  int c;
};

S C::variadic_sret(const char *f, ...) { return S(); }
S C::cdecl_sret() { return S(); }
S C::byval_and_sret(S a) { return S(); }

// CHECK: define void @"\01?variadic_sret@C@@QAA?AUS@@PBDZZ"(%"struct.\01?C@@"* %this, %"struct.\01?S@@"* noalias sret %agg.result, i8* %f, ...)
// CHECK: define void @"\01?cdecl_sret@C@@QAA?AUS@@XZ"(%"struct.\01?C@@"* %this, %"struct.\01?S@@"* noalias sret %agg.result)
// CHECK: define void @"\01?byval_and_sret@C@@QAA?AUS@@U2@@Z"(%"struct.\01?C@@"* %this, %"struct.\01?S@@"* noalias sret %agg.result, %"struct.\01?S@@"* byval align 4 %a)

int main() {
  C c;
  c.variadic_sret("asdf");
  c.cdecl_sret();
  c.byval_and_sret(S());
}
// CHECK-LABEL: define i32 @main()
// CHECK: call void {{.*}} @"\01?variadic_sret@C@@QAA?AUS@@PBDZZ"
// CHECK: call void @"\01?cdecl_sret@C@@QAA?AUS@@XZ"
// CHECK: call void @"\01?byval_and_sret@C@@QAA?AUS@@U2@@Z"

// __fastcall has similar issues.
struct A {
  S __fastcall f(int x);
};
S A::f(int x) {
  return S();
}
// CHECK-LABEL: define x86_fastcallcc void @"\01?f@A@@QAI?AUS@@H@Z"(%"struct.\01?A@@"* inreg %this, %"struct.\01?S@@"* inreg noalias sret %agg.result, i32 %x)
