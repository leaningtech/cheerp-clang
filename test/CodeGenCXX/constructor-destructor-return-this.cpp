//RUN: %clang_cc1 %s -emit-llvm -o - -triple=i686-unknown-linux | FileCheck --check-prefix=CHECKGEN %s
//RUN: %clang_cc1 %s -emit-llvm -o - -triple=thumbv7-apple-ios6.0 -target-abi apcs-gnu | FileCheck --check-prefix=CHECKARM %s
//RUN: %clang_cc1 %s -emit-llvm -o - -triple=thumbv7-apple-ios5.0 -target-abi apcs-gnu | FileCheck --check-prefix=CHECKIOS5 %s
//RUN: %clang_cc1 %s -emit-llvm -o - -triple=i386-pc-win32 -fno-rtti | FileCheck --check-prefix=CHECKMS %s
// FIXME: these tests crash on the bots when run with -triple=x86_64-pc-win32

// Make sure we attach the 'returned' attribute to the 'this' parameter of
// constructors and destructors which return this (and only these cases)

class A {
public:
  A();
  virtual ~A();

private:
  int x_;
};

class B : public A {
public:
  B(int *i);
  virtual ~B();

private:
  int *i_;
};

B::B(int *i) : i_(i) { }
B::~B() { }

// CHECKGEN-LABEL: define void @_ZN1BC2EPi(%class._Z1B* %this, i32* %i)
// CHECKGEN-LABEL: define void @_ZN1BC1EPi(%class._Z1B* %this, i32* %i)
// CHECKGEN-LABEL: define void @_ZN1BD2Ev(%class._Z1B* %this)
// CHECKGEN-LABEL: define void @_ZN1BD1Ev(%class._Z1B* %this)

// CHECKARM-LABEL: define %class._Z1B* @_ZN1BC2EPi(%class._Z1B* returned %this, i32* %i)
// CHECKARM-LABEL: define %class._Z1B* @_ZN1BC1EPi(%class._Z1B* returned %this, i32* %i)
// CHECKARM-LABEL: define %class._Z1B* @_ZN1BD2Ev(%class._Z1B* returned %this)
// CHECKARM-LABEL: define %class._Z1B* @_ZN1BD1Ev(%class._Z1B* returned %this)

// CHECKIOS5-LABEL: define %class._Z1B* @_ZN1BC2EPi(%class._Z1B* %this, i32* %i)
// CHECKIOS5-LABEL: define %class._Z1B* @_ZN1BC1EPi(%class._Z1B* %this, i32* %i)
// CHECKIOS5-LABEL: define %class._Z1B* @_ZN1BD2Ev(%class._Z1B* %this)
// CHECKIOS5-LABEL: define %class._Z1B* @_ZN1BD1Ev(%class._Z1B* %this)

// CHECKMS-LABEL: define x86_thiscallcc %"class.\01?B@@"* @"\01??0B@@QAE@PAH@Z"(%"class.\01?B@@"* returned %this, i32* %i)
// CHECKMS-LABEL: define x86_thiscallcc void @"\01??1B@@UAE@XZ"(%"class.\01?B@@"* %this)

class C : public A, public B {
public:
  C(int *i, char *c);
  virtual ~C();
private:
  char *c_;
};

C::C(int *i, char *c) : B(i), c_(c) { }
C::~C() { }

// CHECKGEN-LABEL: define void @_ZN1CC2EPiPc(%class._Z1C* %this, i32* %i, i8* %c)
// CHECKGEN-LABEL: define void @_ZN1CC1EPiPc(%class._Z1C* %this, i32* %i, i8* %c)
// CHECKGEN-LABEL: define void @_ZN1CD2Ev(%class._Z1C* %this)
// CHECKGEN-LABEL: define void @_ZN1CD1Ev(%class._Z1C* %this)
// CHECKGEN-LABEL: define void @_ZThn8_N1CD1Ev(%class._Z1C* %this)
// CHECKGEN-LABEL: define void @_ZN1CD0Ev(%class._Z1C* %this)
// CHECKGEN-LABEL: define void @_ZThn8_N1CD0Ev(%class._Z1C* %this)

// CHECKARM-LABEL: define %class._Z1C* @_ZN1CC2EPiPc(%class._Z1C* returned %this, i32* %i, i8* %c)
// CHECKARM-LABEL: define %class._Z1C* @_ZN1CC1EPiPc(%class._Z1C* returned %this, i32* %i, i8* %c)
// CHECKARM-LABEL: define %class._Z1C* @_ZN1CD2Ev(%class._Z1C* returned %this)
// CHECKARM-LABEL: define %class._Z1C* @_ZN1CD1Ev(%class._Z1C* returned %this)
// CHECKARM-LABEL: define %class._Z1C* @_ZThn8_N1CD1Ev(%class._Z1C* %this)
// CHECKARM-LABEL: define void @_ZN1CD0Ev(%class._Z1C* %this)
// CHECKARM-LABEL: define void @_ZThn8_N1CD0Ev(%class._Z1C* %this)

