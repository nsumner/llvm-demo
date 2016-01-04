add_library(callcounter-inst
  StaticCallCounter.cpp
  DynamicCallCounter.cpp
)

add_library(callcounter-lib MODULE
  StaticCallCounter.cpp
  DynamicCallCounter.cpp
)
