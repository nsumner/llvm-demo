#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include <stdint.h>
#include <stdio.h>


void logInit(int functionId) {
}

void logExit(int functionId) {
}

void logFnBegin(int functionId) {
}

void logFnEnd(int functionId) {
}

void logAlloc(void *addr, uint64_t size, uint64_t num, int type, int file, int line, int col) {
}

void logAccessPtr(void *ptr, void *value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessStaticString(void *ptr, void *value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessI8(void *ptr, uint8_t value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessI16(void *ptr, uint16_t value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessI32(void *ptr, uint32_t value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessI64(void *ptr, uint64_t value, int type, int file, int line, int col, int typeId, int varId) {
}

/* =============================
 These don't exist: */

void logAccessF8(void *ptr, uint8_t value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessF16(void *ptr, uint16_t value, int type, int file, int line, int col, int typeId, int varId) {
}

/* ============================= */

void logAccessF32(void *ptr, float value, int type, int file, int line, int col, int typeId, int varId) {
}

void logAccessF64(void *ptr, double value, int type, int file, int line, int col, int typeId, int varId) {
}
#endif
