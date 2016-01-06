// RUN: %clang_cc1 %s -triple=arm64-apple-ios7.0.0 -emit-llvm -o - | FileCheck %s
// rdar://12162905

struct S {
  S();
  int iField;
};

S::S() {
  iField = 1;
};

// CHECK: %struct._Z1S* @_ZN1SC2Ev(%struct._Z1S* returned %this)

// CHECK: %struct._Z1S* @_ZN1SC1Ev(%struct._Z1S* returned %this)
// CHECK: [[THISADDR:%[a-zA-z0-9.]+]] = alloca %struct._Z1S*
// CHECK: store %struct._Z1S* %this, %struct._Z1S** [[THISADDR]]
// CHECK: [[THIS1:%.*]] = load %struct._Z1S** [[THISADDR]]
// CHECK: ret %struct._Z1S* [[THIS1]]
