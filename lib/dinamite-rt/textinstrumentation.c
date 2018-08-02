#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include <stdint.h>
#include <stdio.h>

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

const char *filename = "access.trace";
static FILE *out = NULL;

#define OPEN_LOG() if (unlikely(out == NULL)) { printf("Opening log file %s...\n", filename); out = fopen(filename,"w"); }
void logInit(int functionId) {
    OPEN_LOG();
}

void logExit(int functionId) {
    if (out != NULL) {
        printf("Closing log file %s...\n", filename);
        fclose(out);
        out = NULL;
    }
}

void logFnBegin(int functionId) {
    fprintf(out, "fb %d\n", functionId);
    fflush(out);
}

void logFnEnd(int functionId) {
    fprintf(out, "fe %d\n", functionId);
    fflush(out);
}

void logAlloc(void *addr, uint64_t size, uint64_t num, int type, int file, int line, int col) {
    fprintf(out, "%p %llu %llu %d %d %d %d\n", addr, size, num, type, file, line, col);
    fflush(out);
}

void logAccessPtr(void *ptr, void *value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %p %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessStaticString(void *ptr, void *value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %s %c %d %d %d %d %d\n", (char*)ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessI8(void *ptr, uint8_t value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %u %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessI16(void *ptr, uint16_t value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %hu %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessI32(void *ptr, uint32_t value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %u %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessI64(void *ptr, uint64_t value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

/* =============================
 These don't exist: */

void logAccessF8(void *ptr, uint8_t value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %c %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessF16(void *ptr, uint16_t value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %hu %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

/* ============================= */

void logAccessF32(void *ptr, float value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %f %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}

void logAccessF64(void *ptr, double value, int type, int file, int line, int col, int typeId, int varId) {
	fprintf(out, "%p %lf %c %d %d %d %d %d\n", ptr, value, type, file, line, col, typeId, varId);
    fflush(out);

	/*fprintf(out, "%p %llu %c %d %d %d %d %d\n", ptr,[> value, <]type, file, line, col, typeId, varId);*/
}
#endif
