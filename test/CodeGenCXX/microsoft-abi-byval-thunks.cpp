// RUN: %clang_cc1 %s -fno-rtti -triple=i686-pc-win32 -emit-llvm -o - | FileCheck --check-prefix=CHECK32 %s
// RUN: %clang_cc1 %s -fno-rtti -triple=x86_64-pc-win32 -emit-llvm -o - | FileCheck --check-prefix=CHECK64 %s

namespace byval_thunk {
struct Agg {
  Agg();
  Agg(const Agg &);
  ~Agg();
  int x;
};

struct A { virtual void foo(Agg x); };
struct B { virtual void foo(Agg x); };
struct C : A, B { C(); virtual void foo(Agg x); };
C::C() {} // force emission

// CHECK32-LABEL: define linkonce_odr x86_thiscallcc void @"\01?foo@C@byval_thunk@@W3AEXUAgg@2@@Z"
// CHECK32:             (%"struct.\01?C@byval_thunk@@"* %this, <{ %"struct.\01?Agg@byval_thunk@@" }>* inalloca)
// CHECK32:   getelementptr i8* %{{.*}}, i32 -4
// CHECK32:   musttail call x86_thiscallcc void @"\01?foo@C@byval_thunk@@UAEXUAgg@2@@Z"
// CHECK32:       (%"struct.\01?C@byval_thunk@@"* %{{.*}}, <{ %"struct.\01?Agg@byval_thunk@@" }>* inalloca %0)
// CHECK32-NEXT: ret void

// CHECK64-LABEL: define linkonce_odr void @"\01?foo@C@byval_thunk@@W7EAAXUAgg@2@@Z"
// CHECK64:             (%"struct.\01?C@byval_thunk@@"* %this, %"struct.\01?Agg@byval_thunk@@"* %x)
// CHECK64:   getelementptr i8* %{{.*}}, i32 -8
// CHECK64:   call void @"\01?foo@C@byval_thunk@@UEAAXUAgg@2@@Z"
// CHECK64:       (%"struct.\01?C@byval_thunk@@"* %{{.*}}, %"struct.\01?Agg@byval_thunk@@"* %x)
// CHECK64-NOT: call
// CHECK64:   ret void
}

namespace stdcall_thunk {
struct Agg {
  Agg();
  Agg(const Agg &);
  ~Agg();
  int x;
};

struct A { virtual void __stdcall foo(Agg x); };
struct B { virtual void __stdcall foo(Agg x); };
struct C : A, B { C(); virtual void __stdcall foo(Agg x); };
C::C() {} // force emission

// CHECK32-LABEL: define linkonce_odr x86_stdcallcc void @"\01?foo@C@stdcall_thunk@@W3AGXUAgg@2@@Z"
// CHECK32:             (<{ %"struct.\01?C@stdcall_thunk@@"*, %"struct.\01?Agg@stdcall_thunk@@" }>* inalloca)
// CHECK32:   %[[this_slot:[^ ]*]] = getelementptr inbounds <{ %"struct.\01?C@stdcall_thunk@@"*, %"struct.\01?Agg@stdcall_thunk@@" }>* %0, i32 0, i32 0
// CHECK32:   load %"struct.\01?C@stdcall_thunk@@"** %[[this_slot]]
// CHECK32:   getelementptr i8* %{{.*}}, i32 -4
// CHECK32:   store %"struct.\01?C@stdcall_thunk@@"* %{{.*}}, %"struct.\01?C@stdcall_thunk@@"** %[[this_slot]]
// CHECK32:   musttail call x86_stdcallcc void @"\01?foo@C@stdcall_thunk@@UAGXUAgg@2@@Z"
// CHECK32:       (<{ %"struct.\01?C@stdcall_thunk@@"*, %"struct.\01?Agg@stdcall_thunk@@" }>*  inalloca %0)
// CHECK32-NEXT: ret void

// CHECK64-LABEL: define linkonce_odr void @"\01?foo@C@stdcall_thunk@@W7EAAXUAgg@2@@Z"
// CHECK64:             (%"struct.\01?C@stdcall_thunk@@"* %this, %"struct.\01?Agg@stdcall_thunk@@"* %x)
// CHECK64:   getelementptr i8* %{{.*}}, i32 -8
// CHECK64:   call void @"\01?foo@C@stdcall_thunk@@UEAAXUAgg@2@@Z"
// CHECK64:       (%"struct.\01?C@stdcall_thunk@@"* %{{.*}}, %"struct.\01?Agg@stdcall_thunk@@"* %x)
// CHECK64-NOT: call
// CHECK64:   ret void
}

