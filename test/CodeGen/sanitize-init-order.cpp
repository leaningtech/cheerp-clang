// RUN: %clang_cc1 -fsanitize=address -emit-llvm -o - %s | FileCheck %s

// Test blacklist functionality.
// RUN: echo "src:%s=init" > %t-file.blacklist
// RUN: echo "type:PODWithCtorAndDtor=init" > %t-type.blacklist
// RUN: echo "type:NS::PODWithCtor=init" >> %t-type.blacklist
// RUN: %clang_cc1 -fsanitize=address -fsanitize-blacklist=%t-file.blacklist -emit-llvm -o - %s | FileCheck %s --check-prefix=BLACKLIST
// RUN: %clang_cc1 -fsanitize=address -fsanitize-blacklist=%t-type.blacklist -emit-llvm -o - %s | FileCheck %s --check-prefix=BLACKLIST
// REQUIRES: shell

struct PODStruct {
  int x;
};
PODStruct s1;

struct PODWithDtor {
  ~PODWithDtor() { }
  int x;
};
PODWithDtor s2;

struct PODWithCtorAndDtor {
  PODWithCtorAndDtor() { }
  ~PODWithCtorAndDtor() { }
  int x;
};
PODWithCtorAndDtor s3;

namespace NS {
class PODWithCtor {
public:
  PODWithCtor() {}
};

const volatile PODWithCtor array[5][5];
}

// Check that ASan init-order checking ignores structs with trivial default
// constructor.
// CHECK: !llvm.asan.globals = !{![[GLOB_1:[0-9]+]], ![[GLOB_2:[0-9]+]], ![[GLOB_3:[0-9]]], ![[GLOB_4:[0-9]]]}
// CHECK: ![[GLOB_1]] = !{%struct._Z9PODStruct* {{.*}}, i1 false, i1 false}
// CHECK: ![[GLOB_2]] = !{%struct._Z11PODWithDtor* {{.*}}, i1 false, i1 false}
// CHECK: ![[GLOB_3]] = !{%struct._Z18PODWithCtorAndDtor* {{.*}}, i1 true, i1 false}
// CHECK: ![[GLOB_4]] = !{{{.*}}class._ZN2NS11PODWithCtor{{.*}}, i1 true, i1 false}

// BLACKLIST: !llvm.asan.globals = !{![[GLOB_1:[0-9]+]], ![[GLOB_2:[0-9]+]], ![[GLOB_3:[0-9]]], ![[GLOB_4:[0-9]]]}
// BLACKLIST: ![[GLOB_1]] = !{%struct._Z9PODStruct* {{.*}}, i1 false, i1 false}
// BLACKLIST: ![[GLOB_2]] = !{%struct._Z11PODWithDtor* {{.*}}, i1 false, i1 false}
// BLACKLIST: ![[GLOB_3]] = !{%struct._Z18PODWithCtorAndDtor* {{.*}}, i1 false, i1 false}
// BLACKLIST: ![[GLOB_4]] = !{{{.*}}class._ZN2NS11PODWithCtor{{.*}}, i1 false, i1 false}
