add_library(callcounter-inst
  StaticCallCounter.cpp
  DynamicCallCounter.cpp
  )

add_llvm_loadable_module(callcounter-lib
  StaticCallCounter.cpp
  DynamicCallCounter.cpp
  )
