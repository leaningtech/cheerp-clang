//=== WebAssembly.h - Declare WebAssembly target feature support *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares WebAssembly TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_WEBASSEMBLY_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_WEBASSEMBLY_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Compiler.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY WebAssemblyTargetInfo : public TargetInfo {
  static const Builtin::Info BuiltinInfo[];

  enum SIMDEnum {
    NoSIMD,
    SIMD128,
  } SIMDLevel;

public:
  explicit WebAssemblyTargetInfo(const llvm::Triple &T, const TargetOptions &)
      : TargetInfo(T), SIMDLevel(NoSIMD) {
    NoAsmVariants = true;
    SuitableAlign = 128;
    LargeArrayMinWidth = 128;
    LargeArrayAlign = 128;
    SimdDefaultAlign = 128;
    SigAtomicType = SignedLong;
    LongDoubleWidth = LongDoubleAlign = 128;
    LongDoubleFormat = &llvm::APFloat::IEEEquad();
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

private:
  bool
  initFeatureMap(llvm::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override {
    if (CPU == "bleeding-edge")
      Features["simd128"] = true;
    return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
  }

  bool hasFeature(StringRef Feature) const final;

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) final;

  bool isValidCPUName(StringRef Name) const final;

  bool setCPU(const std::string &Name) final { return isValidCPUName(Name); }

  ArrayRef<Builtin::Info> getTargetBuiltins() const final;

  BuiltinVaListKind getBuiltinVaListKind() const final {
    return VoidPtrBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const final { return None; }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const final {
    return None;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const final {
    return false;
  }

  const char *getClobbers() const final { return ""; }

  bool isCLZForZeroUndef() const final { return false; }

  bool hasInt128Type() const final { return true; }

  IntType getIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // WebAssembly prefers long long for explicitly 64-bit integers.
    return BitWidth == 64 ? (IsSigned ? SignedLongLong : UnsignedLongLong)
                          : TargetInfo::getIntTypeByWidth(BitWidth, IsSigned);
  }

  IntType getLeastIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // WebAssembly uses long long for int_least64_t and int_fast64_t.
    return BitWidth == 64
               ? (IsSigned ? SignedLongLong : UnsignedLongLong)
               : TargetInfo::getLeastIntTypeByWidth(BitWidth, IsSigned);
  }
};
class LLVM_LIBRARY_VISIBILITY WebAssembly32TargetInfo
    : public WebAssemblyTargetInfo {
public:
  explicit WebAssembly32TargetInfo(const llvm::Triple &T,
                                   const TargetOptions &Opts)
      : WebAssemblyTargetInfo(T, Opts) {
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    resetDataLayout("e-m:e-p:32:32-i64:64-n32:64-S128");
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY WebAssembly64TargetInfo
    : public WebAssemblyTargetInfo {
public:
  explicit WebAssembly64TargetInfo(const llvm::Triple &T,
                                   const TargetOptions &Opts)
      : WebAssemblyTargetInfo(T, Opts) {
    LongAlign = LongWidth = 64;
    PointerAlign = PointerWidth = 64;
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    resetDataLayout("e-m:e-p:64:64-i64:64-n32:64-S128");
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

// Duetto base class
class DuettoTargetInfo : public TargetInfo {
public:
  DuettoTargetInfo(const llvm::Triple &triple) : TargetInfo(triple) {
    DescriptionString = "b-e-p:32:8-i16:8-i32:8-"
                        "i64:8-f32:8-f64:8-"
                        "a:0:8-f80:8-n8:8:8-S8";
    BigEndian = false;
    ByteAddressable = false;
    PointerAlign = 8;
    ShortAlign = 8;
    IntAlign = 8;
    LongAlign = 8;
    LongLongAlign = 8;
    SuitableAlign = 8;
    HalfAlign = 8;
    FloatAlign = 8;
    DoubleAlign = 8;
    LongDoubleAlign = 8;
    SizeType = UnsignedInt;
  }

  virtual ArrayRef<Builtin::Info> getTargetBuiltins() const {
    return llvm::makeArrayRef(BuiltinInfo, clang::Cheerp::LastTSBuiltin - Builtin::FirstTSBuiltin);
  }

  virtual void getTargetDefines(const LangOptions &Opts,
                                MacroBuilder &Builder) const {
    // Target identification.
    Builder.defineMacro("__DUETTO__");

    if (Opts.CPlusPlus)
      Builder.defineMacro("_GNU_SOURCE");

    Builder.defineMacro("__LITTLE_ENDIAN__");
  }

  virtual BuiltinVaListKind getBuiltinVaListKind() const {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  virtual ArrayRef<const char *> getGCCRegNames() const {
    return None;
  }

  virtual ArrayRef<GCCRegAlias> getGCCRegAliases() const {
    return None;
  }

  virtual bool validateAsmConstraint(const char *&Name,
                                     TargetInfo::ConstraintInfo &Info) const {
    return false;
  }
  virtual const char *getClobbers() const {
    return "";
  }
};
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_WEBASSEMBLY_H
