

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "DynamicCallCounter.h"


using namespace llvm;
using callcounter::DynamicCallCounter;


namespace callcounter {

char DynamicCallCounter::ID = 0;

}


// Returns a map (Function* -> uint64_t).
DenseMap<Function*,uint64_t>
computeFunctionIDs(std::vector<Function*> &functions) {
  DenseMap<Function*,uint64_t> idMap;

  size_t nextID = 0;
  for (auto f : functions) {
    idMap[f] = nextID;
    ++nextID;
  }

  return idMap;
}


// Returns a set of all internal (defined) functions.
DenseSet<Function*>
computeInternal(std::vector<Function*> &functions) {
  DenseSet<Function*> internal;

  for (auto f : functions) {
    if (!f->isDeclaration()) {
      internal.insert(f);
    }
  }

  return internal;
}


// Create the CCOUNT(functionInfo) table used by the runtime library.
void
createFunctionTable(Module &m, uint64_t numFunctions) {
  auto &context = m.getContext();

  // Create the component types of the table
  auto *int64Ty    = Type::getInt64Ty(context);
  auto *stringTy   = Type::getInt8PtrTy(context);
  Type *fieldTys[] = {stringTy, int64Ty};
  auto *structTy   = StructType::get(context, fieldTys, false);
  auto *tableTy    = ArrayType::get(structTy, numFunctions);

  auto *int8Ty = Type::getInt8Ty(context);
  auto *zero   = ConstantInt::get(int64Ty, 0, false);
  Value *gepIndices[] = {zero, zero};

  // Compute and store an externally visible array of function information.
  std::vector<Constant*> values;
  for (auto &f : m) {
     auto *name    = ConstantDataArray::getString(context, f.getName(), true);
     auto *arrayTy = ArrayType::get(int8Ty, f.getName().size() + 1);
     auto *nameVar =
       new GlobalVariable(m, arrayTy, true, GlobalValue::PrivateLinkage, name);
     Constant *structFields[] =
       { ConstantExpr::getGetElementPtr(arrayTy, nameVar, gepIndices), zero };
     values.push_back(ConstantStruct::get(structTy, structFields));
  }
  auto *functionTable = ConstantArray::get(tableTy, values);
  new GlobalVariable(m, tableTy, false, GlobalValue::ExternalLinkage,
                     functionTable, "CaLlCoUnTeR_functionInfo");
}


// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
DynamicCallCounter::runOnModule(Module &m) {
  auto &context = m.getContext();

  // First identify the functions we wish to track
  std::vector<Function*> toCount;
  for (auto &f : m) {
    toCount.push_back(&f);
  }

  ids = computeFunctionIDs(toCount);
  internal = computeInternal(toCount);
  auto const numFunctions = toCount.size();

  // Store the number of functions into an externally visible variable.
  auto *int64Ty = Type::getInt64Ty(context);
  auto *numFunctionsGlobal = ConstantInt::get(int64Ty, numFunctions, false);
  new GlobalVariable(m, int64Ty, true, GlobalValue::ExternalLinkage,
                     numFunctionsGlobal, "CaLlCoUnTeR_numFunctions");

  createFunctionTable(m, numFunctions);

  // Install the result printing function so that it prints out the counts after
  // the entire program is finished executing.
  auto *voidTy  = Type::getVoidTy(context);
  auto *printer = m.getOrInsertFunction("CaLlCoUnTeR_print", voidTy, nullptr);
  appendToGlobalDtors(m, llvm::cast<Function>(printer), 0);

  // Declare the counter function
  auto *helperTy = FunctionType::get(voidTy, int64Ty, false);
  auto *counter  = m.getOrInsertFunction("CaLlCoUnTeR_called", helperTy);

  for (auto f : toCount) {
    // We only want to instrument internally defined functions.
    if (f->isDeclaration()) {
      continue;
    }

    // Count each internal function as it executes.
    handleCalledFunction(*f, counter);

    // Count each external function as it is called.
    for (auto &bb : *f) {
      for (auto &i : bb) {
        handleInstruction(CallSite(&i), counter);
      }
    }
  }

  return true;
}


void
DynamicCallCounter::handleCalledFunction(Function &f, Value *counter) {
  IRBuilder<> builder(f.getEntryBlock().getFirstInsertionPt());
  builder.CreateCall(counter, builder.getInt64(ids[&f]));
}


void
DynamicCallCounter::handleInstruction(CallSite cs, Value *counter) {
  // Check whether the instruction is actually a call
  if (!cs.getInstruction()) {
    return;
  }

  // Check whether the called function is directly invoked
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (!called) {
    return;
  }

  // Check if the function is internal or blacklisted.
  if (internal.count(called) || !ids.count(called)) {
    // Internal functions are counted upon the entry of each function body.
    // Blacklisted functions are not counted. Neither should proceed.
    return;
  }

  // External functions are counted at their invocation sites.
  IRBuilder<> builder(cs.getInstruction());
  builder.CreateCall(counter, builder.getInt64(ids[called]));
}