namespace sret_thunk {
struct Agg {
  Agg();
  Agg(const Agg &);
  ~Agg();
  int x;
};

struct A { virtual Agg __cdecl foo(Agg x); };
struct B { virtual Agg __cdecl foo(Agg x); };
struct C : A, B { C(); virtual Agg __cdecl foo(Agg x); };
C::C() {} // force emission

// CHECK32-LABEL: define linkonce_odr %"struct.\01?Agg@sret_thunk@@"* @"\01?foo@C@sret_thunk@@W3AA?AUAgg@2@U32@@Z"
// CHECK32:             (<{ %"struct.\01?C@sret_thunk@@"*, %"struct.\01?Agg@sret_thunk@@"*, %"struct.\01?Agg@sret_thunk@@" }>* inalloca)
// CHECK32:   %[[this_slot:[^ ]*]] = getelementptr inbounds <{ %"struct.\01?C@sret_thunk@@"*, %"struct.\01?Agg@sret_thunk@@"*, %"struct.\01?Agg@sret_thunk@@" }>* %0, i32 0, i32 0
// CHECK32:   load %"struct.\01?C@sret_thunk@@"** %[[this_slot]]
// CHECK32:   getelementptr i8* %{{.*}}, i32 -4
// CHECK32:   store %"struct.\01?C@sret_thunk@@"* %{{.*}}, %"struct.\01?C@sret_thunk@@"** %[[this_slot]]
// CHECK32:   %[[rv:[^ ]*]] = musttail call %"struct.\01?Agg@sret_thunk@@"* @"\01?foo@C@sret_thunk@@UAA?AUAgg@2@U32@@Z"
// CHECK32:       (<{ %"struct.\01?C@sret_thunk@@"*, %"struct.\01?Agg@sret_thunk@@"*, %"struct.\01?Agg@sret_thunk@@" }>*  inalloca %0)
// CHECK32-NEXT: ret %"struct.\01?Agg@sret_thunk@@"* %[[rv]]

// CHECK64-LABEL: define linkonce_odr void @"\01?foo@C@sret_thunk@@W7EAA?AUAgg@2@U32@@Z"
// CHECK64:             (%"struct.\01?C@sret_thunk@@"* %this, %"struct.\01?Agg@sret_thunk@@"* noalias sret %agg.result, %"struct.\01?Agg@sret_thunk@@"* %x)
// CHECK64:   getelementptr i8* %{{.*}}, i32 -8
// CHECK64:   call void @"\01?foo@C@sret_thunk@@UEAA?AUAgg@2@U32@@Z"
// CHECK64:       (%"struct.\01?C@sret_thunk@@"* %{{.*}}, %"struct.\01?Agg@sret_thunk@@"* sret %agg.result, %"struct.\01?Agg@sret_thunk@@"* %x)
// CHECK64-NOT: call
// CHECK64:   ret void
}

#if 0
// FIXME: When we extend LLVM IR to allow forwarding of varargs through musttail
// calls, use this test.
namespace variadic_thunk {
struct Agg {
  Agg();
  Agg(const Agg &);
  ~Agg();
  int x;
};

struct A { virtual void foo(Agg x, ...); };
struct B { virtual void foo(Agg x, ...); };
struct C : A, B { C(); virtual void foo(Agg x, ...); };
C::C() {} // force emission
}
#endif
