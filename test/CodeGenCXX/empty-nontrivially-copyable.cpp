// RUN: %clang_cc1 -triple armv7-apple-ios -x c++ -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple arm64-apple-ios -x c++ -emit-llvm -o - %s | FileCheck %s

// According to the Itanium ABI (3.1.1), types with non-trivial copy
// constructors passed by value should be passed indirectly, with the caller
// creating a temporary.

struct Empty;

struct Empty {
  Empty(const Empty &e);
  bool check();
};

bool foo(Empty e) {
// CHECK: @_Z3foo5Empty(%struct._Z5Empty* %e)
// CHECK: call {{.*}} @_ZN5Empty5checkEv(%struct._Z5Empty* %e)
  return e.check();
}

void caller(Empty &e) {
// CHECK: @_Z6callerR5Empty(%struct._Z5Empty* dereferenceable({{[0-9]+}}) %e)
// CHECK: call {{.*}} @_ZN5EmptyC1ERKS_(%struct._Z5Empty* [[NEWTMP:%.*]], %struct._Z5Empty*
// CHECK: call {{.*}} @_Z3foo5Empty(%struct._Z5Empty* [[NEWTMP]])
  foo(e);
}
