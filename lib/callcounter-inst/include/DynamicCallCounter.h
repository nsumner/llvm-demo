

#ifndef DYNAMICCALLCOUNTER_H
#define DYNAMICCALLCOUNTER_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


namespace callcounter {


struct DynamicCallCounter : public llvm::ModulePass {
  static char ID;

  llvm::DenseMap<llvm::Function*, uint64_t> ids;
  llvm::DenseSet<llvm::Function*> internal;

  DynamicCallCounter() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module& m) override;

  void handleCalledFunction(llvm::Function& f, llvm::FunctionCallee counter);
  void handleInstruction(llvm::CallBase& cb, llvm::FunctionCallee counter);
};


}  // namespace callcounter


#endif
