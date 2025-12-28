

#ifndef STATICCALLCOUNTER_H
#define STATICCALLCOUNTER_H


#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"


namespace callcounter {


struct StaticCallCounts {
  llvm::DenseMap<llvm::Function*, uint64_t> counts;

  void analyze(const llvm::Module& m);

  void print(llvm::raw_ostream& out) const;

  void handleInstruction(const llvm::CallBase& cb);
};


struct StaticCallCounter : public llvm::AnalysisInfoMixin<StaticCallCounter> {
  using Result = StaticCallCounts;

  StaticCallCounter() {}

  StaticCallCounts run(const llvm::Module& m, llvm::ModuleAnalysisManager& mam);

  static llvm::AnalysisKey Key;
};


}  // namespace callcounter


#endif
