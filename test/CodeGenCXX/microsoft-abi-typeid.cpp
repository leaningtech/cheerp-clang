// RUN: %clang_cc1 -emit-llvm -O1 -o - -triple=i386-pc-win32 %s | FileCheck %s

struct type_info;
namespace std { using ::type_info; }

struct V { virtual void f(); };
struct A : virtual V { A(); };

extern A a;
extern V v;
extern int b;
A* fn();

const std::type_info* test0_typeid() { return &typeid(int); }
// CHECK-LABEL: define %"struct.\01?type_info@@"* @"\01?test0_typeid@@YAPBUtype_info@@XZ"()
// CHECK:   ret %"struct.\01?type_info@@"* bitcast (%rtti.TypeDescriptor2* @"\01??_R0H@8" to %"struct.\01?type_info@@"*)

const std::type_info* test1_typeid() { return &typeid(A); }
// CHECK-LABEL: define %"struct.\01?type_info@@"* @"\01?test1_typeid@@YAPBUtype_info@@XZ"()
// CHECK:   ret %"struct.\01?type_info@@"* bitcast (%rtti.TypeDescriptor7* @"\01??_R0?AUA@@@8" to %"struct.\01?type_info@@"*)

const std::type_info* test2_typeid() { return &typeid(&a); }
// CHECK-LABEL: define %"struct.\01?type_info@@"* @"\01?test2_typeid@@YAPBUtype_info@@XZ"()
// CHECK:   ret %"struct.\01?type_info@@"* bitcast (%rtti.TypeDescriptor7* @"\01??_R0PAUA@@@8" to %"struct.\01?type_info@@"*)

const std::type_info* test3_typeid() { return &typeid(*fn()); }
// CHECK-LABEL: define %"struct.\01?type_info@@"* @"\01?test3_typeid@@YAPBUtype_info@@XZ"()
// CHECK:        [[CALL:%.*]] = tail call %"struct.\01?A@@"* @"\01?fn@@YAPAUA@@XZ"()
// CHECK-NEXT:   [[CMP:%.*]] = icmp eq %"struct.\01?A@@"* [[CALL]], null
// CHECK-NEXT:   br i1 [[CMP]]
// CHECK:        tail call i8* @__RTtypeid(i8* null)
// CHECK-NEXT:   unreachable
// CHECK:        [[THIS:%.*]] = bitcast %"struct.\01?A@@"* [[CALL]] to i8*
// CHECK-NEXT:   [[VBTBLP:%.*]] = getelementptr inbounds %"struct.\01?A@@"* [[CALL]], i32 0, i32 0
// CHECK-NEXT:   [[VBTBL:%.*]] = load i32** [[VBTBLP]], align 4
// CHECK-NEXT:   [[VBSLOT:%.*]] = getelementptr inbounds i32* [[VBTBL]], i32 1
// CHECK-NEXT:   [[VBASE_OFFS:%.*]] = load i32* [[VBSLOT]], align 4
// CHECK-NEXT:   [[ADJ:%.*]] = getelementptr inbounds i8* [[THIS]], i32 [[VBASE_OFFS]]
// CHECK-NEXT:   [[RT:%.*]] = tail call i8* @__RTtypeid(i8* [[ADJ]])
// CHECK-NEXT:   [[RET:%.*]] = bitcast i8* [[RT]] to %"struct.\01?type_info@@"*
// CHECK-NEXT:   ret %"struct.\01?type_info@@"* [[RET]]

const std::type_info* test4_typeid() { return &typeid(b); }
// CHECK: define %"struct.\01?type_info@@"* @"\01?test4_typeid@@YAPBUtype_info@@XZ"()
// CHECK:   ret %"struct.\01?type_info@@"* bitcast (%rtti.TypeDescriptor2* @"\01??_R0H@8" to %"struct.\01?type_info@@"*)

const std::type_info* test5_typeid() { return &typeid(v); }
// CHECK: define %"struct.\01?type_info@@"* @"\01?test5_typeid@@YAPBUtype_info@@XZ"()
// CHECK:        [[RT:%.*]] = tail call i8* @__RTtypeid(i8* bitcast (%"struct.\01?V@@"* @"\01?v@@3UV@@A" to i8*))
// CHECK-NEXT:   [[RET:%.*]] = bitcast i8* [[RT]] to %"struct.\01?type_info@@"*
// CHECK-NEXT:   ret %"struct.\01?type_info@@"* [[RET]]
