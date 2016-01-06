// RUN: %clang_cc1 -triple i686-windows-msvc -fms-compatibility   -emit-llvm -std=c++1y -O0 -o - %s -DMSABI | FileCheck --check-prefix=MSC --check-prefix=M32 %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-compatibility -emit-llvm -std=c++1y -O0 -o - %s -DMSABI | FileCheck --check-prefix=MSC --check-prefix=M64 %s
// RUN: %clang_cc1 -triple i686-windows-gnu                       -emit-llvm -std=c++1y -O0 -o - %s         | FileCheck --check-prefix=GNU --check-prefix=G32 %s
// RUN: %clang_cc1 -triple x86_64-windows-gnu                     -emit-llvm -std=c++1y -O0 -o - %s         | FileCheck --check-prefix=GNU --check-prefix=G64 %s
// RUN: %clang_cc1 -triple i686-windows-msvc -fms-compatibility   -emit-llvm -std=c++1y -O1 -o - %s -DMSABI | FileCheck --check-prefix=MO1 %s
// RUN: %clang_cc1 -triple i686-windows-gnu                       -emit-llvm -std=c++1y -O1 -o - %s         | FileCheck --check-prefix=GO1 %s

// Helper structs to make templates more expressive.
struct ImplicitInst_Imported {};
struct ExplicitDecl_Imported {};
struct ExplicitInst_Imported {};
struct ExplicitSpec_Imported {};
struct ExplicitSpec_Def_Imported {};
struct ExplicitSpec_InlineDef_Imported {};
struct ExplicitSpec_NotImported {};

#define JOIN2(x, y) x##y
#define JOIN(x, y) JOIN2(x, y)
#define UNIQ(name) JOIN(name, __LINE__)
#define USE(func) void UNIQ(use)() { func(); }
#define USEMV(cls, var) int UNIQ(use)() { return ref(cls::var); }
#define USEMF(cls, fun) template<> void useMemFun<__LINE__, cls>() { cls().fun(); }
#define USESPECIALS(cls) void UNIQ(use)() { useSpecials<cls>(); }

template<typename T>
T ref(T const& v) { return v; }

template<int Line, typename T>
void useMemFun();

template<typename T>
void useSpecials() {
  T v; // Default constructor

  T c1(static_cast<const T&>(v)); // Copy constructor
  T c2 = static_cast<const T&>(v); // Copy constructor
  T c3;
  c3 = static_cast<const T&>(v); // Copy assignment

  T m1(static_cast<T&&>(v)); // Move constructor
  T m2 = static_cast<T&&>(v); // Move constructor
  T m3;
  m3 = static_cast<T&&>(v); // Move assignment
}

// Used to force non-trivial special members.
struct ForceNonTrivial {
  ForceNonTrivial();
  ~ForceNonTrivial();
  ForceNonTrivial(const ForceNonTrivial&);
  ForceNonTrivial& operator=(const ForceNonTrivial&);
  ForceNonTrivial(ForceNonTrivial&&);
  ForceNonTrivial& operator=(ForceNonTrivial&&);
};



//===----------------------------------------------------------------------===//
// Class members
//===----------------------------------------------------------------------===//

// Import individual members of a class.
struct ImportMembers {
  struct Nested;

  // M32-DAG: define              x86_thiscallcc void @"\01?normalDef@ImportMembers@@QAEXXZ"(%"struct.\01?ImportMembers@@"* %this)
  // M64-DAG: define                             void @"\01?normalDef@ImportMembers@@QEAAXXZ"(%"struct.\01?ImportMembers@@"* %this)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalDecl@ImportMembers@@QAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalDecl@ImportMembers@@QEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalInclass@ImportMembers@@QAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalInclass@ImportMembers@@QEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalInlineDef@ImportMembers@@QAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalInlineDef@ImportMembers@@QEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalInlineDecl@ImportMembers@@QAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalInlineDecl@ImportMembers@@QEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // G32-DAG: define              x86_thiscallcc void @_ZN13ImportMembers9normalDefEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define                             void @_ZN13ImportMembers9normalDefEv(%struct._Z13ImportMembers* %this)
  // G32-DAG: declare dllimport   x86_thiscallcc void @_ZN13ImportMembers10normalDeclEv(%struct._Z13ImportMembers*)
  // G64-DAG: declare dllimport                  void @_ZN13ImportMembers10normalDeclEv(%struct._Z13ImportMembers*)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers13normalInclassEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers13normalInclassEv(%struct._Z13ImportMembers* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers15normalInlineDefEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers15normalInlineDefEv(%struct._Z13ImportMembers* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers16normalInlineDeclEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers16normalInlineDeclEv(%struct._Z13ImportMembers* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInclass@ImportMembers@@QAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInlineDef@ImportMembers@@QAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInlineDecl@ImportMembers@@QAEXXZ"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers13normalInclassEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers15normalInlineDefEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers16normalInlineDeclEv(
  __declspec(dllimport)                void normalDef(); // dllimport ignored
  __declspec(dllimport)                void normalDecl();
  __declspec(dllimport)                void normalInclass() {}
  __declspec(dllimport)                void normalInlineDef();
  __declspec(dllimport)         inline void normalInlineDecl();

  // M32-DAG: define              x86_thiscallcc void @"\01?virtualDef@ImportMembers@@UAEXXZ"(%"struct.\01?ImportMembers@@"* %this)
  // M64-DAG: define                             void @"\01?virtualDef@ImportMembers@@UEAAXXZ"(%"struct.\01?ImportMembers@@"* %this)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualDecl@ImportMembers@@UAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualDecl@ImportMembers@@UEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualInclass@ImportMembers@@UAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualInclass@ImportMembers@@UEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualInlineDef@ImportMembers@@UAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualInlineDef@ImportMembers@@UEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualInlineDecl@ImportMembers@@UAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualInlineDecl@ImportMembers@@UEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // G32-DAG: define              x86_thiscallcc void @_ZN13ImportMembers10virtualDefEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define                             void @_ZN13ImportMembers10virtualDefEv(%struct._Z13ImportMembers* %this)
  // G32-DAG: declare dllimport   x86_thiscallcc void @_ZN13ImportMembers11virtualDeclEv(%struct._Z13ImportMembers*)
  // G64-DAG: declare dllimport                  void @_ZN13ImportMembers11virtualDeclEv(%struct._Z13ImportMembers*)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers14virtualInclassEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers14virtualInclassEv(%struct._Z13ImportMembers* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers16virtualInlineDefEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers16virtualInlineDefEv(%struct._Z13ImportMembers* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers17virtualInlineDeclEv(%struct._Z13ImportMembers* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers17virtualInlineDeclEv(%struct._Z13ImportMembers* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?virtualInclass@ImportMembers@@UAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?virtualInlineDef@ImportMembers@@UAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?virtualInlineDecl@ImportMembers@@UAEXXZ"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers14virtualInclassEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers16virtualInlineDefEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers17virtualInlineDeclEv(
  __declspec(dllimport) virtual        void virtualDef(); // dllimport ignored
  __declspec(dllimport) virtual        void virtualDecl();
  __declspec(dllimport) virtual        void virtualInclass() {}
  __declspec(dllimport) virtual        void virtualInlineDef();
  __declspec(dllimport) virtual inline void virtualInlineDecl();

