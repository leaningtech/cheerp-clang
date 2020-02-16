//===--- WebAssembly.cpp - WebAssembly ToolChain Implementation -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

wasm::Linker::Linker(const ToolChain &TC)
  : GnuTool("wasm::Linker", "lld", TC) {}

bool wasm::Linker::isLinkJob() const {
  return true;
}

bool wasm::Linker::hasIntegratedCPP() const {
  return false;
}

void wasm::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const ArgList &Args,
                                const char *LinkingOutput) const {

  const ToolChain &ToolChain = getToolChain();
  const char *Linker = Args.MakeArgString(ToolChain.GetLinkerPath());
  ArgStringList CmdArgs;
  CmdArgs.push_back("-flavor");
  CmdArgs.push_back("wasm");

  // Enable garbage collection of unused input sections by default, since code
  // size is of particular importance. This is significantly facilitated by
  // the enabling of -ffunction-sections and -fdata-sections in
  // Clang::ConstructJob.
  if (areOptimizationsEnabled(Args))
    CmdArgs.push_back("--gc-sections");

  if (Args.hasArg(options::OPT_rdynamic))
    CmdArgs.push_back("-export-dynamic");
  if (Args.hasArg(options::OPT_s))
    CmdArgs.push_back("--strip-all");
  if (Args.hasArg(options::OPT_shared))
    CmdArgs.push_back("-shared");
  if (Args.hasArg(options::OPT_static))
    CmdArgs.push_back("-Bstatic");

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  ToolChain.AddFilePathLibArgs(Args, CmdArgs);

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles)) {
    if (Args.hasArg(options::OPT_shared))
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("rcrt1.o")));
    else if (Args.hasArg(options::OPT_pie))
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("Scrt1.o")));
    else
      CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crt1.o")));

    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crti.o")));
  }

  AddLinkerInputs(ToolChain, Inputs, Args, CmdArgs, JA);

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs)) {
    if (ToolChain.ShouldLinkCXXStdlib(Args))
      ToolChain.AddCXXStdlibLibArgs(Args, CmdArgs);

    if (Args.hasArg(options::OPT_pthread))
      CmdArgs.push_back("-lpthread");

    CmdArgs.push_back("-allow-undefined-file");
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("wasm.syms")));
    CmdArgs.push_back("-lc");
    CmdArgs.push_back("-lcompiler_rt");
  }

  if (!Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles))
    CmdArgs.push_back(Args.MakeArgString(ToolChain.GetFilePath("crtn.o")));

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  C.addCommand(llvm::make_unique<Command>(JA, *this, Linker, CmdArgs, Inputs));
}

WebAssembly::WebAssembly(const Driver &D, const llvm::Triple &Triple,
                         const llvm::opt::ArgList &Args)
  : ToolChain(D, Triple, Args) {

  assert(Triple.isArch32Bit() != Triple.isArch64Bit());

  getProgramPaths().push_back(getDriver().getInstalledDir());

  getFilePaths().push_back(getDriver().SysRoot + "/lib");
}

bool WebAssembly::IsMathErrnoDefault() const { return false; }

bool WebAssembly::IsObjCNonFragileABIDefault() const { return true; }

bool WebAssembly::UseObjCMixedDispatch() const { return true; }

bool WebAssembly::isPICDefault() const { return false; }

bool WebAssembly::isPIEDefault() const { return false; }

bool WebAssembly::isPICDefaultForced() const { return false; }

bool WebAssembly::IsIntegratedAssemblerDefault() const { return true; }

// TODO: Support Objective C stuff.
bool WebAssembly::SupportsObjCGC() const { return false; }

bool WebAssembly::hasBlocksRuntime() const { return false; }

// TODO: Support profiling.
bool WebAssembly::SupportsProfiling() const { return false; }

bool WebAssembly::HasNativeLLVMSupport() const { return true; }

void WebAssembly::addClangTargetOptions(const ArgList &DriverArgs,
                                        ArgStringList &CC1Args,
                                        Action::OffloadKind) const {
  if (DriverArgs.hasFlag(clang::driver::options::OPT_fuse_init_array,
                         options::OPT_fno_use_init_array, true))
    CC1Args.push_back("-fuse-init-array");
}

ToolChain::RuntimeLibType WebAssembly::GetDefaultRuntimeLibType() const {
  return ToolChain::RLT_CompilerRT;
}

ToolChain::CXXStdlibType WebAssembly::GetCXXStdlibType(const ArgList &Args) const {
  return ToolChain::CST_Libcxx;
}