// CHECKIOS5-LABEL: define %class._Z1C* @_ZN1CC2EPiPc(%class._Z1C* %this, i32* %i, i8* %c)
// CHECKIOS5-LABEL: define %class._Z1C* @_ZN1CC1EPiPc(%class._Z1C* %this, i32* %i, i8* %c)
// CHECKIOS5-LABEL: define %class._Z1C* @_ZN1CD2Ev(%class._Z1C* %this)
// CHECKIOS5-LABEL: define %class._Z1C* @_ZN1CD1Ev(%class._Z1C* %this)
// CHECKIOS5-LABEL: define %class._Z1C* @_ZThn8_N1CD1Ev(%class._Z1C* %this)
// CHECKIOS5-LABEL: define void @_ZN1CD0Ev(%class._Z1C* %this)
// CHECKIOS5-LABEL: define void @_ZThn8_N1CD0Ev(%class._Z1C* %this)

// CHECKMS-LABEL: define x86_thiscallcc %"class.\01?C@@"* @"\01??0C@@QAE@PAHPAD@Z"(%"class.\01?C@@"* returned %this, i32* %i, i8* %c)
// CHECKMS-LABEL: define x86_thiscallcc void @"\01??1C@@UAE@XZ"(%"class.\01?C@@"* %this)

class D : public virtual A {
public:
  D();
  ~D();
};

D::D() { }
D::~D() { }

// CHECKGEN-LABEL: define void @_ZN1DC2Ev(%class._Z1D* %this, i8** %vtt)
// CHECKGEN-LABEL: define void @_ZN1DC1Ev(%class._Z1D* %this)
// CHECKGEN-LABEL: define void @_ZN1DD2Ev(%class._Z1D* %this, i8** %vtt)
// CHECKGEN-LABEL: define void @_ZN1DD1Ev(%class._Z1D* %this)

// CHECKARM-LABEL: define %class._Z1D* @_ZN1DC2Ev(%class._Z1D* returned %this, i8** %vtt)
// CHECKARM-LABEL: define %class._Z1D* @_ZN1DC1Ev(%class._Z1D* returned %this)
// CHECKARM-LABEL: define %class._Z1D* @_ZN1DD2Ev(%class._Z1D* returned %this, i8** %vtt)
// CHECKARM-LABEL: define %class._Z1D* @_ZN1DD1Ev(%class._Z1D* returned %this)

// CHECKIOS5-LABEL: define %class._Z1D* @_ZN1DC2Ev(%class._Z1D* %this, i8** %vtt)
// CHECKIOS5-LABEL: define %class._Z1D* @_ZN1DC1Ev(%class._Z1D* %this)
// CHECKIOS5-LABEL: define %class._Z1D* @_ZN1DD2Ev(%class._Z1D* %this, i8** %vtt)
// CHECKIOS5-LABEL: define %class._Z1D* @_ZN1DD1Ev(%class._Z1D* %this)

// CHECKMS-LABEL: define x86_thiscallcc %"class.\01?D@@"* @"\01??0D@@QAE@XZ"(%"class.\01?D@@"* returned %this, i32 %is_most_derived)
// CHECKMS-LABEL: define x86_thiscallcc void @"\01??1D@@UAE@XZ"(%"class.\01?D@@"*)

class E {
public:
  E();
  virtual ~E();
};

E* gete();

void test_destructor() {
  const E& e1 = E();
  E* e2 = gete();
  e2->~E();
}

// CHECKARM-LABEL: define void @_Z15test_destructorv()

// Verify that virtual calls to destructors are not marked with a 'returned'
// this parameter at the call site...
// CHECKARM: [[VFN:%.*]] = getelementptr inbounds %class._Z1E* (%class._Z1E*)**
// CHECKARM: [[THUNK:%.*]] = load %class._Z1E* (%class._Z1E*)** [[VFN]]
// CHECKARM: call %class._Z1E* [[THUNK]](%class._Z1E* %

// ...but static calls create declarations with 'returned' this
// CHECKARM: {{%.*}} = call %class._Z1E* @_ZN1ED1Ev(%class._Z1E* %

// CHECKARM: declare %class._Z1E* @_ZN1ED1Ev(%class._Z1E* returned)
