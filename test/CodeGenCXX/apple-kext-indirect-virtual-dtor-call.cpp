// RUN: %clang_cc1 -triple x86_64-apple-darwin10 -fapple-kext -fno-rtti -emit-llvm -o - %s | FileCheck %s

// CHECK-LABEL: define void @_ZN2B1D0Ev
// CHECK: [[T1:%.*]] = load void (%struct._Z2B1*)** getelementptr inbounds (void (%struct._Z2B1*)** bitcast ([5 x i8*]* @_ZTV2B1 to void (%struct._Z2B1*)**), i64 2)
// CHECK-NEXT: call void [[T1]](%struct._Z2B1* [[T2:%.*]])
// CHECK-LABEL: define void @_Z6DELETEP2B1
// CHECK: [[T3:%.*]] = load void (%struct._Z2B1*)** getelementptr inbounds (void (%struct._Z2B1*)** bitcast ([5 x i8*]* @_ZTV2B1 to void (%struct._Z2B1*)**), i64 2)
// CHECK-NEXT:  call void [[T3]](%struct._Z2B1* [[T4:%.*]])


struct B1 { 
  virtual ~B1(); 
};

B1::~B1() {}

void DELETE(B1 *pb1) {
  pb1->B1::~B1();
}
