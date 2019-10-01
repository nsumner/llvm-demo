
#include "llvm/ADT/Triple.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/CodeGen/CommandFlags.inc"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"

#include <memory>
#include <string>

#include "DynamicCallCounter.h"
#include "StaticCallCounter.h"

#include "config.h"

using namespace llvm;
using std::string;
using std::unique_ptr;
using std::vector;
using llvm::sys::ExecuteAndWait;
using llvm::sys::findProgramByName;
using llvm::legacy::PassManager;


enum class AnalysisType {
  STATIC,
  DYNAMIC,
};


static cl::OptionCategory callCounterCategory{"call counter options"};

static cl::opt<string> inPath{cl::Positional,
                              cl::desc{"<Module to analyze>"},
                              cl::value_desc{"bitcode filename"},
                              cl::init(""),
                              cl::Required,
                              cl::cat{callCounterCategory}};

static cl::opt<AnalysisType> analysisType{
    cl::desc{"Select analyis type:"},
    cl::values(clEnumValN(AnalysisType::STATIC,
                          "static",
                          "Count static direct calls."),
               clEnumValN(AnalysisType::DYNAMIC,
                          "dynamic",
                          "Count dynamic direct calls.")
               ),
    cl::Required,
    cl::cat{callCounterCategory}};

static cl::opt<string> outFile{"o",
                               cl::desc{"Filename of the instrumented program"},
                               cl::value_desc{"filename"},
                               cl::init(""),
                               cl::cat{callCounterCategory}};

static cl::opt<char> optLevel{
    "O",
    cl::desc{"Optimization level. [-O0, -O1, -O2, or -O3] (default = '-O2')"},
    cl::Prefix,
    cl::ZeroOrMore,
    cl::init('2'),
    cl::cat{callCounterCategory}};

static cl::list<string> libPaths{"L",
                                 cl::Prefix,
                                 cl::desc{"Specify a library search path"},
                                 cl::value_desc{"directory"},
                                 cl::cat{callCounterCategory}};

static cl::list<string> libraries{"l",
                                  cl::Prefix,
                                  cl::desc{"Specify libraries to link against"},
                                  cl::value_desc{"library prefix"},
                                  cl::cat{callCounterCategory}};


static void
compile(Module& m, StringRef outputPath) {
  string err;

  Triple triple        = Triple(m.getTargetTriple());
  Target const* target = TargetRegistry::lookupTarget(MArch, triple, err);
  if (!target) {
    report_fatal_error("Unable to find target:\n " + err);
  }

  CodeGenOpt::Level level = CodeGenOpt::Default;
  switch (optLevel) {
    default:
      report_fatal_error("Invalid optimization level.\n");
    // No fall through
    case '0': level = CodeGenOpt::None; break;
    case '1': level = CodeGenOpt::Less; break;
    case '2': level = CodeGenOpt::Default; break;
    case '3': level = CodeGenOpt::Aggressive; break;
  }

  string FeaturesStr;
  TargetOptions options = InitTargetOptionsFromCodeGenFlags();
  unique_ptr<TargetMachine> machine(
      target->createTargetMachine(triple.getTriple(),
                                  MCPU,
                                  FeaturesStr,
                                  options,
                                  getRelocModel(),
                                  CMModel.getValue(),
                                  level));
  assert(machine && "Could not allocate target machine!");

  if (FloatABIForCalls != FloatABI::Default) {
    options.FloatABIType = FloatABIForCalls;
  }

  std::error_code errc;
  auto out =
      std::make_unique<ToolOutputFile>(outputPath, errc, sys::fs::F_None);
  if (!out) {
    report_fatal_error("Unable to create file:\n " + errc.message());
  }

  // Build up all of the passes that we want to do to the module.
  legacy::PassManager pm;

  // Add target specific info and transforms
  TargetLibraryInfoImpl tlii(triple);
  pm.add(new TargetLibraryInfoWrapperPass(tlii));

  m.setDataLayout(machine->createDataLayout());

  {  // Bound this scope
    raw_pwrite_stream* os(&out->os());

    FileType = TargetMachine::CGFT_ObjectFile;
    std::unique_ptr<buffer_ostream> bos;
    if (!out->os().supportsSeeking()) {
      bos = std::make_unique<buffer_ostream>(*os);
      os  = bos.get();
    }

    // Ask the target to add backend passes as necessary.
    if (machine->addPassesToEmitFile(pm, *os, nullptr, FileType)) {
      report_fatal_error("target does not support generation "
                         "of this file type!\n");
    }

    // Before executing passes, print the final values of the LLVM options.
    cl::PrintOptionValues();

    pm.run(m);
  }

  // Keep the output binary if we've been successful to this point.
  out->keep();
}


