#ifndef UTIL_HPP
#define UTIL_HPP

#include "llvm/Pass.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <map>
#include <vector>
#include <set>
#include <iostream>

#include <cxxabi.h>

using namespace std;
using namespace llvm;

std::string demangle(const char* name);

void printType(llvm::Type *t);

size_t getFunctionLOCSize(llvm::Function &f);

bool checkFunctionLoops(Function &f);

size_t longestPathSize(Function &f);

#endif
