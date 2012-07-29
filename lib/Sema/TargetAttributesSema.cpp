//===-- TargetAttributesSema.cpp - Encapsulate target attributes-*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains semantic analysis implementation for target-specific
// attributes.
//
//===----------------------------------------------------------------------===//

#include "TargetAttributesSema.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/Template.h"
#include "clang/Sema/TemplateDeduction.h"
#include "llvm/ADT/Triple.h"

using namespace clang;

TargetAttributesSema::~TargetAttributesSema() {}
bool TargetAttributesSema::ProcessDeclAttribute(Scope *scope, Decl *D,
                                    const AttributeList &Attr, Sema &S) const {
  return false;
}

static void HandleMSP430InterruptAttr(Decl *d,
                                      const AttributeList &Attr, Sema &S) {
    // Check the attribute arguments.
    if (Attr.getNumArgs() != 1) {
      S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
        << Attr.getName() << 1;
      return;
    }

    // FIXME: Check for decl - it should be void ()(void).

    Expr *NumParamsExpr = static_cast<Expr *>(Attr.getArg(0));
    llvm::APSInt NumParams(32);
    if (!NumParamsExpr->isIntegerConstantExpr(NumParams, S.Context)) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
        << Attr.getName() << AANT_ArgumentIntegerConstant
        << NumParamsExpr->getSourceRange();
      return;
    }

    unsigned Num = NumParams.getLimitedValue(255);
    if ((Num & 1) || Num > 30) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_out_of_bounds)
        << "interrupt" << (int)NumParams.getSExtValue()
        << NumParamsExpr->getSourceRange();
      return;
    }

    d->addAttr(::new (S.Context) MSP430InterruptAttr(Attr.getLoc(), S.Context, Num));
    d->addAttr(::new (S.Context) UsedAttr(Attr.getLoc(), S.Context));
  }

namespace {
  class MSP430AttributesSema : public TargetAttributesSema {
  public:
    MSP430AttributesSema() { }
    bool ProcessDeclAttribute(Scope *scope, Decl *D,
                              const AttributeList &Attr, Sema &S) const {
      if (Attr.getName()->getName() == "interrupt") {
        HandleMSP430InterruptAttr(D, Attr, S);
        return true;
      }
      return false;
    }
  };
}

static void HandleX86ForceAlignArgPointerAttr(Decl *D,
                                              const AttributeList& Attr,
                                              Sema &S) {
  // Check the attribute arguments.
  if (Attr.getNumArgs() != 0) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 0;
    return;
  }

  // If we try to apply it to a function pointer, don't warn, but don't
  // do anything, either. It doesn't matter anyway, because there's nothing
  // special about calling a force_align_arg_pointer function.
  ValueDecl *VD = dyn_cast<ValueDecl>(D);
  if (VD && VD->getType()->isFunctionPointerType())
    return;
  // Also don't warn on function pointer typedefs.
  TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(D);
  if (TD && (TD->getUnderlyingType()->isFunctionPointerType() ||
             TD->getUnderlyingType()->isFunctionType()))
    return;
  // Attribute can only be applied to function types.
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << /* function */0;
    return;
  }

  D->addAttr(::new (S.Context) X86ForceAlignArgPointerAttr(Attr.getRange(),
                                                           S.Context));
}

DLLImportAttr *Sema::mergeDLLImportAttr(Decl *D, SourceRange Range,
                                        unsigned AttrSpellingListIndex) {
  if (D->hasAttr<DLLExportAttr>()) {
    Diag(Range.getBegin(), diag::warn_attribute_ignored) << "dllimport";
    return NULL;
  }

  if (D->hasAttr<DLLImportAttr>())
    return NULL;

  if (VarDecl *VD = dyn_cast<VarDecl>(D)) {
    if (VD->hasDefinition()) {
      // dllimport cannot be applied to definitions.
      Diag(D->getLocation(), diag::warn_attribute_invalid_on_definition)
        << "dllimport";
      return NULL;
    }
  }

  return ::new (Context) DLLImportAttr(Range, Context,
                                       AttrSpellingListIndex);
}

static void HandleDLLImportAttr(Decl *D, const AttributeList &Attr, Sema &S) {
  // check the attribute arguments.
  if (Attr.getNumArgs() != 0) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 0;
    return;
  }

  // Attribute can be applied only to functions or variables.
  FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
  if (!FD && !isa<VarDecl>(D)) {
    // Apparently Visual C++ thinks it is okay to not emit a warning
    // in this case, so only emit a warning when -fms-extensions is not
    // specified.
    if (!S.getLangOpts().MicrosoftExt)
      S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
        << Attr.getName() << 2 /*variable and function*/;
    return;
  }

  // Currently, the dllimport attribute is ignored for inlined functions.
  // Warning is emitted.
  if (FD && FD->isInlineSpecified()) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << "dllimport";
    return;
  }

  unsigned Index = Attr.getAttributeSpellingListIndex();
  DLLImportAttr *NewAttr = S.mergeDLLImportAttr(D, Attr.getRange(), Index);
  if (NewAttr)
    D->addAttr(NewAttr);
}