  // MSC-DAG: define                           void @"\01?staticDef@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticDecl@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInclass@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDef@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@ImportMembers@@SAXXZ"()
  // GNU-DAG: define                           void @_ZN13ImportMembers9staticDefEv()
  // GNU-DAG: declare dllimport                void @_ZN13ImportMembers10staticDeclEv()
  // GNU-DAG: define linkonce_odr              void @_ZN13ImportMembers13staticInclassEv()
  // GNU-DAG: define linkonce_odr              void @_ZN13ImportMembers15staticInlineDefEv()
  // GNU-DAG: define linkonce_odr              void @_ZN13ImportMembers16staticInlineDeclEv()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInclass@ImportMembers@@SAXXZ"()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInlineDef@ImportMembers@@SAXXZ"()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInlineDecl@ImportMembers@@SAXXZ"()
  // GO1-DAG: define linkonce_odr              void @_ZN13ImportMembers13staticInclassEv()
  // GO1-DAG: define linkonce_odr              void @_ZN13ImportMembers15staticInlineDefEv()
  // GO1-DAG: define linkonce_odr              void @_ZN13ImportMembers16staticInlineDeclEv()
  __declspec(dllimport) static         void staticDef(); // dllimport ignored
  __declspec(dllimport) static         void staticDecl();
  __declspec(dllimport) static         void staticInclass() {}
  __declspec(dllimport) static         void staticInlineDef();
  __declspec(dllimport) static  inline void staticInlineDecl();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?protectedNormalDecl@ImportMembers@@IAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                void @"\01?protectedNormalDecl@ImportMembers@@IEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportMembers19protectedNormalDeclEv(%struct._Z13ImportMembers*)
  // G64-DAG: declare dllimport                void @_ZN13ImportMembers19protectedNormalDeclEv(%struct._Z13ImportMembers*)
  // MSC-DAG: declare dllimport                void @"\01?protectedStaticDecl@ImportMembers@@KAXXZ"()
  // GNU-DAG: declare dllimport                void @_ZN13ImportMembers19protectedStaticDeclEv()
protected:
  __declspec(dllimport)                void protectedNormalDecl();
  __declspec(dllimport) static         void protectedStaticDecl();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?privateNormalDecl@ImportMembers@@AAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare dllimport                void @"\01?privateNormalDecl@ImportMembers@@AEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportMembers17privateNormalDeclEv(%struct._Z13ImportMembers*)
  // G64-DAG: declare dllimport                void @_ZN13ImportMembers17privateNormalDeclEv(%struct._Z13ImportMembers*)
  // MSC-DAG: declare dllimport                void @"\01?privateStaticDecl@ImportMembers@@CAXXZ"()
  // GNU-DAG: declare dllimport                void @_ZN13ImportMembers17privateStaticDeclEv()
private:
  __declspec(dllimport)                void privateNormalDecl();
  __declspec(dllimport) static         void privateStaticDecl();

  // M32-DAG: declare           x86_thiscallcc void @"\01?ignored@ImportMembers@@QAEXXZ"(%"struct.\01?ImportMembers@@"*)
  // M64-DAG: declare                          void @"\01?ignored@ImportMembers@@QEAAXXZ"(%"struct.\01?ImportMembers@@"*)
  // G32-DAG: declare           x86_thiscallcc void @_ZN13ImportMembers7ignoredEv(%struct._Z13ImportMembers*)
  // G64-DAG: declare                          void @_ZN13ImportMembers7ignoredEv(%struct._Z13ImportMembers*)
public:
  void ignored();

  // MSC-DAG: @"\01?StaticField@ImportMembers@@2HA"               = external dllimport global i32
  // MSC-DAG: @"\01?StaticConstField@ImportMembers@@2HB"          = external dllimport constant i32
  // MSC-DAG: @"\01?StaticConstFieldEqualInit@ImportMembers@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldBraceInit@ImportMembers@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?ConstexprField@ImportMembers@@2HB"            = available_externally dllimport constant i32 1, align 4
  // GNU-DAG: @_ZN13ImportMembers11StaticFieldE                   = external dllimport global i32
  // GNU-DAG: @_ZN13ImportMembers16StaticConstFieldE              = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportMembers25StaticConstFieldEqualInitE     = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportMembers25StaticConstFieldBraceInitE     = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportMembers14ConstexprFieldE                = external dllimport constant i32
  __declspec(dllimport) static         int  StaticField;
  __declspec(dllimport) static  const  int  StaticConstField;
  __declspec(dllimport) static  const  int  StaticConstFieldEqualInit = 1;
  __declspec(dllimport) static  const  int  StaticConstFieldBraceInit{1};
  __declspec(dllimport) constexpr static int ConstexprField = 1;

  template<int Line, typename T> friend void useMemFun();
};

       void ImportMembers::normalDef() {} // dllimport ignored
inline void ImportMembers::normalInlineDef() {}
       void ImportMembers::normalInlineDecl() {}
       void ImportMembers::virtualDef() {} // dllimport ignored
inline void ImportMembers::virtualInlineDef() {}
       void ImportMembers::virtualInlineDecl() {}
       void ImportMembers::staticDef() {} // dllimport ignored
inline void ImportMembers::staticInlineDef() {}
       void ImportMembers::staticInlineDecl() {}

USEMF(ImportMembers, normalDef)
USEMF(ImportMembers, normalDecl)
USEMF(ImportMembers, normalInclass)
USEMF(ImportMembers, normalInlineDef)
USEMF(ImportMembers, normalInlineDecl)
USEMF(ImportMembers, virtualDef)
USEMF(ImportMembers, virtualDecl)
USEMF(ImportMembers, virtualInclass)
USEMF(ImportMembers, virtualInlineDef)
USEMF(ImportMembers, virtualInlineDecl)
USEMF(ImportMembers, staticDef)
USEMF(ImportMembers, staticDecl)
USEMF(ImportMembers, staticInclass)
USEMF(ImportMembers, staticInlineDef)
USEMF(ImportMembers, staticInlineDecl)
USEMF(ImportMembers, protectedNormalDecl)
USEMF(ImportMembers, protectedStaticDecl)
USEMF(ImportMembers, privateNormalDecl)
USEMF(ImportMembers, privateStaticDecl)
USEMF(ImportMembers, ignored)

USEMV(ImportMembers, StaticField)
USEMV(ImportMembers, StaticConstField)
USEMV(ImportMembers, StaticConstFieldEqualInit)
USEMV(ImportMembers, StaticConstFieldBraceInit)
USEMV(ImportMembers, ConstexprField)


// Import individual members of a nested class.
struct ImportMembers::Nested {
  // M32-DAG: define              x86_thiscallcc void @"\01?normalDef@Nested@ImportMembers@@QAEXXZ"(%"struct.\01?Nested@ImportMembers@@"* %this)
  // M64-DAG: define                             void @"\01?normalDef@Nested@ImportMembers@@QEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"* %this)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalDecl@Nested@ImportMembers@@QAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalDecl@Nested@ImportMembers@@QEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalInclass@Nested@ImportMembers@@QAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalInclass@Nested@ImportMembers@@QEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalInlineDef@Nested@ImportMembers@@QAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalInlineDef@Nested@ImportMembers@@QEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?normalInlineDecl@Nested@ImportMembers@@QAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?normalInlineDecl@Nested@ImportMembers@@QEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // G32-DAG: define              x86_thiscallcc void @_ZN13ImportMembers6Nested9normalDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define                             void @_ZN13ImportMembers6Nested9normalDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G32-DAG: declare dllimport   x86_thiscallcc void @_ZN13ImportMembers6Nested10normalDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // G64-DAG: declare dllimport                  void @_ZN13ImportMembers6Nested10normalDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested13normalInclassEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers6Nested13normalInclassEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested15normalInlineDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers6Nested15normalInlineDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested16normalInlineDeclEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers6Nested16normalInlineDeclEv(%struct._ZN13ImportMembers6NestedE* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInclass@Nested@ImportMembers@@QAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInlineDef@Nested@ImportMembers@@QAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?normalInlineDecl@Nested@ImportMembers@@QAEXXZ"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested13normalInclassEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested15normalInlineDefEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested16normalInlineDeclEv(
  __declspec(dllimport)                void normalDef(); // dllimport ignored
  __declspec(dllimport)                void normalDecl();
  __declspec(dllimport)                void normalInclass() {}
  __declspec(dllimport)                void normalInlineDef();
  __declspec(dllimport)         inline void normalInlineDecl();

