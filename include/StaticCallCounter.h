

#ifndef STATICCALLCOUNTER_H
#define STATICCALLCOUNTER_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


namespace callcounter {


struct StaticCallCounter : public llvm::ModulePass {

  static char ID;

  llvm::DenseMap<llvm::Function*, uint64_t> counts;

  StaticCallCounter()
    : llvm::ModulePass(ID)
      { }

  virtual bool runOnModule(llvm::Module &m) override;

  virtual void print(llvm::raw_ostream &out,
                     llvm::Module const *m) const override;

  void handleInstruction(llvm::CallSite cs);
};


}


#endif

