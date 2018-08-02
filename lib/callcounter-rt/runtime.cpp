
#include <cstdint>
#include <cstdio>


extern "C" {


// This macro allows us to prefix strings so that they are less likely to
// conflict with existing symbol names in the examined programs.
// e.g. CCOUNT(entry) yields CaLlCoUnTeR_entry
#define CCOUNT(X) CaLlCoUnTeR_##X

// The count of the number of functions is stored in a global variable inside
// the instrumented module.
extern uint64_t CCOUNT(numFunctions);

// An array of information for each function ID is stored within the
// instrumented module.
extern struct {
  char* name;
  uint64_t count;
} CCOUNT(functionInfo)[];


void
CCOUNT(called)(uint64_t id) {
  ++CCOUNT(functionInfo)[id].count;
}


void
CCOUNT(print)() {
  printf("=====================\n"
         "Direct Function Calls\n"
         "=====================\n");
  for (size_t id = 0; id < CCOUNT(numFunctions); ++id) {
    auto& info = CCOUNT(functionInfo)[id];
    printf("%s: %lu\n", info.name, info.count);
  }
}
}
