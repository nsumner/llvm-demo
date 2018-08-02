#ifndef LOGFUNCTIONMANAGER_HPP
#define LOGFUNCTIONMANAGER_HPP

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

#include <string>
#include <iostream>

enum value_types {
    FLOAT = 0, INTEGER, VALUE_TYPES_MAX
};

enum value_sizes {
    S8 = 0, S16, S32, S64, VALUE_SIZES_MAX
};

using namespace llvm;
using namespace std;

class LogFunctionManager {
    private:
        string getInstrumentationLibPath();
        Function *loadExternalFunction(Module *m, Module *extM, const char *name);

    public:
        Function *logFunctions[VALUE_TYPES_MAX][VALUE_SIZES_MAX];
        Function *ptrLogFunc; 
        Function *stringLogFunc; 
        Function *allocLogFunc; 
        Function *fnBeginLogFunc; 
        Function *fnEndLogFunc; 
        Function *initLogFunc; 
        Function *exitLogFunc; 

        int getSizeIndex(int size);
        void loadFunctions(Module *m);
        Function *getLogFunction(Value *v, Function *parent);
        bool isLogFunction(Function *f);
};

#endif
