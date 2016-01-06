// RUN: %clang_cc1 -emit-llvm %s -o - -triple=x86_64-apple-darwin10 -mconstructor-aliases | FileCheck %s

struct A { 
  A();
};

// CHECK: @_ZN1AC1Ev = alias {{.*}} @_ZN1AC2Ev
// CHECK-LABEL: define void @_ZN1AC2Ev(%struct._Z1A* %this) unnamed_addr
A::A() { }

struct B : virtual A { 
  B();
};

// CHECK-LABEL: define void @_ZN1BC2Ev(%struct._Z1B* %this, i8** %vtt) unnamed_addr
// CHECK-LABEL: define void @_ZN1BC1Ev(%struct._Z1B* %this) unnamed_addr
B::B() { }

struct C : virtual A {
  C(bool);
};

// CHECK-LABEL: define void @_ZN1CC2Eb(%struct._Z1C* %this, i8** %vtt, i1 zeroext) unnamed_addr
// CHECK-LABEL: define void @_ZN1CC1Eb(%struct._Z1C* %this, i1 zeroext) unnamed_addr
C::C(bool) { }

// PR6251
namespace PR6251 {

// Test that we don't call the A<char> constructor twice.

template<typename T>
struct A { A(); };

struct B : virtual A<char> { };
struct C : virtual A<char> { };

struct D : B, C  {
  D();
};

// CHECK-LABEL: define void @_ZN6PR62511DC1Ev(%struct._ZN6PR62511DE* %this) unnamed_addr
// CHECK: call void @_ZN6PR62511AIcEC2Ev
// CHECK-NOT: call void @_ZN6PR62511AIcEC2Ev
// CHECK: ret void
D::D() { }

}