  // M32-DAG: define              x86_thiscallcc void @"\01?virtualDef@Nested@ImportMembers@@UAEXXZ"(%"struct.\01?Nested@ImportMembers@@"* %this)
  // M64-DAG: define                             void @"\01?virtualDef@Nested@ImportMembers@@UEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"* %this)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualDecl@Nested@ImportMembers@@UAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualDecl@Nested@ImportMembers@@UEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualInclass@Nested@ImportMembers@@UAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualInclass@Nested@ImportMembers@@UEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualInlineDef@Nested@ImportMembers@@UAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualInlineDef@Nested@ImportMembers@@UEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01?virtualInlineDecl@Nested@ImportMembers@@UAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                  void @"\01?virtualInlineDecl@Nested@ImportMembers@@UEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // G32-DAG: define              x86_thiscallcc void @_ZN13ImportMembers6Nested10virtualDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define                             void @_ZN13ImportMembers6Nested10virtualDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G32-DAG: declare dllimport   x86_thiscallcc void @_ZN13ImportMembers6Nested11virtualDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // G64-DAG: declare dllimport                  void @_ZN13ImportMembers6Nested11virtualDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested14virtualInclassEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers6Nested14virtualInclassEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested16virtualInlineDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers6Nested16virtualInlineDefEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN13ImportMembers6Nested17virtualInlineDeclEv(%struct._ZN13ImportMembers6NestedE* %this)
  // G64-DAG: define linkonce_odr                void @_ZN13ImportMembers6Nested17virtualInlineDeclEv(%struct._ZN13ImportMembers6NestedE* %this)

  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?virtualInclass@Nested@ImportMembers@@UAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?virtualInlineDef@Nested@ImportMembers@@UAEXXZ"(
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01?virtualInlineDecl@Nested@ImportMembers@@UAEXXZ"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc                   void @_ZN13ImportMembers6Nested14virtualInclassEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc                   void @_ZN13ImportMembers6Nested16virtualInlineDefEv(
  // GO1-DAG: define linkonce_odr x86_thiscallcc                   void @_ZN13ImportMembers6Nested17virtualInlineDeclEv(
  __declspec(dllimport) virtual        void virtualDef(); // dllimport ignored
  __declspec(dllimport) virtual        void virtualDecl();
  __declspec(dllimport) virtual        void virtualInclass() {}
  __declspec(dllimport) virtual        void virtualInlineDef();
  __declspec(dllimport) virtual inline void virtualInlineDecl();

  // MSC-DAG: define                           void @"\01?staticDef@Nested@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticDecl@Nested@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInclass@Nested@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDef@Nested@ImportMembers@@SAXXZ"()
  // MSC-DAG: declare dllimport                void @"\01?staticInlineDecl@Nested@ImportMembers@@SAXXZ"()
  // GNU-DAG: define                           void @_ZN13ImportMembers6Nested9staticDefEv()
  // GNU-DAG: declare dllimport                void @_ZN13ImportMembers6Nested10staticDeclEv()
  // GNU-DAG: define linkonce_odr              void @_ZN13ImportMembers6Nested13staticInclassEv()
  // GNU-DAG: define linkonce_odr              void @_ZN13ImportMembers6Nested15staticInlineDefEv()
  // GNU-DAG: define linkonce_odr              void @_ZN13ImportMembers6Nested16staticInlineDeclEv()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInclass@Nested@ImportMembers@@SAXXZ"()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInlineDef@Nested@ImportMembers@@SAXXZ"()
  // MO1-DAG: define available_externally dllimport void @"\01?staticInlineDecl@Nested@ImportMembers@@SAXXZ"()
  // GO1-DAG: define linkonce_odr              void @_ZN13ImportMembers6Nested13staticInclassEv()
  // GO1-DAG: define linkonce_odr              void @_ZN13ImportMembers6Nested15staticInlineDefEv()
  // GO1-DAG: define linkonce_odr              void @_ZN13ImportMembers6Nested16staticInlineDeclEv()
  __declspec(dllimport) static         void staticDef(); // dllimport ignored
  __declspec(dllimport) static         void staticDecl();
  __declspec(dllimport) static         void staticInclass() {}
  __declspec(dllimport) static         void staticInlineDef();
  __declspec(dllimport) static  inline void staticInlineDecl();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?protectedNormalDecl@Nested@ImportMembers@@IAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                void @"\01?protectedNormalDecl@Nested@ImportMembers@@IEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportMembers6Nested19protectedNormalDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // G64-DAG: declare dllimport                void @_ZN13ImportMembers6Nested19protectedNormalDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // MSC-DAG: declare dllimport                void @"\01?protectedStaticDecl@Nested@ImportMembers@@KAXXZ"()
  // GNU-DAG: declare dllimport                void @_ZN13ImportMembers6Nested19protectedStaticDeclEv()
protected:
  __declspec(dllimport)                void protectedNormalDecl();
  __declspec(dllimport) static         void protectedStaticDecl();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01?privateNormalDecl@Nested@ImportMembers@@AAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare dllimport                void @"\01?privateNormalDecl@Nested@ImportMembers@@AEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // G32-DAG: declare dllimport x86_thiscallcc void @_ZN13ImportMembers6Nested17privateNormalDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // G64-DAG: declare dllimport                void @_ZN13ImportMembers6Nested17privateNormalDeclEv(%struct._ZN13ImportMembers6NestedE*)
  // MSC-DAG: declare dllimport                void @"\01?privateStaticDecl@Nested@ImportMembers@@CAXXZ"()
  // GNU-DAG: declare dllimport                void @_ZN13ImportMembers6Nested17privateStaticDeclEv()
private:
  __declspec(dllimport)                void privateNormalDecl();
  __declspec(dllimport) static         void privateStaticDecl();

  // M32-DAG: declare           x86_thiscallcc void @"\01?ignored@Nested@ImportMembers@@QAEXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // M64-DAG: declare                          void @"\01?ignored@Nested@ImportMembers@@QEAAXXZ"(%"struct.\01?Nested@ImportMembers@@"*)
  // G32-DAG: declare           x86_thiscallcc void @_ZN13ImportMembers6Nested7ignoredEv(%struct._ZN13ImportMembers6NestedE*)
  // G64-DAG: declare                          void @_ZN13ImportMembers6Nested7ignoredEv(%struct._ZN13ImportMembers6NestedE*)
public:
  void ignored();

  // MSC-DAG: @"\01?StaticField@Nested@ImportMembers@@2HA"               = external dllimport global i32
  // MSC-DAG: @"\01?StaticConstField@Nested@ImportMembers@@2HB"          = external dllimport constant i32
  // MSC-DAG: @"\01?StaticConstFieldEqualInit@Nested@ImportMembers@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldBraceInit@Nested@ImportMembers@@2HB" = available_externally dllimport constant i32 1, align 4
  // MSC-DAG: @"\01?ConstexprField@Nested@ImportMembers@@2HB"            = available_externally dllimport constant i32 1, align 4
  // GNU-DAG: @_ZN13ImportMembers6Nested11StaticFieldE                   = external dllimport global i32
  // GNU-DAG: @_ZN13ImportMembers6Nested16StaticConstFieldE              = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportMembers6Nested25StaticConstFieldEqualInitE     = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportMembers6Nested25StaticConstFieldBraceInitE     = external dllimport constant i32
  // GNU-DAG: @_ZN13ImportMembers6Nested14ConstexprFieldE                = external dllimport constant i32
  __declspec(dllimport) static         int  StaticField;
  __declspec(dllimport) static  const  int  StaticConstField;
  __declspec(dllimport) static  const  int  StaticConstFieldEqualInit = 1;
  __declspec(dllimport) static  const  int  StaticConstFieldBraceInit{1};
  __declspec(dllimport) constexpr static int ConstexprField = 1;

  template<int Line, typename T> friend void useMemFun();
};

       void ImportMembers::Nested::normalDef() {} // dllimport ignored
inline void ImportMembers::Nested::normalInlineDef() {}
       void ImportMembers::Nested::normalInlineDecl() {}
       void ImportMembers::Nested::virtualDef() {} // dllimport ignored
inline void ImportMembers::Nested::virtualInlineDef() {}
       void ImportMembers::Nested::virtualInlineDecl() {}
       void ImportMembers::Nested::staticDef() {} // dllimport ignored
inline void ImportMembers::Nested::staticInlineDef() {}
       void ImportMembers::Nested::staticInlineDecl() {}

