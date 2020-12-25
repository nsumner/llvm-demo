
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake" 
               "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY
)
add_executable(callcounter
  main.cpp
)
target_include_directories(callcounter
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)


llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES
  ${LLVM_TARGETS_TO_BUILD}
  asmparser linker bitreader bitwriter irreader
  target mc support
)

target_link_libraries(callcounter
  PRIVATE
    callcounter-inst
    ${REQ_LLVM_LIBRARIES}
)

# Platform dependencies.
if( WIN32 )
  find_library(SHLWAPI_LIBRARY shlwapi)
  target_link_libraries(callcounter
    PRIVATE
      ${SHLWAPI_LIBRARY}
  )
else()
  find_package(Threads REQUIRED)
  find_package(Curses REQUIRED)
  target_link_libraries(callcounter
    PRIVATE
      ${CMAKE_THREAD_LIBS_INIT}
      ${CMAKE_DL_LIBS}
      ${CURSES_LIBRARIES}
  )
endif()

set_target_properties(callcounter PROPERTIES
  LINKER_LANGUAGE CXX
  PREFIX ""
  CXX_STANDARD 17
)

install(TARGETS callcounter
  RUNTIME DESTINATION bin
)