DLLExportAttr *Sema::mergeDLLExportAttr(Decl *D, SourceRange Range,
                                        unsigned AttrSpellingListIndex) {
  if (DLLImportAttr *Import = D->getAttr<DLLImportAttr>()) {
    Diag(Import->getLocation(), diag::warn_attribute_ignored) << "dllimport";
    D->dropAttr<DLLImportAttr>();
  }

  if (D->hasAttr<DLLExportAttr>())
    return NULL;

  return ::new (Context) DLLExportAttr(Range, Context,
                                       AttrSpellingListIndex);
}

static void HandleDLLExportAttr(Decl *D, const AttributeList &Attr, Sema &S) {
  // check the attribute arguments.
  if (Attr.getNumArgs() != 0) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 0;
    return;
  }

  // Attribute can be applied only to functions or variables.
  FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
  if (!FD && !isa<VarDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::warn_attribute_wrong_decl_type)
      << Attr.getName() << 2 /*variable and function*/;
    return;
  }

  // Currently, the dllexport attribute is ignored for inlined functions, unless
  // the -fkeep-inline-functions flag has been used. Warning is emitted;
  if (FD && FD->isInlineSpecified()) {
    // FIXME: ... unless the -fkeep-inline-functions flag has been used.
    S.Diag(Attr.getLoc(), diag::warn_attribute_ignored) << "dllexport";
    return;
  }

  unsigned Index = Attr.getAttributeSpellingListIndex();
  DLLExportAttr *NewAttr = S.mergeDLLExportAttr(D, Attr.getRange(), Index);
  if (NewAttr)
    D->addAttr(NewAttr);
}

namespace {
  class X86AttributesSema : public TargetAttributesSema {
  public:
    X86AttributesSema() { }
    bool ProcessDeclAttribute(Scope *scope, Decl *D,
                              const AttributeList &Attr, Sema &S) const {
      const llvm::Triple &Triple(S.Context.getTargetInfo().getTriple());
      if (Triple.getOS() == llvm::Triple::Win32 ||
          Triple.getOS() == llvm::Triple::MinGW32) {
        switch (Attr.getKind()) {
        case AttributeList::AT_DLLImport: HandleDLLImportAttr(D, Attr, S);
                                          return true;
        case AttributeList::AT_DLLExport: HandleDLLExportAttr(D, Attr, S);
                                          return true;
        default:                          break;
        }
      }
      if (Triple.getArch() != llvm::Triple::x86_64 &&
          (Attr.getName()->getName() == "force_align_arg_pointer" ||
           Attr.getName()->getName() == "__force_align_arg_pointer__")) {
        HandleX86ForceAlignArgPointerAttr(D, Attr, S);
        return true;
      }
      return false;
    }
  };
}

static void HandleMips16Attr(Decl *D, const AttributeList &Attr, Sema &S) {
  // check the attribute arguments.
  if (Attr.hasParameterOrArguments()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 0;
    return;
  }
  // Attribute can only be applied to function types.
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << /* function */0;
    return;
  }
  D->addAttr(::new (S.Context) Mips16Attr(Attr.getRange(), S.Context,
                                          Attr.getAttributeSpellingListIndex()));
}

static void HandleNoMips16Attr(Decl *D, const AttributeList &Attr, Sema &S) {
  // check the attribute arguments.
  if (Attr.hasParameterOrArguments()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 0;
    return;
  }
  // Attribute can only be applied to function types.
  if (!isa<FunctionDecl>(D)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_decl_type)
      << Attr.getName() << /* function */0;
    return;
  }
  D->addAttr(::new (S.Context)
             NoMips16Attr(Attr.getRange(), S.Context,
                          Attr.getAttributeSpellingListIndex()));
}

namespace {
  class MipsAttributesSema : public TargetAttributesSema {
  public:
    MipsAttributesSema() { }
    bool ProcessDeclAttribute(Scope *scope, Decl *D, const AttributeList &Attr,
                              Sema &S) const {
      if (Attr.getName()->getName() == "mips16") {
        HandleMips16Attr(D, Attr, S);
        return true;
      } else if (Attr.getName()->getName() == "nomips16") {
        HandleNoMips16Attr(D, Attr, S);
        return true;
      }
      return false;
    }
  };
}