void WebAssembly::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                            ArgStringList &CC1Args) const {
  if (!DriverArgs.hasArg(options::OPT_nostdinc))
    addSystemInclude(DriverArgs, CC1Args, getDriver().SysRoot + "/include");
}

void WebAssembly::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                               ArgStringList &CC1Args) const {
  if (!DriverArgs.hasArg(options::OPT_nostdlibinc) &&
      !DriverArgs.hasArg(options::OPT_nostdincxx))
    addSystemInclude(DriverArgs, CC1Args,
                     getDriver().SysRoot + "/include/c++/v1");
}

Tool *WebAssembly::buildLinker() const {
  return new tools::wasm::Linker(*this);
}

void cheerp::Link::ConstructJob(Compilation &C, const JobAction &JA,
                                const InputInfo &Output,
                                const InputInfoList &Inputs,
                                const ArgList &Args,
                                const char *LinkingOutput) const {
  ArgStringList CmdArgs;

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  for (InputInfoList::const_iterator
         it = Inputs.begin(), ie = Inputs.end(); it != ie; ++it) {
    const InputInfo &II = *it;
    if(II.isFilename())
      CmdArgs.push_back(II.getFilename());
  }

  // Add standard libraries
  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nodefaultlibs)) {
    if (C.getDriver().CCCIsCXX()) {
      CmdArgs.push_back(Args.MakeArgString(getToolChain().GetFilePath("libstdlibs.bc")));
    } else {
      CmdArgs.push_back(Args.MakeArgString(getToolChain().GetFilePath("libc.bc")));
      CmdArgs.push_back(Args.MakeArgString(getToolChain().GetFilePath("libm.bc")));
    }
  }

  // Add wasm helper if needed
  Arg *CheerpMode = Args.getLastArg(options::OPT_cheerp_mode_EQ);
  Arg *CheerpLinearOutput = Args.getLastArg(options::OPT_cheerp_linear_output_EQ);
  if((CheerpMode &&
      (CheerpMode->getValue() == StringRef("wasm") ||
       CheerpMode->getValue() == StringRef("wast"))) ||
     (CheerpLinearOutput &&
       (CheerpLinearOutput->getValue() == StringRef("wasm") ||
        CheerpLinearOutput->getValue() == StringRef("wast"))))
  {
    CmdArgs.push_back(Args.MakeArgString(getToolChain().GetFilePath("libwasm.bc")));
  }
 
  // Do not add the same library more than once
  std::set<std::string> usedLibs;
  for (arg_iterator it = Args.filtered_begin(options::OPT_l),
         ie = Args.filtered_end(); it != ie; ++it) {
    std::string libName("lib");
    libName += (*it)->getValue();
    std::string bcLibName = libName + ".bc";
    std::string foundLib = getToolChain().GetFilePath(bcLibName.c_str());
    if (foundLib == bcLibName) {
      // Try again using .a, the internal format is still assumed to be BC
      std::string aLibName = libName + ".a";
      foundLib = getToolChain().GetFilePath(aLibName.c_str());
      if(foundLib == aLibName)
        foundLib = bcLibName;
    }
    if (usedLibs.count(foundLib))
      continue;
    usedLibs.insert(foundLib);
    CmdArgs.push_back(Args.MakeArgString(foundLib));
  }

  const char *Exec = Args.MakeArgString((getToolChain().GetProgramPath("llvm-link")));
  C.addCommand(llvm::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}