USEMF(ImportMembers::Nested, normalDef)
USEMF(ImportMembers::Nested, normalDecl)
USEMF(ImportMembers::Nested, normalInclass)
USEMF(ImportMembers::Nested, normalInlineDef)
USEMF(ImportMembers::Nested, normalInlineDecl)
USEMF(ImportMembers::Nested, virtualDef)
USEMF(ImportMembers::Nested, virtualDecl)
USEMF(ImportMembers::Nested, virtualInclass)
USEMF(ImportMembers::Nested, virtualInlineDef)
USEMF(ImportMembers::Nested, virtualInlineDecl)
USEMF(ImportMembers::Nested, staticDef)
USEMF(ImportMembers::Nested, staticDecl)
USEMF(ImportMembers::Nested, staticInclass)
USEMF(ImportMembers::Nested, staticInlineDef)
USEMF(ImportMembers::Nested, staticInlineDecl)
USEMF(ImportMembers::Nested, protectedNormalDecl)
USEMF(ImportMembers::Nested, protectedStaticDecl)
USEMF(ImportMembers::Nested, privateNormalDecl)
USEMF(ImportMembers::Nested, privateStaticDecl)
USEMF(ImportMembers::Nested, ignored)

USEMV(ImportMembers::Nested, StaticField)
USEMV(ImportMembers::Nested, StaticConstField)
USEMV(ImportMembers::Nested, StaticConstFieldEqualInit)
USEMV(ImportMembers::Nested, StaticConstFieldBraceInit)
USEMV(ImportMembers::Nested, ConstexprField)


// Import special member functions.
struct ImportSpecials {
  // M32-DAG: declare dllimport x86_thiscallcc %"struct.\01?ImportSpecials@@"* @"\01??0ImportSpecials@@QAE@XZ"(%"struct.\01?ImportSpecials@@"* returned)
  // M64-DAG: declare dllimport                %"struct.\01?ImportSpecials@@"* @"\01??0ImportSpecials@@QEAA@XZ"(%"struct.\01?ImportSpecials@@"* returned)
  // G32-DAG: declare dllimport x86_thiscallcc void                    @_ZN14ImportSpecialsC1Ev(%struct._Z14ImportSpecials*)
  // G64-DAG: declare dllimport                void                    @_ZN14ImportSpecialsC1Ev(%struct._Z14ImportSpecials*)
  __declspec(dllimport) ImportSpecials();

  // M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportSpecials@@QAE@XZ"(%"struct.\01?ImportSpecials@@"*)
  // M64-DAG: declare dllimport                void @"\01??1ImportSpecials@@QEAA@XZ"(%"struct.\01?ImportSpecials@@"*)
  // G32-DAG: declare dllimport x86_thiscallcc void                    @_ZN14ImportSpecialsD1Ev(%struct._Z14ImportSpecials*)
  // G64-DAG: declare dllimport                void                    @_ZN14ImportSpecialsD1Ev(%struct._Z14ImportSpecials*)
  __declspec(dllimport) ~ImportSpecials();

  // M32-DAG: declare dllimport x86_thiscallcc %"struct.\01?ImportSpecials@@"* @"\01??0ImportSpecials@@QAE@ABU0@@Z"(%"struct.\01?ImportSpecials@@"* returned, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                %"struct.\01?ImportSpecials@@"* @"\01??0ImportSpecials@@QEAA@AEBU0@@Z"(%"struct.\01?ImportSpecials@@"* returned, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: declare dllimport x86_thiscallcc void                    @_ZN14ImportSpecialsC1ERKS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: declare dllimport                void                    @_ZN14ImportSpecialsC1ERKS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportSpecials(const ImportSpecials&);

  // M32-DAG: declare dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportSpecials@@"* @"\01??4ImportSpecials@@QAEAAU0@ABU0@@Z"(%"struct.\01?ImportSpecials@@"*, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                dereferenceable({{[0-9]+}}) %"struct.\01?ImportSpecials@@"* @"\01??4ImportSpecials@@QEAAAEAU0@AEBU0@@Z"(%"struct.\01?ImportSpecials@@"*, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: declare dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z14ImportSpecials* @_ZN14ImportSpecialsaSERKS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: declare dllimport                dereferenceable({{[0-9]+}}) %struct._Z14ImportSpecials* @_ZN14ImportSpecialsaSERKS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportSpecials& operator=(const ImportSpecials&);

  // M32-DAG: declare dllimport x86_thiscallcc %"struct.\01?ImportSpecials@@"* @"\01??0ImportSpecials@@QAE@$$QAU0@@Z"(%"struct.\01?ImportSpecials@@"* returned, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                %"struct.\01?ImportSpecials@@"* @"\01??0ImportSpecials@@QEAA@$$QEAU0@@Z"(%"struct.\01?ImportSpecials@@"* returned, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: declare dllimport x86_thiscallcc void                    @_ZN14ImportSpecialsC1EOS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: declare dllimport                void                    @_ZN14ImportSpecialsC1EOS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportSpecials(ImportSpecials&&);

  // M32-DAG: declare dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportSpecials@@"* @"\01??4ImportSpecials@@QAEAAU0@$$QAU0@@Z"(%"struct.\01?ImportSpecials@@"*, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                dereferenceable({{[0-9]+}}) %"struct.\01?ImportSpecials@@"* @"\01??4ImportSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%"struct.\01?ImportSpecials@@"*, %"struct.\01?ImportSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: declare dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z14ImportSpecials* @_ZN14ImportSpecialsaSEOS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: declare dllimport                dereferenceable({{[0-9]+}}) %struct._Z14ImportSpecials* @_ZN14ImportSpecialsaSEOS_(%struct._Z14ImportSpecials*, %struct._Z14ImportSpecials* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportSpecials& operator=(ImportSpecials&&);
};
USESPECIALS(ImportSpecials)


// Export inline special member functions.
struct ImportInlineSpecials {
  // M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QAE@XZ"(%"struct.\01?ImportInlineSpecials@@"* returned)
  // M64-DAG: declare dllimport                  %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QEAA@XZ"(%"struct.\01?ImportInlineSpecials@@"* returned)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsC1Ev(%struct._Z20ImportInlineSpecials* %this)
  // G64-DAG: define linkonce_odr                void @_ZN20ImportInlineSpecialsC1Ev(%struct._Z20ImportInlineSpecials* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QAE@XZ"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsC1Ev(
  __declspec(dllimport) ImportInlineSpecials() {}

  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01??1ImportInlineSpecials@@QAE@XZ"(%"struct.\01?ImportInlineSpecials@@"*)
  // M64-DAG: declare dllimport                  void @"\01??1ImportInlineSpecials@@QEAA@XZ"(%"struct.\01?ImportInlineSpecials@@"*)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsD1Ev(%struct._Z20ImportInlineSpecials* %this)
  // G64-DAG: define linkonce_odr                void @_ZN20ImportInlineSpecialsD1Ev(%struct._Z20ImportInlineSpecials* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01??1ImportInlineSpecials@@QAE@XZ"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsD1Ev(
  __declspec(dllimport) ~ImportInlineSpecials() {}

  // M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QAE@ABU0@@Z"(%"struct.\01?ImportInlineSpecials@@"* returned, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QEAA@AEBU0@@Z"(%"struct.\01?ImportInlineSpecials@@"* returned, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsC1ERKS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                void @_ZN20ImportInlineSpecialsC1ERKS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QAE@ABU0@@Z"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsC1ERKS_(
  __declspec(dllimport) inline ImportInlineSpecials(const ImportInlineSpecials&);

  // M32-DAG: declare dllimport   x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportInlineSpecials@@"* @"\01??4ImportInlineSpecials@@QAEAAU0@ABU0@@Z"(%"struct.\01?ImportInlineSpecials@@"*, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  dereferenceable({{[0-9]+}}) %"struct.\01?ImportInlineSpecials@@"* @"\01??4ImportInlineSpecials@@QEAAAEAU0@AEBU0@@Z"(%"struct.\01?ImportInlineSpecials@@"*, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z20ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSERKS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                dereferenceable({{[0-9]+}}) %struct._Z20ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSERKS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportInlineSpecials@@"* @"\01??4ImportInlineSpecials@@QAEAAU0@ABU0@@Z"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z20ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSERKS_(
  __declspec(dllimport) ImportInlineSpecials& operator=(const ImportInlineSpecials&);

