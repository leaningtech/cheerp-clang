// RUN: %clang_cc1 -fno-rtti -emit-llvm %s -o - -mconstructor-aliases -triple=i386-pc-win32 | FileCheck %s

struct Left {
  virtual void left();
};

struct Right {
  virtual void right();
};

struct ChildNoOverride : Left, Right {
};

struct ChildOverride : Left, Right {
  virtual void left();
  virtual void right();
};

extern "C" void foo(void *);

void call_left_no_override(ChildNoOverride *child) {
// CHECK: define void @"\01?call_left_no_override
// CHECK: %[[CHILD:.*]] = load %"struct.\01?ChildNoOverride@@"

  child->left();
// Only need to cast 'this' to Left*.
// CHECK: %[[LEFT:.*]] = bitcast %"struct.\01?ChildNoOverride@@"* %[[CHILD]] to %"struct.\01?Left@@"*
// CHECK: %[[VFPTR:.*]] = getelementptr %"struct.\01?Left@@"* %[[LEFT]], i32 0, i32 0
// CHECK: %[[VFTABLE0:.*]] = load i32 (...)*** %[[VFPTR]]
// CHECK: %[[VFTABLE:.*]] = bitcast i32 (...)** %[[VFTABLE0]] to void (%"struct.\01?Left@@"*)**
// CHECK: %[[VFUN:.*]] = getelementptr inbounds void (%"struct.\01?Left@@"*)** %[[VFTABLE]], i64 0
// CHECK: %[[VFUN_VALUE:.*]] = load void (%"struct.\01?Left@@"*)** %[[VFUN]]
// CHECK: call x86_thiscallcc void %[[VFUN_VALUE]](%"struct.\01?Left@@"* %[[LEFT]])
// CHECK: ret
}

void ChildOverride::left() {
// CHECK: define x86_thiscallcc void @"\01?left@ChildOverride@@UAEXXZ"(%"struct.\01?ChildOverride@@"* %[[THIS:.*]])
//
// No need to adjust 'this' as the ChildOverride's layout begins with Left.
// CHECK: %[[THIS_ADDR:.*]] = alloca %"struct.\01?ChildOverride@@"*, align 4
// CHECK: store %"struct.\01?ChildOverride@@"* %[[THIS]], %"struct.\01?ChildOverride@@"** %[[THIS_ADDR]], align 4

  foo(this);
// CHECK: %[[THIS:.*]] = load %"struct.\01?ChildOverride@@"** %[[THIS_ADDR]]
// CHECK: %[[THIS_i8:.*]] = bitcast %"struct.\01?ChildOverride@@"* %[[THIS]] to i8*
// CHECK: call void @foo(i8* %[[THIS_i8]])
// CHECK: ret
}

void call_left_override(ChildOverride *child) {
// CHECK: define void @"\01?call_left_override
// CHECK: %[[CHILD:.*]] = load %"struct.\01?ChildOverride@@"

  child->left();
// CHECK: %[[VFPTR:.*]] = getelementptr %"struct.\01?ChildOverride@@"* %[[CHILD]], i32 0, i32 0
// CHECK: %[[VFTABLE:.*]] = bitcast {{.*}} %[[VFPTR]] to void (%"struct.\01?ChildOverride@@"*)***
// CHECK: %[[VFTABLE0:.*]] = load void (%"struct.\01?ChildOverride@@"*)*** %[[VFTABLE]]
// CHECK: %[[VFUN:.*]] = getelementptr inbounds void (%"struct.\01?ChildOverride@@"*)** %[[VFTABLE0]], i64 0
// CHECK: %[[VFUN_VALUE:.*]] = load void (%"struct.\01?ChildOverride@@"*)** %[[VFUN]]
//
// CHECK: call x86_thiscallcc void %[[VFUN_VALUE]](%"struct.\01?ChildOverride@@"* %[[CHILD]])
// CHECK: ret
}

