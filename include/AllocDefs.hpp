#ifndef ALLOCDEFS_HPP
#define ALLOCDEFS_HPP

#include "llvm/IR/Function.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace llvm;

typedef struct _AllocDefinition {
    string name;
    int sizeIdx;
    int numIdx;
    int addrIdx;
} AllocDefinition;

class AllocDefManager {
    vector<AllocDefinition> allocDefs;
    string getAllocInPath();
    public:
        void loadAllocDefs();
        AllocDefinition *getAllocDef(Function *f);
};

#endif
