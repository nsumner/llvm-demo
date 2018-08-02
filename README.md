These programs are demonstrations of how LLVM can be used for (very simple)
static and dynamic analyses. However, they use many of the initial building
blocks necessary for solving more complex problems.

The provided analyses count the number of direct invocations of a particular
function are present within a program or within and execution depending on
whether static or dynamic analysis is used.

Building with CMake
==============================================
1. Clone the demo repository.

        git clone https://github.com/nsumner/llvm-demo.git

2. Create a new directory for building.

        mkdir demobuild

3. Change into the new directory.

        cd demobuild

4. Run CMake with the path to the LLVM source.

        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
            -DLLVM_DIR=</path/to/LLVM/build>/lib/cmake/llvm/ ../llvm-demo

5. Run make inside the build directory:

        make

This produces a callcounter tool called bin/callcounter and supporting
libraries in lib/.

Note, building with a tool like ninja can be done by adding `-G Ninja` to
the cmake invocation and running ninja instead of make.

Running
==============================================

First suppose that you have a program compiled to bitcode:

    clang -g -c -emit-llvm ../llvm-demo/test/simpletest.c -o calls.bc

Running the dynamic call printer:

    bin/callcounter -dynamic calls.bc -o calls
    ./calls

Running the static call printer:

    bin/callcounter -static calls.bc

or by loading the pass as a plugin for `opt`:

    opt -analyze -load lib/libcallcounter-lib.so -callcounter calls.bc