void cheerp::CheerpOptimizer::ConstructJob(Compilation &C, const JobAction &JA,
                                          const InputInfo &Output,
                                          const InputInfoList &Inputs,
                                          const ArgList &Args,
                                          const char *LinkingOutput) const {
  ArgStringList CmdArgs;

  CmdArgs.push_back("-march=cheerp");
  if(Args.hasArg(options::OPT_cheerp_preexecute))
    CmdArgs.push_back("-PreExecute");
  if(Args.hasArg(options::OPT_cheerp_preexecute_main))
    CmdArgs.push_back("-cheerp-preexecute-main");
  CmdArgs.push_back("-GlobalDepsAnalyzer");
  if(!Args.hasArg(options::OPT_cheerp_no_type_optimizer))
    CmdArgs.push_back("-TypeOptimizer");
  CmdArgs.push_back("-ReplaceNopCastsAndByteSwaps");
  CmdArgs.push_back("-FreeAndDeleteRemoval");
  // Inlining from -Os may generate additional memcpy calls that we will remove in the backend
  CmdArgs.push_back("-StructMemFuncLowering");
  CmdArgs.push_back("-Os");
  // -Os converts loops to canonical form, which may causes empty forwarding branches, remove those
  CmdArgs.push_back("-simplifycfg");
  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  const InputInfo &II = *Inputs.begin();
  CmdArgs.push_back(II.getFilename());

  // Honor -mllvm
  Args.AddAllArgValues(CmdArgs, options::OPT_mllvm);
  // Honor -cheerp-no-pointer-scev
  if (Arg *CheerpNoPointerSCEV = Args.getLastArg(options::OPT_cheerp_no_pointer_scev))
    CheerpNoPointerSCEV->render(Args, CmdArgs);

  const char *Exec = Args.MakeArgString((getToolChain().GetProgramPath("opt")));
  C.addCommand(llvm::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}

void cheerp::CheerpCompiler::ConstructJob(Compilation &C, const JobAction &JA,
                                          const InputInfo &Output,
                                          const InputInfoList &Inputs,
                                          const ArgList &Args,
                                          const char *LinkingOutput) const {
  const Driver &D = getToolChain().getDriver();
  ArgStringList CmdArgs;

  CmdArgs.push_back("-march=cheerp");

  if(Arg* cheerpAsmJSMemFile = Args.getLastArg(options::OPT_cheerp_asmjs_mem_file_EQ))
  {
    CmdArgs.push_back("-o");
    CmdArgs.push_back(Output.getFilename());

    std::string secondaryFile("-cheerp-secondary-output-file=");
    secondaryFile += cheerpAsmJSMemFile->getValue();
    CmdArgs.push_back(Args.MakeArgString(secondaryFile));

    std::string secondaryPath("-cheerp-secondary-output-path=");
    secondaryPath += cheerpAsmJSMemFile->getValue();
    CmdArgs.push_back(Args.MakeArgString(secondaryPath));
  }
  else if(Arg* cheerpWasmLoader = Args.getLastArg(options::OPT_cheerp_wasm_loader_EQ))
  {
    CmdArgs.push_back("-o");
    CmdArgs.push_back(Args.MakeArgString(cheerpWasmLoader->getValue()));

    std::string secondaryFile("-cheerp-secondary-output-file=");
    secondaryFile += Output.getFilename();
    CmdArgs.push_back(Args.MakeArgString(secondaryFile));
 
    std::string secondaryPath("-cheerp-secondary-output-path=");
     if(Arg* cheerpWasmFile = Args.getLastArg(options::OPT_cheerp_wasm_file_EQ)) {
      secondaryPath.append(llvm::sys::path::filename(cheerpWasmFile->getValue()));
     } else {
      secondaryPath.append(llvm::sys::path::filename(Output.getFilename()));
     }
    CmdArgs.push_back(Args.MakeArgString(secondaryPath));
  }
  else
  {
    CmdArgs.push_back("-o");
    CmdArgs.push_back(Output.getFilename());
    if (Arg* cheerpMode = Args.getLastArg(options::OPT_cheerp_mode_EQ))
    {
      if (cheerpMode->getValue() == StringRef("wasm") || cheerpMode->getValue() == StringRef("wast"))
      {
        CmdArgs.push_back("-cheerp-wasm-only");
      }
    }
  }

  if(Arg* cheerpLinearOutput = Args.getLastArg(options::OPT_cheerp_linear_output_EQ))
    cheerpLinearOutput->render(Args, CmdArgs);
  else if(Arg *CheerpMode = C.getArgs().getLastArg(options::OPT_cheerp_mode_EQ))
  {
    std::string linearOut("-cheerp-linear-output=");
    if (CheerpMode->getValue() == StringRef("wast"))
    {
      linearOut += "wast";
    }
    else if (CheerpMode->getValue() == StringRef("asmjs"))
    {
      linearOut += "asmjs";
    }
    else
    {
      // NOTE: we use "wasm" also for -cheerp-mode=genericjs
      linearOut += "wasm";
    }
    CmdArgs.push_back(Args.MakeArgString(linearOut));
  }

  if(Args.getLastArg(options::OPT_cheerp_make_module)) {
    CmdArgs.push_back("-cheerp-make-module=closure");
  }
  if(Arg* cheerpMakeModuleEq = Args.getLastArg(options::OPT_cheerp_make_module_EQ)) {
    if (cheerpMakeModuleEq->getValue() != StringRef("closure") &&
        cheerpMakeModuleEq->getValue() != StringRef("commonjs")) {
      D.Diag(diag::err_drv_invalid_value)
      << cheerpMakeModuleEq->getAsString(Args) << cheerpMakeModuleEq->getValue();
    }
    cheerpMakeModuleEq->render(Args, CmdArgs);
  }

  if(Arg* cheerpStrictLinkingEq = Args.getLastArg(options::OPT_cheerp_strict_linking_EQ)) {
    if (cheerpStrictLinkingEq->getValue() != StringRef("warning") &&
        cheerpStrictLinkingEq->getValue() != StringRef("error")) {
      D.Diag(diag::err_drv_invalid_value)
      << cheerpStrictLinkingEq->getAsString(Args) << cheerpStrictLinkingEq->getValue();
    }
    cheerpStrictLinkingEq->render(Args, CmdArgs);
  }

  if(Arg* cheerpSourceMap = Args.getLastArg(options::OPT_cheerp_sourcemap_EQ))
    cheerpSourceMap->render(Args, CmdArgs);
  if(Arg* cheerpSourceMapPrefix = Args.getLastArg(options::OPT_cheerp_sourcemap_prefix_EQ))
    cheerpSourceMapPrefix->render(Args, CmdArgs);
  if(Arg* cheerpSourceMapStandAlone = Args.getLastArg(options::OPT_cheerp_sourcemap_standalone))
    cheerpSourceMapStandAlone->render(Args, CmdArgs);
  if(Arg* cheerpPrettyCode = Args.getLastArg(options::OPT_cheerp_pretty_code))
    cheerpPrettyCode->render(Args, CmdArgs);
  if(Arg* cheerpAsmJSSymbolicGlobals = Args.getLastArg(options::OPT_cheerp_asmjs_symbolic_globals))
    cheerpAsmJSSymbolicGlobals->render(Args, CmdArgs);
  if(Arg* cheerpNoNativeMath = Args.getLastArg(options::OPT_cheerp_no_native_math))
    cheerpNoNativeMath->render(Args, CmdArgs);
  if(Arg* cheerpNoMathImul = Args.getLastArg(options::OPT_cheerp_no_math_imul))
    cheerpNoMathImul->render(Args, CmdArgs);
  if(Arg* cheerpNoMathFround = Args.getLastArg(options::OPT_cheerp_no_math_fround))
    cheerpNoMathFround->render(Args, CmdArgs);
  if(Arg* cheerpNoCredits = Args.getLastArg(options::OPT_cheerp_no_credits))
    cheerpNoCredits->render(Args, CmdArgs);
  if(Arg* cheerpForceTypedArrays = Args.getLastArg(options::OPT_cheerp_force_typed_arrays))
    cheerpForceTypedArrays->render(Args, CmdArgs);
  if(Arg* cheerpReservedNames = Args.getLastArg(options::OPT_cheerp_reserved_names_EQ))
    cheerpReservedNames->render(Args, CmdArgs);
  if(Arg* cheerpGlobalPrefix = Args.getLastArg(options::OPT_cheerp_global_prefix_EQ))
    cheerpGlobalPrefix->render(Args, CmdArgs);
  if(Arg *cheerpHeapSize = Args.getLastArg(options::OPT_cheerp_linear_heap_size))
    cheerpHeapSize->render(Args, CmdArgs);
  if(Arg *cheerpStackSize = Args.getLastArg(options::OPT_cheerp_linear_stack_size))
    cheerpStackSize->render(Args, CmdArgs);
  if(Arg* cheerpNoICF = Args.getLastArg(options::OPT_cheerp_no_icf))
    cheerpNoICF->render(Args, CmdArgs);
  if(Arg* cheerpBoundsCheck = Args.getLastArg(options::OPT_cheerp_bounds_check))
    cheerpBoundsCheck->render(Args, CmdArgs);
  if(Arg* cheerpCfgLegacy = Args.getLastArg(options::OPT_cheerp_cfg_legacy))
    cheerpCfgLegacy->render(Args, CmdArgs);
  if(Arg* cheerpRegisterizeLegacy = Args.getLastArg(options::OPT_cheerp_registerize_legacy))
    cheerpRegisterizeLegacy->render(Args, CmdArgs);
  if(Arg* cheerpAvoidWasmTraps = Args.getLastArg(options::OPT_cheerp_avoid_wasm_traps))
    cheerpAvoidWasmTraps->render(Args, CmdArgs);
  if(Arg* cheerpFixFuncCasts = Args.getLastArg(options::OPT_cheerp_fix_wrong_func_casts))
    cheerpFixFuncCasts->render(Args, CmdArgs);

  // Set output to binary mode to avoid linefeed conversion on Windows.
  CmdArgs.push_back("-filetype");
  CmdArgs.push_back("obj");

  const InputInfo &II = *Inputs.begin();
  CmdArgs.push_back(II.getFilename());

  // Honor -mllvm
  Args.AddAllArgValues(CmdArgs, options::OPT_mllvm);

  const char *Exec = Args.MakeArgString((getToolChain().GetProgramPath("llc")));
  C.addCommand(llvm::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}
