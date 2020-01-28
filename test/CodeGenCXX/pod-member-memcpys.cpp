// RUN: %clang_cc1 -triple x86_64-apple-darwin10 -emit-llvm -std=c++03 -fexceptions -fcxx-exceptions -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i386-apple-darwin10 -emit-llvm -std=c++03 -o - %s | FileCheck --check-prefix=CHECK-2 %s

struct POD {
  int w, x, y, z;
};

struct PODLike {
  int w, x, y, z;
  PODLike();
  ~PODLike();
};

struct NonPOD {
  NonPOD();
  NonPOD(const NonPOD&);
  NonPOD& operator=(const NonPOD&);
};

struct Basic {
  int a, b, c, d;
  NonPOD np;
  int w, x, y, z;
};

struct PODMember {
  int a, b, c, d;
  POD p;
  NonPOD np;
  int w, x, y, z;
};

struct PODLikeMember {
  int a, b, c, d;
  PODLike pl;
  NonPOD np;
  int w, x, y, z;
};

struct ArrayMember {
  int a, b, c, d;
  int e[12];
  NonPOD np;
  int f[12];
  int w, x, y, z;
};

struct VolatileMember {
  int a, b, c, d;
  volatile int v;
  NonPOD np;
  int w, x, y, z;
};

struct BitfieldMember {
  int a, b, c, d;
  NonPOD np;
  int w : 6;
  int x : 6;
  int y : 6;
  int z : 6;
};

struct BitfieldMember2 {
  unsigned a : 1;
  unsigned b, c, d;
  NonPOD np;
};

struct BitfieldMember3 {
  virtual void f();
  int   : 8;
  int x : 1;
  int y;
};

struct InnerClassMember {
  struct {
    int a, b, c, d;
  } a;
  int b, c, d, e;
  NonPOD np;
  int w, x, y, z;
};

struct ReferenceMember {
  ReferenceMember(int &a, int &b, int &c, int &d)
    : a(a), b(b), c(c), d(d) {}
  int &a;
  int &b;
  NonPOD np;
  int &c;
  int &d;
};

struct __attribute__((packed)) PackedMembers {
  char c;
  NonPOD np;
  int w, x, y, z;
};

// COPY-ASSIGNMENT OPERATORS:

// Assignment operators are output in the order they're encountered.

#define CALL_AO(T) void callAO##T(T& a, const T& b) { a = b; } 

CALL_AO(Basic)
CALL_AO(PODMember)
CALL_AO(PODLikeMember)
CALL_AO(ArrayMember)
CALL_AO(VolatileMember)
CALL_AO(BitfieldMember)
CALL_AO(InnerClassMember)
CALL_AO(PackedMembers)

// Basic copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z5Basic* @_ZN5BasicaSERKS_(%struct._Z5Basic* %this, %struct._Z5Basic* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret %struct._Z5Basic*

// PODMember copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z9PODMember* @_ZN9PODMemberaSERKS_(%struct._Z9PODMember* %this, %struct._Z9PODMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 32, i32 4{{.*}})
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret %struct._Z9PODMember*

// PODLikeMember copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z13PODLikeMember* @_ZN13PODLikeMemberaSERKS_(%struct._Z13PODLikeMember* %this, %struct._Z13PODLikeMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 32, i32 4{{.*}})
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret %struct._Z13PODLikeMember*

// ArrayMember copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z11ArrayMember* @_ZN11ArrayMemberaSERKS_(%struct._Z11ArrayMember* %this, %struct._Z11ArrayMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 64, i32 4{{.*}})
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 64, i32 4{{.*}})
// CHECK: ret %struct._Z11ArrayMember*

// VolatileMember copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z14VolatileMember* @_ZN14VolatileMemberaSERKS_(%struct._Z14VolatileMember* %this, %struct._Z14VolatileMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: load volatile i32, i32* {{.*}}, align 4
// CHECK: store volatile i32 {{.*}}, align 4
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret %struct._Z14VolatileMember*

// BitfieldMember copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z14BitfieldMember* @_ZN14BitfieldMemberaSERKS_(%struct._Z14BitfieldMember* %this, %struct._Z14BitfieldMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 3, i32 1{{.*}})
// CHECK: ret %struct._Z14BitfieldMember*

