

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"

#include "StaticCallCounter.h"

// This program can be used by opt. After compilation, use:
// opt -load buildpath/lib/callcounter-inst.so --callcounter -analyze bitcode.bc


using namespace llvm;
using callcounter::StaticCallCounter;


namespace callcounter {

char StaticCallCounter::ID = 0;

RegisterPass<StaticCallCounter> X("callcounter",
                                  "Print the static count of direct calls");

}


// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. The actual output, however, is produced separately.
bool
StaticCallCounter::runOnModule(Module& m) {
  for (auto& f : m) {
    for (auto& bb : f) {
      for (auto& i : bb) {
        handleInstruction(CallSite(&i));
      }
    }
  }

  return false;
}


void
StaticCallCounter::handleInstruction(CallSite cs) {
  // Check whether the instruction is actually a call
  if (!cs.getInstruction()) {
    return;
  }

  // Check whether the called function is directly invoked
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
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


// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
void
StaticCallCounter::print(raw_ostream& out, Module const* m) const {
  out << "Function Counts\n"
      << "===============\n";
  for (auto& kvPair : counts) {
    auto* function = kvPair.first;
    uint64_t count = kvPair.second;
    out << function->getName() << " : " << count << "\n";
  }
}