static void
link(StringRef objectFile, StringRef outputFile) {
  auto clang = findProgramByName("clang++");
  string opt("-O");
  opt += optLevel;

  if (!clang) {
    report_fatal_error("Unable to find clang.");
  }
  vector<string> args{clang.get(), opt, "-o", outputFile, objectFile};

  for (auto& libPath : libPaths) {
    args.push_back("-L" + libPath);
  }

  for (auto& library : libraries) {
    args.push_back("-l" + library);
  }

  vector<llvm::StringRef> charArgs;
  charArgs.reserve(args.size());
  for (auto& arg : args) {
    charArgs.emplace_back(arg);
  }

  for (auto& arg : args) {
    outs() << arg.c_str() << " ";
  }
  outs() << "\n";

  string err;
  auto result = ExecuteAndWait(
      clang.get(),
      llvm::makeArrayRef(charArgs),
      llvm::NoneType::None,
      {},
      0,
      0,
      &err
    );
  if (-1 == result) {
    report_fatal_error("Unable to link output file.");
  }
}


static void
generateBinary(Module& m, StringRef outputFilename) {
  // Compiling to native should allow things to keep working even when the
  // version of clang on the system and the version of LLVM used to compile
  // the tool don't quite match up.
  string objectFile = outputFilename.str() + ".o";
  compile(m, objectFile);
  link(objectFile, outputFilename);
}


static void
saveModule(Module const& m, StringRef filename) {
  std::error_code errc;
  raw_fd_ostream out(filename.data(), errc, sys::fs::F_None);

  if (errc) {
    report_fatal_error("error saving llvm module to '" + filename + "': \n"
                       + errc.message());
  }
  WriteBitcodeToFile(m, out);
}


static void
prepareLinkingPaths(SmallString<32> invocationPath) {
  // First search the directory of the binary for the library, in case it is
  // all bundled together.
  sys::path::remove_filename(invocationPath);
  if (!invocationPath.empty()) {
    libPaths.push_back(invocationPath.str());
  }
// If the builder doesn't plan on installing it, we still need to get to the
// runtime library somehow, so just build in the path to the temporary one.
#ifdef CMAKE_INSTALL_PREFIX
  libPaths.push_back(CMAKE_INSTALL_PREFIX "/lib");
#elif defined(CMAKE_TEMP_LIBRARY_PATH)
  libPaths.push_back(CMAKE_TEMP_LIBRARY_PATH);
#elif defined(TEMP_LIBRARY_PATH)
  // This is a bit of a hack
  libPaths.push_back(TEMP_LIBRARY_PATH "/Debug+Asserts/lib/");
  libPaths.push_back(TEMP_LIBRARY_PATH "/Release+Asserts/lib/");
  libPaths.push_back(TEMP_LIBRARY_PATH "/Debug/lib/");
  libPaths.push_back(TEMP_LIBRARY_PATH "/Release/lib/");
#endif
  libraries.push_back(RUNTIME_LIB);
  libraries.push_back("rt");
}


static void
instrumentForDynamicCount(Module& m) {
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  if (outFile.getValue().empty()) {
    errs() << "-o command line option must be specified.\n";
    exit(-1);
  }

  // Build up all of the passes that we want to run on the module.
  legacy::PassManager pm;
  pm.add(new callcounter::DynamicCallCounter());
  pm.add(createVerifierPass());
  pm.run(m);

  generateBinary(m, outFile);
  saveModule(m, outFile + ".callcounter.bc");
}


struct StaticCountPrinter : public ModulePass {
  static char ID;
  raw_ostream& out;

  explicit StaticCountPrinter(raw_ostream& out) : ModulePass(ID), out(out) {}

  bool
  runOnModule(Module& m) override {
    getAnalysis<callcounter::StaticCallCounter>().print(out, &m);
    return false;
  }

  void
  getAnalysisUsage(AnalysisUsage& au) const override {
    au.addRequired<callcounter::StaticCallCounter>();
    au.setPreservesAll();
  }
};

char StaticCountPrinter::ID = 0;


static void
countStaticCalls(Module& m) {
  // Build up all of the passes that we want to run on the module.
  legacy::PassManager pm;
  pm.add(new callcounter::StaticCallCounter());
  pm.add(new StaticCountPrinter(outs()));
  pm.run(m);
}


int
main(int argc, char** argv) {
  // This boilerplate provides convenient stack traces and clean LLVM exit
  // handling. It also initializes the built in support for convenient
  // command line option handling.
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm_shutdown_obj shutdown;
  cl::HideUnrelatedOptions(callCounterCategory);
  cl::ParseCommandLineOptions(argc, argv);

  // Construct an IR file from the filename passed on the command line.
  SMDiagnostic err;
  LLVMContext context;
  unique_ptr<Module> module = parseIRFile(inPath.getValue(), err, context);

  if (!module.get()) {
    errs() << "Error reading bitcode file: " << inPath << "\n";
    err.print(argv[0], errs());
    return -1;
  }

  if (AnalysisType::DYNAMIC == analysisType) {
    prepareLinkingPaths(StringRef(argv[0]));
    instrumentForDynamicCount(*module);
  } else {
    countStaticCalls(*module);
  }

  return 0;
}