// InnerClass copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z16InnerClassMember* @_ZN16InnerClassMemberaSERKS_(%struct._Z16InnerClassMember* %this, %struct._Z16InnerClassMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 32, i32 4{{.*}})
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret %struct._Z16InnerClassMember*

// PackedMembers copy-assignment:
// CHECK-LABEL: define linkonce_odr dereferenceable({{[0-9]+}}) %struct._Z13PackedMembers* @_ZN13PackedMembersaSERKS_(%struct._Z13PackedMembers* %this, %struct._Z13PackedMembers* dereferenceable({{[0-9]+}}))
// CHECK: call dereferenceable({{[0-9]+}}) %struct._Z6NonPOD* @_ZN6NonPODaSERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 1{{.*}})
// CHECK: ret %struct._Z13PackedMembers*

// COPY-CONSTRUCTORS:

// Clang outputs copy-constructors in the reverse of the order that
// copy-constructor calls are encountered. Add functions that call the copy
// constructors of the classes above in reverse order here.

#define CALL_CC(T) T callCC##T(const T& b) { return b; }

CALL_CC(PackedMembers)
// PackedMembers copy-assignment:
// CHECK-LABEL: define linkonce_odr void @_ZN13PackedMembersC2ERKS_(%struct._Z13PackedMembers* %this, %struct._Z13PackedMembers* dereferenceable({{[0-9]+}}))
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 1{{.*}})
// CHECK: ret void

CALL_CC(BitfieldMember2)
// BitfieldMember2 copy-constructor:
// CHECK-2-LABEL: define linkonce_odr void @_ZN15BitfieldMember2C2ERKS_(%struct._Z15BitfieldMember2* %this, %struct._Z15BitfieldMember2* dereferenceable({{[0-9]+}}))
// CHECK-2: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4, i1 false)
// CHECK-2: call void @_ZN6NonPODC1ERKS_
// CHECK-2: ret void

CALL_CC(BitfieldMember3)
// BitfieldMember3 copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN15BitfieldMember3C2ERKS_(%struct._Z15BitfieldMember3* %this, %struct._Z15BitfieldMember3* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 8, i32 8, i1 false)
// CHECK: ret void

CALL_CC(ReferenceMember)
// ReferenceMember copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN15ReferenceMemberC2ERKS_(%struct._Z15ReferenceMember* %this, %struct._Z15ReferenceMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 8{{.*}})
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 8{{.*}})
// CHECK: ret void

CALL_CC(InnerClassMember)
// InnerClass copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN16InnerClassMemberC2ERKS_(%struct._Z16InnerClassMember* %this, %struct._Z16InnerClassMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 32, i32 4{{.*}})
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret void

CALL_CC(BitfieldMember)
// BitfieldMember copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN14BitfieldMemberC2ERKS_(%struct._Z14BitfieldMember* %this, %struct._Z14BitfieldMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 3, i32 1{{.*}})
// CHECK: ret void

CALL_CC(VolatileMember)
// VolatileMember copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN14VolatileMemberC2ERKS_(%struct._Z14VolatileMember* %this, %struct._Z14VolatileMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: load volatile i32, i32* {{.*}}, align 4
// CHECK: store volatile i32 {{.*}}, align 4
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret void

CALL_CC(ArrayMember)
// ArrayMember copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN11ArrayMemberC2ERKS_(%struct._Z11ArrayMember* %this, %struct._Z11ArrayMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 64, i32 4{{.*}})
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 64, i32 4{{.*}})
// CHECK: ret void

CALL_CC(PODLikeMember)
// PODLikeMember copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN13PODLikeMemberC2ERKS_(%struct._Z13PODLikeMember* %this, %struct._Z13PODLikeMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 32, i32 4{{.*}})
// CHECK: invoke void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret void
// CHECK: landingpad
// CHECK: invoke void @_ZN7PODLikeD1Ev

CALL_CC(PODMember)
// PODMember copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN9PODMemberC2ERKS_(%struct._Z9PODMember* %this, %struct._Z9PODMember* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 32, i32 4{{.*}})
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret void

CALL_CC(Basic)
// Basic copy-constructor:
// CHECK-LABEL: define linkonce_odr void @_ZN5BasicC2ERKS_(%struct._Z5Basic* %this, %struct._Z5Basic* dereferenceable({{[0-9]+}}))
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: call void @_ZN6NonPODC1ERKS_
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64({{.*}}i64 16, i32 4{{.*}})
// CHECK: ret void