  // M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QAE@$$QAU0@@Z"(%"struct.\01?ImportInlineSpecials@@"* returned, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QEAA@$$QEAU0@@Z"(%"struct.\01?ImportInlineSpecials@@"* returned, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsC1EOS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                void @_ZN20ImportInlineSpecialsC1EOS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %"struct.\01?ImportInlineSpecials@@"* @"\01??0ImportInlineSpecials@@QAE@$$QAU0@@Z"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN20ImportInlineSpecialsC1EOS_(
  __declspec(dllimport) ImportInlineSpecials(ImportInlineSpecials&&) {}

  // M32-DAG: declare dllimport   x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportInlineSpecials@@"* @"\01??4ImportInlineSpecials@@QAEAAU0@$$QAU0@@Z"(%"struct.\01?ImportInlineSpecials@@"*, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  dereferenceable({{[0-9]+}}) %"struct.\01?ImportInlineSpecials@@"* @"\01??4ImportInlineSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%"struct.\01?ImportInlineSpecials@@"*, %"struct.\01?ImportInlineSpecials@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z20ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSEOS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                dereferenceable({{[0-9]+}}) %struct._Z20ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSEOS_(%struct._Z20ImportInlineSpecials* %this, %struct._Z20ImportInlineSpecials* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportInlineSpecials@@"* @"\01??4ImportInlineSpecials@@QAEAAU0@$$QAU0@@Z"(
  // GO1-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z20ImportInlineSpecials* @_ZN20ImportInlineSpecialsaSEOS_(
  __declspec(dllimport) ImportInlineSpecials& operator=(ImportInlineSpecials&&) { return *this; }
};
ImportInlineSpecials::ImportInlineSpecials(const ImportInlineSpecials&) {}
inline ImportInlineSpecials& ImportInlineSpecials::operator=(const ImportInlineSpecials&) { return *this; }
USESPECIALS(ImportInlineSpecials)


// Import defaulted member functions.
struct ImportDefaulted {
  // M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QAE@XZ"(%"struct.\01?ImportDefaulted@@"* returned)
  // M64-DAG: declare dllimport                  %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QEAA@XZ"(%"struct.\01?ImportDefaulted@@"* returned)
  // G32-DAG: define linkonce_odr x86_thiscallcc void                     @_ZN15ImportDefaultedC1Ev(%struct._Z15ImportDefaulted* %this)
  // G64-DAG: define linkonce_odr                void                     @_ZN15ImportDefaultedC1Ev(%struct._Z15ImportDefaulted* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QAE@XZ"(%"struct.\01?ImportDefaulted@@"* returned %this)
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN15ImportDefaultedC1Ev(%struct._Z15ImportDefaulted* %this)
  __declspec(dllimport) ImportDefaulted() = default;

  // M32-DAG: declare dllimport   x86_thiscallcc void @"\01??1ImportDefaulted@@QAE@XZ"(%"struct.\01?ImportDefaulted@@"*)
  // M64-DAG: declare dllimport                  void @"\01??1ImportDefaulted@@QEAA@XZ"(%"struct.\01?ImportDefaulted@@"*)
  // G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN15ImportDefaultedD1Ev(%struct._Z15ImportDefaulted* %this)
  // G64-DAG: define linkonce_odr                void @_ZN15ImportDefaultedD1Ev(%struct._Z15ImportDefaulted* %this)
  // MO1-DAG: define available_externally dllimport x86_thiscallcc void @"\01??1ImportDefaulted@@QAE@XZ"(%"struct.\01?ImportDefaulted@@"* %this)
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN15ImportDefaultedD1Ev(%struct._Z15ImportDefaulted* %this)
  __declspec(dllimport) ~ImportDefaulted() = default;

  // M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QAE@ABU0@@Z"(%"struct.\01?ImportDefaulted@@"* returned, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QEAA@AEBU0@@Z"(%"struct.\01?ImportDefaulted@@"* returned, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc void                     @_ZN15ImportDefaultedC1ERKS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                void                     @_ZN15ImportDefaultedC1ERKS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QAE@ABU0@@Z"(%"struct.\01?ImportDefaulted@@"* returned %this, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN15ImportDefaultedC1ERKS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportDefaulted(const ImportDefaulted&) = default;

  // M32-DAG: declare dllimport   x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaulted@@"* @"\01??4ImportDefaulted@@QAEAAU0@ABU0@@Z"(%"struct.\01?ImportDefaulted@@"*, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaulted@@"* @"\01??4ImportDefaulted@@QEAAAEAU0@AEBU0@@Z"(%"struct.\01?ImportDefaulted@@"*, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z15ImportDefaulted* @_ZN15ImportDefaultedaSERKS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                dereferenceable({{[0-9]+}}) %struct._Z15ImportDefaulted* @_ZN15ImportDefaultedaSERKS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaulted@@"* @"\01??4ImportDefaulted@@QAEAAU0@ABU0@@Z"(%"struct.\01?ImportDefaulted@@"* %this, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // GO1-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z15ImportDefaulted* @_ZN15ImportDefaultedaSERKS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportDefaulted& operator=(const ImportDefaulted&) = default;

  // M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QAE@$$QAU0@@Z"(%"struct.\01?ImportDefaulted@@"* returned, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QEAA@$$QEAU0@@Z"(%"struct.\01?ImportDefaulted@@"* returned, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc void                     @_ZN15ImportDefaultedC1EOS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                void                     @_ZN15ImportDefaultedC1EOS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc %"struct.\01?ImportDefaulted@@"* @"\01??0ImportDefaulted@@QAE@$$QAU0@@Z"(%"struct.\01?ImportDefaulted@@"* returned %this, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // GO1-DAG: define linkonce_odr x86_thiscallcc void @_ZN15ImportDefaultedC1EOS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportDefaulted(ImportDefaulted&&) = default;

  // M32-DAG: declare dllimport   x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaulted@@"* @"\01??4ImportDefaulted@@QAEAAU0@$$QAU0@@Z"(%"struct.\01?ImportDefaulted@@"*, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // M64-DAG: declare dllimport                  dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaulted@@"* @"\01??4ImportDefaulted@@QEAAAEAU0@$$QEAU0@@Z"(%"struct.\01?ImportDefaulted@@"*, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // G32-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z15ImportDefaulted* @_ZN15ImportDefaultedaSEOS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // G64-DAG: define linkonce_odr                dereferenceable({{[0-9]+}}) %struct._Z15ImportDefaulted* @_ZN15ImportDefaultedaSEOS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  // MO1-DAG: define available_externally dllimport x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaulted@@"* @"\01??4ImportDefaulted@@QAEAAU0@$$QAU0@@Z"(%"struct.\01?ImportDefaulted@@"* %this, %"struct.\01?ImportDefaulted@@"* dereferenceable({{[0-9]+}}))
  // GO1-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z15ImportDefaulted* @_ZN15ImportDefaultedaSEOS_(%struct._Z15ImportDefaulted* %this, %struct._Z15ImportDefaulted* dereferenceable({{[0-9]+}}))
  __declspec(dllimport) ImportDefaulted& operator=(ImportDefaulted&&) = default;

  ForceNonTrivial v; // ensure special members are non-trivial
};
USESPECIALS(ImportDefaulted)


// Import defaulted member function definitions.
struct ImportDefaultedDefs {
  __declspec(dllimport) inline ImportDefaultedDefs();
  __declspec(dllimport) inline ~ImportDefaultedDefs();

  __declspec(dllimport) ImportDefaultedDefs(const ImportDefaultedDefs&);
  __declspec(dllimport) ImportDefaultedDefs& operator=(const ImportDefaultedDefs&);

  __declspec(dllimport) ImportDefaultedDefs(ImportDefaultedDefs&&);
  __declspec(dllimport) ImportDefaultedDefs& operator=(ImportDefaultedDefs&&);
};