namespace {
  class DuettoAttributesSema : public TargetAttributesSema {
  private:
    FunctionTemplateDecl* getTemplateFromName(Sema& S, const char* tName) const
    {
      const IdentifierInfo& info=S.Context.Idents.get(tName);
      DeclContext::lookup_result l=S.CurContext->lookup(DeclarationName(&info));
      if(l.size() != 1)
      {
         llvm::errs() << "Missing special definition\n";
         ::abort();
      }
      return dyn_cast<FunctionTemplateDecl>(l[0]);
    }
    void handleClient(Sema &S, Decl* D, const AttributeList &attr) const
    {
      D->addAttr(::new (S.Context) ClientAttr(attr.getRange(), S.Context));
    }
    void handleServer(Sema &S, Decl* D, const AttributeList &attr) const
    {
      D->addAttr(::new (S.Context) ServerAttr(attr.getRange(), S.Context));
      //This should be a function
      FunctionDecl* F=dyn_cast<FunctionDecl>(D);
      assert(F);
      //Skel for the server
      FunctionTemplateDecl* skelTemplateDecl=getTemplateFromName(S,"serverSkel");
      QualType funcType=F->getType();
      QualType funcPtrType=S.Context.getPointerType(funcType);
      QualType resultType=F->getResultType();
      CanQualType canonicalFuncPtrType=S.Context.getCanonicalType(funcPtrType);
      CanQualType canonicalResultType=S.Context.getCanonicalType(resultType);
      SmallVector<DeducedTemplateArgument,4> Deduced;
      Deduced.push_back(DeducedTemplateArgument(TemplateArgument(canonicalFuncPtrType)));
      Deduced.push_back(DeducedTemplateArgument(TemplateArgument(F, false)));
      Deduced.push_back(DeducedTemplateArgument(TemplateArgument(canonicalResultType)));
      //Add the types of the function argument
      FunctionDecl::param_iterator it=F->param_begin();
      SmallVector<TemplateArgument,4> FArgsPack;
      for(;it!=F->param_end();++it)
	  FArgsPack.push_back(TemplateArgument((*it)->getOriginalType()));

      if(F->param_size()!=0)
	  Deduced.push_back(DeducedTemplateArgument(TemplateArgument(&FArgsPack[0],FArgsPack.size())));
      else
	  Deduced.push_back(DeducedTemplateArgument(TemplateArgument((const TemplateArgument*)NULL,0)));

      sema::TemplateDeductionInfo info2(attr.getLoc());
      FunctionDecl* skelFn;
#ifndef NDEBUG
      Sema::TemplateDeductionResult ret2=
#endif
      S.FinishTemplateArgumentDeduction(skelTemplateDecl, Deduced, 1, skelFn, info2, NULL);
      assert(ret2==Sema::TDK_Success);
      S.InstantiateFunctionDefinition(attr.getLoc(), skelFn, true, true);
      S.WeakTopLevelDecls().push_back(skelFn);
      //Force the function to be used, so that it's emitted
      skelFn->addAttr(::new (S.Context) UsedAttr(attr.getLoc(), S.Context));
      F->skelFunction = skelFn;
      //Stub for the client
      FunctionTemplateDecl* stubTemplateDecl=getTemplateFromName(S,"clientStub");
      Deduced.clear();
      Deduced.push_back(DeducedTemplateArgument(TemplateArgument(canonicalResultType)));
      //Add the types of the function argument
      if(F->param_size()!=0)
          Deduced.push_back(DeducedTemplateArgument(TemplateArgument(&FArgsPack[0],FArgsPack.size())));
      else
	  Deduced.push_back(DeducedTemplateArgument(TemplateArgument((const TemplateArgument*)NULL,0)));

      FunctionDecl* stubFn;
#ifndef NDEBUG
      ret2=
#endif
      S.FinishTemplateArgumentDeduction(stubTemplateDecl, Deduced, 1, stubFn, info2, NULL);
      S.InstantiateFunctionDefinition(attr.getLoc(), stubFn, true, true);
      S.WeakTopLevelDecls().push_back(stubFn);
      //Force the function to be used, so that it's emitted
      stubFn->addAttr(::new (S.Context) UsedAttr(attr.getLoc(), S.Context));
      F->stubFunction = stubFn;
    }
  public:
    DuettoAttributesSema() { }
    bool ProcessDeclAttribute(Scope *scope, Decl *D, const AttributeList &Attr,
                              Sema &S) const {
      if (Attr.getKind() == AttributeList::AT_Client)
      {
        handleClient(S, D, Attr);
	return true;
      }
      else if (Attr.getKind() == AttributeList::AT_Server)
      {
        handleServer(S, D, Attr);
	return true;
      }
      return false;
    }
  };
}

const TargetAttributesSema &Sema::getTargetAttributesSema() const {
  if (TheTargetAttributesSema)
    return *TheTargetAttributesSema;

  const llvm::Triple &Triple(Context.getTargetInfo().getTriple());
  switch (Triple.getArch()) {
  case llvm::Triple::msp430:
    return *(TheTargetAttributesSema = new MSP430AttributesSema);
  case llvm::Triple::x86:
  case llvm::Triple::x86_64:
    return *(TheTargetAttributesSema = new X86AttributesSema);
  case llvm::Triple::mips:
  case llvm::Triple::mipsel:
    return *(TheTargetAttributesSema = new MipsAttributesSema);
  case llvm::Triple::duetto:
    return *(TheTargetAttributesSema = new DuettoAttributesSema);
  default:
    return *(TheTargetAttributesSema = new TargetAttributesSema);
  }
}