void call_right_no_override(ChildNoOverride *child) {
// CHECK: define void @"\01?call_right_no_override
// CHECK: %[[CHILD:.*]] = load %"struct.\01?ChildNoOverride@@"

  child->right();
// When calling a right base's virtual method, one needs to adjust 'this' at
// the caller site.
//
// CHECK: %[[CHILD_i8:.*]] = bitcast %"struct.\01?ChildNoOverride@@"* %[[CHILD]] to i8*
// CHECK: %[[RIGHT_i8:.*]] = getelementptr inbounds i8* %[[CHILD_i8]], i32 4
// CHECK: %[[RIGHT:.*]] = bitcast i8* %[[RIGHT_i8]] to %"struct.\01?Right@@"*
//
// CHECK: %[[VFPTR:.*]] = getelementptr %"struct.\01?Right@@"* %[[RIGHT]], i32 0, i32 0
// CHECK: %[[VFTABLE0:.*]] = load i32 (...)*** %[[VFPTR]]
// CHECK: %[[VFTABLE:.*]] = bitcast i32 (...)** %[[VFTABLE0]] to void (%"struct.\01?Right@@"*)**
// CHECK: %[[VFUN:.*]] = getelementptr inbounds void (%"struct.\01?Right@@"*)** %[[VFTABLE]], i64 0
// CHECK: %[[VFUN_VALUE:.*]] = load void (%"struct.\01?Right@@"*)** %[[VFUN]]
// CHECK: call x86_thiscallcc void %[[VFUN_VALUE]](%"struct.\01?Right@@"* %[[RIGHT]])
// CHECK: ret
}

void ChildOverride::right() {
// CHECK: define x86_thiscallcc void @"\01?right@ChildOverride@@UAEXXZ"(i8*
//
// ChildOverride::right gets 'this' cast to Right* in ECX (i.e. this+4) so we
// need to adjust 'this' before use.
//
// CHECK: %[[THIS_ADDR:.*]] = alloca %"struct.\01?ChildOverride@@"*, align 4
// CHECK: %[[THIS_i8:.*]] = getelementptr inbounds i8* %[[ECX:.*]], i32 -4
// CHECK: %[[THIS:.*]] = bitcast i8* %[[THIS_i8]] to %"struct.\01?ChildOverride@@"*
// CHECK: store %"struct.\01?ChildOverride@@"* %[[THIS]], %"struct.\01?ChildOverride@@"** %[[THIS_ADDR]], align 4

  foo(this);
// CHECK: %[[THIS:.*]] = load %"struct.\01?ChildOverride@@"** %[[THIS_ADDR]]
// CHECK: %[[THIS_PARAM:.*]] = bitcast %"struct.\01?ChildOverride@@"* %[[THIS]] to i8*
// CHECK: call void @foo(i8* %[[THIS_PARAM]])
// CHECK: ret
}

void call_right_override(ChildOverride *child) {
// CHECK: define void @"\01?call_right_override
// CHECK: %[[CHILD:.*]] = load %"struct.\01?ChildOverride@@"

  child->right();
// When calling a right child's virtual method, one needs to adjust 'this' at
// the caller site.
//
// CHECK: %[[CHILD_i8:.*]] = bitcast %"struct.\01?ChildOverride@@"* %[[CHILD]] to i8*
//
// CHECK: %[[VFPTR_i8:.*]] = getelementptr inbounds i8* %[[CHILD_i8]], i32 4
// CHECK: %[[VFPTR:.*]] = bitcast {{.*}} %[[VFPTR_i8]] to
// CHECK: %[[VFPTR_GEP:.*]] = getelementptr {{.*}} %[[VFPTR]]
// CHECK: %[[VFPTR2:.*]] = bitcast {{.*}} %[[VFPTR_GEP]] to
// CHECK: %[[VFTABLE:.*]] = load void (i8*)*** %[[VFPTR2]]
// CHECK: %[[VFUN:.*]] = getelementptr inbounds void (i8*)** %[[VFTABLE]], i64 0
// CHECK: %[[VFUN_VALUE:.*]] = load void (i8*)** %[[VFUN]]
//
// CHECK: %[[CHILD_i8:.*]] = bitcast %"struct.\01?ChildOverride@@"* %[[CHILD]] to i8*
// CHECK: %[[RIGHT:.*]] = getelementptr inbounds i8* %[[CHILD_i8]], i32 4
// CHECK: %[[RIGHT1:.*]] = bitcast {{.*}} %[[RIGHT]] to
// CHECK: %[[RIGHT2:.*]] = bitcast {{.*}} %[[RIGHT1]] to
//
// CHECK: call x86_thiscallcc void %[[VFUN_VALUE]](i8* %[[RIGHT2]])
// CHECK: ret
}

struct GrandchildOverride : ChildOverride {
  virtual void right();
};

void GrandchildOverride::right() {
// CHECK: define x86_thiscallcc void @"\01?right@GrandchildOverride@@UAEXXZ"(i8*
//
// CHECK: %[[THIS_ADDR:.*]] = alloca %"struct.\01?GrandchildOverride@@"*, align 4
// CHECK: %[[THIS_i8:.*]] = getelementptr inbounds i8* %[[ECX:.*]], i32 -4
// CHECK: %[[THIS:.*]] = bitcast i8* %[[THIS_i8]] to %"struct.\01?GrandchildOverride@@"*
// CHECK: store %"struct.\01?GrandchildOverride@@"* %[[THIS]], %"struct.\01?GrandchildOverride@@"** %[[THIS_ADDR]], align 4

  foo(this);
// CHECK: %[[THIS:.*]] = load %"struct.\01?GrandchildOverride@@"** %[[THIS_ADDR]]
// CHECK: %[[THIS_PARAM:.*]] = bitcast %"struct.\01?GrandchildOverride@@"* %[[THIS]] to i8*
// CHECK: call void @foo(i8* %[[THIS_PARAM]])
// CHECK: ret
}

