// RUN: %clang_cc1 -std=c++14 %s -triple=x86_64-linux -emit-llvm -o - | FileCheck %s

struct f {
  void operator()() const {}
};

template <typename T> auto vtemplate = f{};

int main() { vtemplate<int>(); }

// CHECK: @_Z9vtemplateIiE = linkonce_odr global %struct._Z1f undef, comdat

// CHECK: define i32 @main()
// CHECK: call void @_ZNK1fclEv(%struct._Z1f* @_Z9vtemplateIiE)
