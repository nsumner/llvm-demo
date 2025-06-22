add_library(callcounter-rt)
target_sources(callcounter-rt
  PRIVATE
    runtime.cpp
)
set_target_properties(callcounter-rt PROPERTIES
  LINKER_LANGUAGE CXX
  POSITION_INDEPENDENT_CODE ON
)