void call_grandchild_right(GrandchildOverride *obj) {
  // Just make sure we don't crash.
  obj->right();
}

void emit_ctors() {
  Left l;
  // CHECK: define {{.*}} @"\01??0Left@@QAE@XZ"
  // CHECK-NOT: getelementptr
  // CHECK:   store i32 (...)** bitcast ([1 x i8*]* @"\01??_7Left@@6B@" to i32 (...)**)
  // CHECK: ret

  Right r;
  // CHECK: define {{.*}} @"\01??0Right@@QAE@XZ"
  // CHECK-NOT: getelementptr
  // CHECK:   store i32 (...)** bitcast ([1 x i8*]* @"\01??_7Right@@6B@" to i32 (...)**)
  // CHECK: ret

  ChildOverride co;
  // CHECK: define {{.*}} @"\01??0ChildOverride@@QAE@XZ"
  // CHECK:   %[[THIS:.*]] = load %"struct.\01?ChildOverride@@"**
  // CHECK:   %[[VFPTR:.*]] = bitcast %"struct.\01?ChildOverride@@"* %[[THIS]] to i32 (...)***
  // CHECK:   store i32 (...)** bitcast ([1 x i8*]* @"\01??_7ChildOverride@@6BLeft@@@" to i32 (...)**), i32 (...)*** %[[VFPTR]]
  // CHECK:   %[[THIS_i8:.*]] = bitcast %"struct.\01?ChildOverride@@"* %[[THIS]] to i8*
  // CHECK:   %[[VFPTR_i8:.*]] = getelementptr inbounds i8* %[[THIS_i8]], i32 4
  // CHECK:   %[[VFPTR:.*]] = bitcast i8* %[[VFPTR_i8]] to i32 (...)***
  // CHECK:   store i32 (...)** bitcast ([1 x i8*]* @"\01??_7ChildOverride@@6BRight@@@" to i32 (...)**), i32 (...)*** %[[VFPTR]]
  // CHECK: ret

  GrandchildOverride gc;
  // CHECK: define {{.*}} @"\01??0GrandchildOverride@@QAE@XZ"
  // CHECK:   %[[THIS:.*]] = load %"struct.\01?GrandchildOverride@@"**
  // CHECK:   %[[VFPTR:.*]] = bitcast %"struct.\01?GrandchildOverride@@"* %[[THIS]] to i32 (...)***
  // CHECK:   store i32 (...)** bitcast ([1 x i8*]* @"\01??_7GrandchildOverride@@6BLeft@@@" to i32 (...)**), i32 (...)*** %[[VFPTR]]
  // CHECK:   %[[THIS_i8:.*]] = bitcast %"struct.\01?GrandchildOverride@@"* %[[THIS]] to i8*
  // CHECK:   %[[VFPTR_i8:.*]] = getelementptr inbounds i8* %[[THIS_i8]], i32 4
  // CHECK:   %[[VFPTR:.*]] = bitcast i8* %[[VFPTR_i8]] to i32 (...)***
  // CHECK:   store i32 (...)** bitcast ([1 x i8*]* @"\01??_7GrandchildOverride@@6BRight@@@" to i32 (...)**), i32 (...)*** %[[VFPTR]]
  // CHECK: ret
}

struct LeftWithNonVirtualDtor {
  virtual void left();
  ~LeftWithNonVirtualDtor();
};

struct AsymmetricChild : LeftWithNonVirtualDtor, Right {
  virtual ~AsymmetricChild();
};

void call_asymmetric_child_complete_dtor() {
  // CHECK-LABEL: define void @"\01?call_asymmetric_child_complete_dtor@@YAXXZ"
  AsymmetricChild obj;
  // CHECK: call x86_thiscallcc %"struct.\01?AsymmetricChild@@"* @"\01??0AsymmetricChild@@QAE@XZ"(%"struct.\01?AsymmetricChild@@"* %[[OBJ:.*]])
  // CHECK-NOT: getelementptr
  // CHECK: call x86_thiscallcc void @"\01??1AsymmetricChild@@UAE@XZ"(%"struct.\01?AsymmetricChild@@"* %[[OBJ]])
  // CHECK: ret
}