#ifdef MSABI
// For MinGW, the function will not be dllimport, and we cannot add the attribute now.
// M32-DAG: declare dllimport x86_thiscallcc %"struct.\01?ImportDefaultedDefs@@"* @"\01??0ImportDefaultedDefs@@QAE@XZ"(%"struct.\01?ImportDefaultedDefs@@"* returned)
// M64-DAG: declare dllimport                %"struct.\01?ImportDefaultedDefs@@"* @"\01??0ImportDefaultedDefs@@QEAA@XZ"(%"struct.\01?ImportDefaultedDefs@@"* returned)
__declspec(dllimport) ImportDefaultedDefs::ImportDefaultedDefs() = default;
#endif

#ifdef MSABI
// For MinGW, the function will not be dllimport, and we cannot add the attribute now.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??1ImportDefaultedDefs@@QAE@XZ"(%"struct.\01?ImportDefaultedDefs@@"*)
// M64-DAG: declare dllimport                void @"\01??1ImportDefaultedDefs@@QEAA@XZ"(%"struct.\01?ImportDefaultedDefs@@"*)
__declspec(dllimport) ImportDefaultedDefs::~ImportDefaultedDefs() = default;
#endif

// M32-DAG: declare dllimport   x86_thiscallcc %"struct.\01?ImportDefaultedDefs@@"* @"\01??0ImportDefaultedDefs@@QAE@ABU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"* returned, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// M64-DAG: declare dllimport                  %"struct.\01?ImportDefaultedDefs@@"* @"\01??0ImportDefaultedDefs@@QEAA@AEBU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"* returned, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN19ImportDefaultedDefsC1ERKS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
// G64-DAG: define linkonce_odr                void @_ZN19ImportDefaultedDefsC1ERKS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
inline ImportDefaultedDefs::ImportDefaultedDefs(const ImportDefaultedDefs&) = default;

// M32-DAG: declare dllimport   x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaultedDefs@@"* @"\01??4ImportDefaultedDefs@@QAEAAU0@ABU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"*, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// M64-DAG: declare dllimport                  dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaultedDefs@@"* @"\01??4ImportDefaultedDefs@@QEAAAEAU0@AEBU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"*, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// G32-DAG: define linkonce_odr x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z19ImportDefaultedDefs* @_ZN19ImportDefaultedDefsaSERKS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
// G64-DAG: define linkonce_odr                dereferenceable({{[0-9]+}}) %struct._Z19ImportDefaultedDefs* @_ZN19ImportDefaultedDefsaSERKS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
inline ImportDefaultedDefs& ImportDefaultedDefs::operator=(const ImportDefaultedDefs&) = default;

// M32-DAG: define x86_thiscallcc %"struct.\01?ImportDefaultedDefs@@"* @"\01??0ImportDefaultedDefs@@QAE@$$QAU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"* returned %this, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// M64-DAG: define                %"struct.\01?ImportDefaultedDefs@@"* @"\01??0ImportDefaultedDefs@@QEAA@$$QEAU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"* returned %this, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// G32-DAG: define x86_thiscallcc void @_ZN19ImportDefaultedDefsC1EOS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
// G64-DAG: define                void @_ZN19ImportDefaultedDefsC1EOS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
// G32-DAG: define x86_thiscallcc void @_ZN19ImportDefaultedDefsC2EOS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
// G64-DAG: define                void @_ZN19ImportDefaultedDefsC2EOS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
ImportDefaultedDefs::ImportDefaultedDefs(ImportDefaultedDefs&&) = default; // dllimport ignored

// M32-DAG: define x86_thiscallcc dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaultedDefs@@"* @"\01??4ImportDefaultedDefs@@QAEAAU0@$$QAU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"* %this, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// M64-DAG: define                dereferenceable({{[0-9]+}}) %"struct.\01?ImportDefaultedDefs@@"* @"\01??4ImportDefaultedDefs@@QEAAAEAU0@$$QEAU0@@Z"(%"struct.\01?ImportDefaultedDefs@@"* %this, %"struct.\01?ImportDefaultedDefs@@"* dereferenceable({{[0-9]+}}))
// G32-DAG: define x86_thiscallcc dereferenceable({{[0-9]+}}) %struct._Z19ImportDefaultedDefs* @_ZN19ImportDefaultedDefsaSEOS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
// G64-DAG: define                dereferenceable({{[0-9]+}}) %struct._Z19ImportDefaultedDefs* @_ZN19ImportDefaultedDefsaSEOS_(%struct._Z19ImportDefaultedDefs* %this, %struct._Z19ImportDefaultedDefs* dereferenceable({{[0-9]+}}))
ImportDefaultedDefs& ImportDefaultedDefs::operator=(ImportDefaultedDefs&&) = default; // dllimport ignored

USESPECIALS(ImportDefaultedDefs)


// Import allocation functions.
struct ImportAlloc {
  __declspec(dllimport) void* operator new(__SIZE_TYPE__);
  __declspec(dllimport) void* operator new[](__SIZE_TYPE__);
  __declspec(dllimport) void operator delete(void*);
  __declspec(dllimport) void operator delete[](void*);
};

// M32-DAG: declare dllimport i8* @"\01??2ImportAlloc@@SAPAXI@Z"(i32)
// M64-DAG: declare dllimport i8* @"\01??2ImportAlloc@@SAPEAX_K@Z"(i64)
// G32-DAG: declare dllimport i8* @_ZN11ImportAllocnwEj(i32)
// G64-DAG: declare dllimport i8* @_ZN11ImportAllocnwEy(i64)
void UNIQ(use)() { new ImportAlloc(); }

// M32-DAG: declare dllimport i8* @"\01??_UImportAlloc@@SAPAXI@Z"(i32)
// M64-DAG: declare dllimport i8* @"\01??_UImportAlloc@@SAPEAX_K@Z"(i64)
// G32-DAG: declare dllimport i8* @_ZN11ImportAllocnaEj(i32)
// G64-DAG: declare dllimport i8* @_ZN11ImportAllocnaEy(i64)
void UNIQ(use)() { new ImportAlloc[1]; }

// M32-DAG: declare dllimport void @"\01??3ImportAlloc@@SAXPAX@Z"(i8*)
// M64-DAG: declare dllimport void @"\01??3ImportAlloc@@SAXPEAX@Z"(i8*)
// G32-DAG: declare dllimport void @_ZN11ImportAllocdlEPv(i8*)
// G64-DAG: declare dllimport void @_ZN11ImportAllocdlEPv(i8*)
void UNIQ(use)(ImportAlloc* ptr) { delete ptr; }

// M32-DAG: declare dllimport void @"\01??_VImportAlloc@@SAXPAX@Z"(i8*)
// M64-DAG: declare dllimport void @"\01??_VImportAlloc@@SAXPEAX@Z"(i8*)
// G32-DAG: declare dllimport void @_ZN11ImportAllocdaEPv(i8*)
// G64-DAG: declare dllimport void @_ZN11ImportAllocdaEPv(i8*)
void UNIQ(use)(ImportAlloc* ptr) { delete[] ptr; }


//===----------------------------------------------------------------------===//
// Class member templates
//===----------------------------------------------------------------------===//

struct MemFunTmpl {
  template<typename T>                              void normalDef() {}
  template<typename T> __declspec(dllimport)        void importedNormal() {}
  template<typename T>                       static void staticDef() {}
  template<typename T> __declspec(dllimport) static void importedStatic() {}
};

// Import implicit instantiation of an imported member function template.
// M32-DAG: declare dllimport   x86_thiscallcc void @"\01??$importedNormal@UImplicitInst_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                  void @"\01??$importedNormal@UImplicitInst_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN10MemFunTmpl14importedNormalI21ImplicitInst_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
// G64-DAG: define linkonce_odr                void @_ZN10MemFunTmpl14importedNormalI21ImplicitInst_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
USEMF(MemFunTmpl, importedNormal<ImplicitInst_Imported>)

// MSC-DAG: declare dllimport                void @"\01??$importedStatic@UImplicitInst_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: define linkonce_odr              void @_ZN10MemFunTmpl14importedStaticI21ImplicitInst_ImportedEEvv()
USE(MemFunTmpl::importedStatic<ImplicitInst_Imported>)


