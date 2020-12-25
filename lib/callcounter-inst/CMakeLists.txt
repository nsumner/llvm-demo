
llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES
  core analysis support
)

add_library(callcounter-inst
  StaticCallCounter.cpp
  DynamicCallCounter.cpp
)
target_link_libraries(callcounter-inst
  INTERFACE
    ${REQ_LLVM_LIBRARIES}
)
target_include_directories(callcounter-inst
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(callcounter-inst PROPERTIES
  LINKER_LANGUAGE CXX
  CXX_STANDARD 17
)

add_library(callcounter-lib MODULE
  StaticCallCounter.cpp
  DynamicCallCounter.cpp
)
target_link_libraries(callcounter-lib
  INTERFACE
    ${REQ_LLVM_LIBRARIES}
)
target_include_directories(callcounter-lib
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(callcounter-lib PROPERTIES
  LINKER_LANGUAGE CXX
  CXX_STANDARD 17
)
