set(LLVM_USED_LIBS callcounter-inst)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake" 
               "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(callcounter
  main.cpp
  )

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${LLVM_TARGETS_TO_BUILD}
        asmparser core linker bitreader bitwriter irreader ipo scalaropts
        analysis target mc support)

target_link_libraries(callcounter callcounter-inst ${REQ_LLVM_LIBRARIES})

# Platform dependencies.
if( WIN32 )
  target_link_libraries(callcounter
    shlwapi
  )
else()
  target_link_libraries(callcounter
    pthread
    dl
    curses
  )
endif()

set_target_properties(callcounter
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX "")

install(TARGETS callcounter
  RUNTIME DESTINATION bin)

