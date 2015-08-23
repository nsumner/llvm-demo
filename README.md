These programs are demonstrations of how LLVM can be used for (very simple)
static and dynamic analyses. However, they use many of the initial building
blocks necessary for solving more complex problems.

The provided analyses count the number of direct invocations of a particular
function are present within a program or within and execution depending on
whether static or dynamic analysis is used.

These instructions assume that your current directory starts out as the "demo"
directory within the package.


Building with CMake
==============================================

1. Create a new directory for building.

        mkdir ../callcounterbuild

2. Change into the new directory.

        cd ../callcounterbuild

3. Run CMake with the path to the LLVM source. For LLVM 3.7
  and later, LLVM can be built with configure even if the demo is built
  with CMake.

        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
            -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
            -DLLVM_DIR=</path/to/LLVM/build>/share/llvm/cmake/ ../demo

4. Run make inside the build directory:

        make

This produces a callcounter tool called bin/callcounter and supporting
libraries in lib/.

Note, building with a tool like ninja can be done by adding '-G Ninja' to
the cmake invocation and running ninja instead of make.

Running
==============================================

First suppose that you have a program compiled to bitcode:

    clang -g -c -emit-llvm ../demo/test/simpletest.c -o calls.bc

Running the dynamic call printer:

    bin/callcounter -dynamic calls.bc -o calls
    ./calls

Running the static call printer:

    bin/callcounter -static calls.bc
