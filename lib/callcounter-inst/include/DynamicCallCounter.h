

#ifndef DYNAMICCALLCOUNTER_H
#define DYNAMICCALLCOUNTER_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


namespace callcounter {


struct DynamicCallCounter : public llvm::PassInfoMixin<DynamicCallCounter> {

  llvm::DenseMap<llvm::Function*, uint64_t> ids;
  llvm::DenseSet<llvm::Function*> internal;

  DynamicCallCounter() {}

  llvm::PreservedAnalyses run(llvm::Module& M, llvm::ModuleAnalysisManager& mam);

  void handleCalledFunction(llvm::Function& f, llvm::FunctionCallee counter);
  void handleInstruction(llvm::CallBase& cb, llvm::FunctionCallee counter);
};


}  // namespace callcounter


#endif