// Import explicit instantiation declaration of an imported member function
// template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??$importedNormal@UExplicitDecl_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                void @"\01??$importedNormal@UExplicitDecl_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: declare x86_thiscallcc           void @_ZN10MemFunTmpl14importedNormalI21ExplicitDecl_ImportedEEvv(%struct._Z10MemFunTmpl*)
// G64-DAG: declare                          void @_ZN10MemFunTmpl14importedNormalI21ExplicitDecl_ImportedEEvv(%struct._Z10MemFunTmpl*)
extern template void MemFunTmpl::importedNormal<ExplicitDecl_Imported>();
USEMF(MemFunTmpl, importedNormal<ExplicitDecl_Imported>)

// MSC-DAG: declare dllimport                void @"\01??$importedStatic@UExplicitDecl_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: declare                          void @_ZN10MemFunTmpl14importedStaticI21ExplicitDecl_ImportedEEvv()
extern template void MemFunTmpl::importedStatic<ExplicitDecl_Imported>();
USE(MemFunTmpl::importedStatic<ExplicitDecl_Imported>)


// Import explicit instantiation definition of an imported member function
// template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??$importedNormal@UExplicitInst_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                void @"\01??$importedNormal@UExplicitInst_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: define weak_odr x86_thiscallcc   void @_ZN10MemFunTmpl14importedNormalI21ExplicitInst_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
// G64-DAG: define weak_odr                  void @_ZN10MemFunTmpl14importedNormalI21ExplicitInst_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
template void MemFunTmpl::importedNormal<ExplicitInst_Imported>();
USEMF(MemFunTmpl, importedNormal<ExplicitInst_Imported>)

// MSC-DAG: declare dllimport                void @"\01??$importedStatic@UExplicitInst_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: define weak_odr                  void @_ZN10MemFunTmpl14importedStaticI21ExplicitInst_ImportedEEvv()
template void MemFunTmpl::importedStatic<ExplicitInst_Imported>();
USE(MemFunTmpl::importedStatic<ExplicitInst_Imported>)


// Import specialization of an imported member function template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??$importedNormal@UExplicitSpec_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                void @"\01??$importedNormal@UExplicitSpec_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN10MemFunTmpl14importedNormalI21ExplicitSpec_ImportedEEvv(%struct._Z10MemFunTmpl*)
// G64-DAG: declare dllimport                void @_ZN10MemFunTmpl14importedNormalI21ExplicitSpec_ImportedEEvv(%struct._Z10MemFunTmpl*)
template<> __declspec(dllimport) void MemFunTmpl::importedNormal<ExplicitSpec_Imported>();
USEMF(MemFunTmpl, importedNormal<ExplicitSpec_Imported>)

// M32-DAG-FIXME: declare dllimport x86_thiscallcc void @"\01??$importedNormal@UExplicitSpec_Def_Imported@@@MemFunTmpl@@QAEXXZ"(%struct._Z10MemFunTmpl*)
// M64-DAG-FIXME: declare dllimport                void @"\01??$importedNormal@UExplicitSpec_Def_Imported@@@MemFunTmpl@@QEAAXXZ"(%struct._Z10MemFunTmpl*)
#ifdef MSABI
//template<> __declspec(dllimport) void MemFunTmpl::importedNormal<ExplicitSpec_Def_Imported>() {}
//USEMF(MemFunTmpl, importedNormal<ExplicitSpec_Def_Imported>)
#endif

// M32-DAG: declare dllimport   x86_thiscallcc void @"\01??$importedNormal@UExplicitSpec_InlineDef_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                  void @"\01??$importedNormal@UExplicitSpec_InlineDef_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN10MemFunTmpl14importedNormalI31ExplicitSpec_InlineDef_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
// G64-DAG: define linkonce_odr                void @_ZN10MemFunTmpl14importedNormalI31ExplicitSpec_InlineDef_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
template<> __declspec(dllimport) inline void MemFunTmpl::importedNormal<ExplicitSpec_InlineDef_Imported>() {}
USEMF(MemFunTmpl, importedNormal<ExplicitSpec_InlineDef_Imported>)


// MSC-DAG: declare dllimport                void @"\01??$importedStatic@UExplicitSpec_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: declare dllimport                void @_ZN10MemFunTmpl14importedStaticI21ExplicitSpec_ImportedEEvv()
template<> __declspec(dllimport) void MemFunTmpl::importedStatic<ExplicitSpec_Imported>();
USE(MemFunTmpl::importedStatic<ExplicitSpec_Imported>)

// MSC-DAG-FIXME: declare dllimport                void @"\01??$importedStatic@UExplicitSpec_Def_Imported@@@MemFunTmpl@@SAXXZ"()
#ifdef MSABI
//template<> __declspec(dllimport) void MemFunTmpl::importedStatic<ExplicitSpec_Def_Imported>() {}
//USE(MemFunTmpl::importedStatic<ExplicitSpec_Def_Imported>)
#endif

// MSC-DAG: declare dllimport                void @"\01??$importedStatic@UExplicitSpec_InlineDef_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: define linkonce_odr              void @_ZN10MemFunTmpl14importedStaticI31ExplicitSpec_InlineDef_ImportedEEvv()
template<> __declspec(dllimport) inline void MemFunTmpl::importedStatic<ExplicitSpec_InlineDef_Imported>() {}
USE(MemFunTmpl::importedStatic<ExplicitSpec_InlineDef_Imported>)


// Not importing specialization of an imported member function template without
// explicit dllimport.
// M32-DAG: define x86_thiscallcc void @"\01??$importedNormal@UExplicitSpec_NotImported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"* %this)
// M64-DAG: define                void @"\01??$importedNormal@UExplicitSpec_NotImported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"* %this)
// G32-DAG: define x86_thiscallcc void @_ZN10MemFunTmpl14importedNormalI24ExplicitSpec_NotImportedEEvv(%struct._Z10MemFunTmpl* %this)
// G64-DAG: define                void @_ZN10MemFunTmpl14importedNormalI24ExplicitSpec_NotImportedEEvv(%struct._Z10MemFunTmpl* %this)
template<> void MemFunTmpl::importedNormal<ExplicitSpec_NotImported>() {}
USEMF(MemFunTmpl, importedNormal<ExplicitSpec_NotImported>)

// MSC-DAG: define                void @"\01??$importedStatic@UExplicitSpec_NotImported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: define                void @_ZN10MemFunTmpl14importedStaticI24ExplicitSpec_NotImportedEEvv()
template<> void MemFunTmpl::importedStatic<ExplicitSpec_NotImported>() {}
USE(MemFunTmpl::importedStatic<ExplicitSpec_NotImported>)


// Import explicit instantiation declaration of a non-imported member function
// template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??$normalDef@UExplicitDecl_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                void @"\01??$normalDef@UExplicitDecl_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: declare x86_thiscallcc           void @_ZN10MemFunTmpl9normalDefI21ExplicitDecl_ImportedEEvv(%struct._Z10MemFunTmpl*)
// G64-DAG: declare                          void @_ZN10MemFunTmpl9normalDefI21ExplicitDecl_ImportedEEvv(%struct._Z10MemFunTmpl*)
extern template __declspec(dllimport) void MemFunTmpl::normalDef<ExplicitDecl_Imported>();
USEMF(MemFunTmpl, normalDef<ExplicitDecl_Imported>)

// MSC-DAG: declare dllimport                void @"\01??$staticDef@UExplicitDecl_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: declare                          void @_ZN10MemFunTmpl9staticDefI21ExplicitDecl_ImportedEEvv()
extern template __declspec(dllimport) void MemFunTmpl::staticDef<ExplicitDecl_Imported>();
USE(MemFunTmpl::staticDef<ExplicitDecl_Imported>)


// Import explicit instantiation definition of a non-imported member function
// template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??$normalDef@UExplicitInst_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                void @"\01??$normalDef@UExplicitInst_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: define weak_odr x86_thiscallcc   void @_ZN10MemFunTmpl9normalDefI21ExplicitInst_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
// G64-DAG: define weak_odr                  void @_ZN10MemFunTmpl9normalDefI21ExplicitInst_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
template __declspec(dllimport) void MemFunTmpl::normalDef<ExplicitInst_Imported>();
USEMF(MemFunTmpl, normalDef<ExplicitInst_Imported>)

