

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"

#include "StaticCallCounter.h"


using namespace llvm;
using callcounter::StaticCallCounts;
using callcounter::StaticCallCounter;


namespace callcounter {

AnalysisKey StaticCallCounter::Key;

}


void
StaticCallCounts::analyze(const Module& m) {
  for (auto& f : m) {
    for (auto& bb : f) {
      for (auto& i : bb) {
        if (const auto* cb = dyn_cast<const CallBase>(&i)) {
          handleInstruction(*cb);
        }
      }
    }
  }
}


void
StaticCallCounts::handleInstruction(const CallBase& cb) {
  // Check whether the called function is directly invoked
  auto called = dyn_cast<Function>(cb.getCalledOperand()->stripPointerCasts());
  if (!called) {
    return;
  }

  // Update the count for the particular call
  auto count = counts.find(called);
  if (counts.end() == count) {
    count = counts.insert(std::make_pair(called, 0)).first;
  }
  ++count->second;
}


void
StaticCallCounts::print(raw_ostream& out) const {
  out << "Function Counts\n"
      << "===============\n";
  for (auto& kvPair : counts) {
    auto* function = kvPair.first;
    uint64_t count = kvPair.second;
    out << function->getName() << " : " << count << "\n";
  }
}


StaticCallCounts
StaticCallCounter::run(const Module& m, ModuleAnalysisManager& mam) {
  StaticCallCounts counts;
  counts.analyze(m);
  return counts;
}
