// RUN: %clang_cc1 -emit-llvm %s -o - -triple=i686-pc-win32 -mconstructor-aliases -fno-rtti | FileCheck %s

struct A {
  A() : a(42) {}
  A(const A &o) : a(o.a) {}
  ~A() {}
  int a;
};

struct B {
  A foo(A o);
  A __cdecl bar(A o);
  A __stdcall baz(A o);
  A __fastcall qux(A o);
};

A B::foo(A x) {
  return x;
}

// CHECK-LABEL: define x86_thiscallcc %"struct.\01?A@@"* @"\01?foo@B@@QAE?AUA@@U2@@Z"
// CHECK:       (%"struct.\01?B@@"* %this, <{ %"struct.\01?A@@"*, %"struct.\01?A@@" }>* inalloca)
// CHECK:   getelementptr inbounds <{ %"struct.\01?A@@"*, %"struct.\01?A@@" }>* %{{.*}}, i32 0, i32 0
// CHECK:   load %"struct.\01?A@@"**
// CHECK:   ret %"struct.\01?A@@"*

A B::bar(A x) {
  return x;
}

// CHECK-LABEL: define %"struct.\01?A@@"* @"\01?bar@B@@QAA?AUA@@U2@@Z"
// CHECK:       (<{ %"struct.\01?B@@"*, %"struct.\01?A@@"*, %"struct.\01?A@@" }>* inalloca)
// CHECK:   getelementptr inbounds <{ %"struct.\01?B@@"*, %"struct.\01?A@@"*, %"struct.\01?A@@" }>* %{{.*}}, i32 0, i32 1
// CHECK:   load %"struct.\01?A@@"**
// CHECK:   ret %"struct.\01?A@@"*

A B::baz(A x) {
  return x;
}

// CHECK-LABEL: define x86_stdcallcc %"struct.\01?A@@"* @"\01?baz@B@@QAG?AUA@@U2@@Z"
// CHECK:       (<{ %"struct.\01?B@@"*, %"struct.\01?A@@"*, %"struct.\01?A@@" }>* inalloca)
// CHECK:   getelementptr inbounds <{ %"struct.\01?B@@"*, %"struct.\01?A@@"*, %"struct.\01?A@@" }>* %{{.*}}, i32 0, i32 1
// CHECK:   load %"struct.\01?A@@"**
// CHECK:   ret %"struct.\01?A@@"*

A B::qux(A x) {
  return x;
}

// CHECK-LABEL: define x86_fastcallcc void @"\01?qux@B@@QAI?AUA@@U2@@Z"
// CHECK:       (%"struct.\01?B@@"* inreg %this, %"struct.\01?A@@"* inreg noalias sret %agg.result, <{ %"struct.\01?A@@" }>* inalloca)
// CHECK:   ret void

int main() {
  B b;
  A a = b.foo(A());
  a = b.bar(a);
  a = b.baz(a);
  a = b.qux(a);
}

// CHECK: call x86_thiscallcc %"struct.\01?A@@"* @"\01?foo@B@@QAE?AUA@@U2@@Z"
// CHECK:       (%"struct.\01?B@@"* %{{[^,]*}}, <{ %"struct.\01?A@@"*, %"struct.\01?A@@" }>* inalloca %{{[^,]*}})
// CHECK: call %"struct.\01?A@@"* @"\01?bar@B@@QAA?AUA@@U2@@Z"
// CHECK:       (<{ %"struct.\01?B@@"*, %"struct.\01?A@@"*, %"struct.\01?A@@" }>* inalloca %{{[^,]*}})
// CHECK: call x86_stdcallcc %"struct.\01?A@@"* @"\01?baz@B@@QAG?AUA@@U2@@Z"
// CHECK:       (<{ %"struct.\01?B@@"*, %"struct.\01?A@@"*, %"struct.\01?A@@" }>* inalloca %{{[^,]*}})
// CHECK: call x86_fastcallcc void @"\01?qux@B@@QAI?AUA@@U2@@Z"
// CHECK:       (%"struct.\01?B@@"* inreg %{{[^,]*}}, %"struct.\01?A@@"* inreg sret %{{.*}}, <{ %"struct.\01?A@@" }>* inalloca %{{[^,]*}})