// MSC-DAG: declare dllimport                void @"\01??$staticDef@UExplicitInst_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: define weak_odr                  void @_ZN10MemFunTmpl9staticDefI21ExplicitInst_ImportedEEvv()
template __declspec(dllimport) void MemFunTmpl::staticDef<ExplicitInst_Imported>();
USE(MemFunTmpl::staticDef<ExplicitInst_Imported>)


// Import specialization of a non-imported member function template.
// M32-DAG: declare dllimport x86_thiscallcc void @"\01??$normalDef@UExplicitSpec_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                void @"\01??$normalDef@UExplicitSpec_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: declare dllimport x86_thiscallcc void @_ZN10MemFunTmpl9normalDefI21ExplicitSpec_ImportedEEvv(%struct._Z10MemFunTmpl*)
// G64-DAG: declare dllimport                void @_ZN10MemFunTmpl9normalDefI21ExplicitSpec_ImportedEEvv(%struct._Z10MemFunTmpl*)
template<> __declspec(dllimport) void MemFunTmpl::normalDef<ExplicitSpec_Imported>();
USEMF(MemFunTmpl, normalDef<ExplicitSpec_Imported>)

// M32-DAG-FIXME: declare dllimport x86_thiscallcc void @"\01??$normalDef@UExplicitSpec_Def_Imported@@@MemFunTmpl@@QAEXXZ"(%struct._Z10MemFunTmpl*)
// M64-DAG-FIXME: declare dllimport                void @"\01??$normalDef@UExplicitSpec_Def_Imported@@@MemFunTmpl@@QEAAXXZ"(%struct._Z10MemFunTmpl*)
#ifdef MSABI
//template<> __declspec(dllimport) void MemFunTmpl::normalDef<ExplicitSpec_Def_Imported>() {}
//USEMF(MemFunTmpl, normalDef<ExplicitSpec_Def_Imported>)
#endif

// M32-DAG: declare dllimport   x86_thiscallcc void @"\01??$normalDef@UExplicitSpec_InlineDef_Imported@@@MemFunTmpl@@QAEXXZ"(%"struct.\01?MemFunTmpl@@"*)
// M64-DAG: declare dllimport                  void @"\01??$normalDef@UExplicitSpec_InlineDef_Imported@@@MemFunTmpl@@QEAAXXZ"(%"struct.\01?MemFunTmpl@@"*)
// G32-DAG: define linkonce_odr x86_thiscallcc void @_ZN10MemFunTmpl9normalDefI31ExplicitSpec_InlineDef_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
// G64-DAG: define linkonce_odr                void @_ZN10MemFunTmpl9normalDefI31ExplicitSpec_InlineDef_ImportedEEvv(%struct._Z10MemFunTmpl* %this)
template<> __declspec(dllimport) inline void MemFunTmpl::normalDef<ExplicitSpec_InlineDef_Imported>() {}
USEMF(MemFunTmpl, normalDef<ExplicitSpec_InlineDef_Imported>)


// MSC-DAG: declare dllimport void @"\01??$staticDef@UExplicitSpec_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: declare dllimport void @_ZN10MemFunTmpl9staticDefI21ExplicitSpec_ImportedEEvv()
template<> __declspec(dllimport) void MemFunTmpl::staticDef<ExplicitSpec_Imported>();
USE(MemFunTmpl::staticDef<ExplicitSpec_Imported>)

// MSC-DAG-FIXME: declare dllimport void @"\01??$staticDef@UExplicitSpec_Def_Imported@@@MemFunTmpl@@SAXXZ"()
#ifdef MSABI
//template<> __declspec(dllimport) void MemFunTmpl::staticDef<ExplicitSpec_Def_Imported>() {}
//USE(MemFunTmpl::staticDef<ExplicitSpec_Def_Imported>)
#endif

// MSC-DAG: declare dllimport void @"\01??$staticDef@UExplicitSpec_InlineDef_Imported@@@MemFunTmpl@@SAXXZ"()
// GNU-DAG: define linkonce_odr void @_ZN10MemFunTmpl9staticDefI31ExplicitSpec_InlineDef_ImportedEEvv()
template<> __declspec(dllimport) inline void MemFunTmpl::staticDef<ExplicitSpec_InlineDef_Imported>() {}
USE(MemFunTmpl::staticDef<ExplicitSpec_InlineDef_Imported>)



struct MemVarTmpl {
  template<typename T>                       static const int StaticVar = 1;
  template<typename T> __declspec(dllimport) static const int ImportedStaticVar = 1;
};

// Import implicit instantiation of an imported member variable template.
// MSC-DAG: @"\01??$ImportedStaticVar@UImplicitInst_Imported@@@MemVarTmpl@@2HB" = available_externally dllimport constant i32 1, align 4
// GNU-DAG: @_ZN10MemVarTmpl17ImportedStaticVarI21ImplicitInst_ImportedEE       = external dllimport constant i32
USEMV(MemVarTmpl, ImportedStaticVar<ImplicitInst_Imported>)

// Import explicit instantiation declaration of an imported member variable
// template.
// MSC-DAG: @"\01??$ImportedStaticVar@UExplicitDecl_Imported@@@MemVarTmpl@@2HB" = external dllimport constant i32
// GNU-DAG: @_ZN10MemVarTmpl17ImportedStaticVarI21ExplicitDecl_ImportedEE       = external dllimport constant i32
extern template const int MemVarTmpl::ImportedStaticVar<ExplicitDecl_Imported>;
USEMV(MemVarTmpl, ImportedStaticVar<ExplicitDecl_Imported>)

// An explicit instantiation definition of an imported member variable template
// cannot be imported because the template must be defined which is illegal. The
// in-class initializer does not count.

// Import specialization of an imported member variable template.
// MSC-DAG: @"\01??$ImportedStaticVar@UExplicitSpec_Imported@@@MemVarTmpl@@2HB" = external dllimport constant i32
// GNU-DAG: @_ZN10MemVarTmpl17ImportedStaticVarI21ExplicitSpec_ImportedEE       = external dllimport constant i32
template<> __declspec(dllimport) const int MemVarTmpl::ImportedStaticVar<ExplicitSpec_Imported>;
USEMV(MemVarTmpl, ImportedStaticVar<ExplicitSpec_Imported>)

// Not importing specialization of a member variable template without explicit
// dllimport.
// MSC-DAG: @"\01??$ImportedStaticVar@UExplicitSpec_NotImported@@@MemVarTmpl@@2HB" = external constant i32
// GNU-DAG: @_ZN10MemVarTmpl17ImportedStaticVarI24ExplicitSpec_NotImportedEE       = external constant i32
template<> const int MemVarTmpl::ImportedStaticVar<ExplicitSpec_NotImported>;
USEMV(MemVarTmpl, ImportedStaticVar<ExplicitSpec_NotImported>)


// Import explicit instantiation declaration of a non-imported member variable
// template.
// MSC-DAG: @"\01??$StaticVar@UExplicitDecl_Imported@@@MemVarTmpl@@2HB" = external dllimport constant i32
// GNU-DAG: @_ZN10MemVarTmpl9StaticVarI21ExplicitDecl_ImportedEE        = external dllimport constant i32
extern template __declspec(dllimport) const int MemVarTmpl::StaticVar<ExplicitDecl_Imported>;
USEMV(MemVarTmpl, StaticVar<ExplicitDecl_Imported>)

// An explicit instantiation definition of a non-imported member variable template
// cannot be imported because the template must be defined which is illegal. The
// in-class initializer does not count.

// Import specialization of a non-imported member variable template.
// MSC-DAG: @"\01??$StaticVar@UExplicitSpec_Imported@@@MemVarTmpl@@2HB" = external dllimport constant i32
// GNU-DAG: @_ZN10MemVarTmpl9StaticVarI21ExplicitSpec_ImportedEE        = external dllimport constant i32
template<> __declspec(dllimport) const int MemVarTmpl::StaticVar<ExplicitSpec_Imported>;
USEMV(MemVarTmpl, StaticVar<ExplicitSpec_Imported>)
