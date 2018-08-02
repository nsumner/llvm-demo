#ifndef INSTRUMENTATION_H
#define INSTRUMENTATION_H

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "dinamite/binaryinstrumentation.h"
#include "dinamite/dinamite_time.h"

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define MAX_THREADS 128

static FILE *out[MAX_THREADS];
static logentry *entries[MAX_THREADS];
static int current[MAX_THREADS];

#define BUFFER_SIZE 4 * 4096

static pthread_once_t dinamite_once_control = PTHREAD_ONCE_INIT;
pthread_key_t tls_key;
static pthread_mutex_t id_mtx = PTHREAD_MUTEX_INITIALIZER;
static int next_id = 0;

#define DINAMITE_VERBOSE


static bool __dinamite_exclude_tid(pid_t tid) {

	char *excluded_tids, *token, *orig;

	excluded_tids = malloc(MAX_THREADS);
	if (excluded_tids == NULL)
		return false;

	orig = excluded_tids;
	if (getenv("DINAMITE_EXCLUDE_TID") == NULL) { 
        return false;
    }
	strncpy(excluded_tids, getenv("DINAMITE_EXCLUDE_TID"),
		MAX_THREADS-1);
	if(strlen(excluded_tids) < 1)
		return false;

	while ((token = strsep(&excluded_tids, ",")) != NULL) {
		pid_t e_tid = (pid_t)atoi(token);
		if(e_tid == tid) {
#ifdef DINAMITE_VERBOSE
			printf("Excluding tid %d from trace\n", tid);
#endif
			free(orig);
			return true;
		}
	}
	free(orig);
	return false;
}

static void __dinamite_create_key(void) {

	int ret = pthread_key_create(&tls_key, NULL);

	if(ret) {
		fprintf(stderr,
			"pthread_key_create: could not create "
			"a local-storage key: %s\n", strerror(ret));
		exit(-1);
	}
}

static inline int __dinamite_get_next_id(void) {

	int ret;
	pthread_mutex_lock(&id_mtx);
	ret = next_id++;
	pthread_mutex_unlock(&id_mtx);

	return ret;
}

static inline pid_t __dinamite_gettid(void) {

	int tid;

	int ret = pthread_once(&dinamite_once_control, __dinamite_create_key);

	if(ret) {
		fprintf(stderr,
			"pthread_once: could not create "
			"a local-storage key: %s\n", strerror(ret));
		exit(-1);
	}

	if( (tid = (pid_t)(long)pthread_getspecific(tls_key)) == 0) {
		tid = __dinamite_get_next_id();
		if(__dinamite_exclude_tid(tid)) {
			/*
			 * The user wanted to discard log records for this
			 * thread ID. Set the tid to an invalid value to
			 * force log records discarded.
			 */
			tid = MAX_THREADS;
		}
		pthread_setspecific(tls_key, (void *) (long)tid);
	}

	return tid;
}

static inline bool
__dinamite_ok_tid(pid_t tid, bool quiet) {

	if(tid > MAX_THREADS -1)
		return false;
	return true;
}

static inline bool
__dinamite_init_buffer(pid_t tid) {

	entries[tid] = (logentry *)malloc(sizeof(logentry) * BUFFER_SIZE);
	if(entries[tid] == NULL) {
		fprintf(stderr, "Warning: could not allocate entries buffer "
			"for thread %d: %s\n", tid, strerror(errno));
		return false;
	}
	return true;
}

static inline bool
__dinamite_ok_buffer(pid_t tid) {

	if(entries[tid] == NULL)
		return __dinamite_init_buffer(tid);
	else
		return true;
}

static inline bool
__dinamite_opened_outfile(pid_t tid) {

	char fname[PATH_MAX];
	char *prefix = NULL;

	if(!__dinamite_ok_tid(tid, true))
		return false;

	prefix = getenv("DINAMITE_TRACE_PREFIX");

	if(prefix != NULL)
		snprintf((char*)fname, PATH_MAX-1, "%s/trace.bin.%d", prefix,
			 tid);
	else
		snprintf((char*)fname, PATH_MAX-1, "trace.bin.%d", tid);
	out[tid] = fopen(fname, "wb");

	if(out[tid] == NULL) {
		fprintf(stderr,
			"Warning: could not open file %s\n", strerror(errno));
		return false;
	}
	fprintf(stdout,
		"Opened file %s\n", fname);
	return true;
}

static inline int
__dinamite_ok_outfile(pid_t tid) {

	if(!__dinamite_ok_tid(tid, true))
		return false;

	if(out[tid] == NULL)
		return __dinamite_opened_outfile(tid);
	else return true;
}

static
void insertOrWrite(logentry *le) {

	pid_t tid = __dinamite_gettid();
	if(!__dinamite_ok_tid(tid, true) || !__dinamite_ok_buffer(tid))
		return;

	if (current[tid] >= BUFFER_SIZE) {
		if(__dinamite_ok_outfile(tid)) {
			fwrite(entries[tid], sizeof(logentry), BUFFER_SIZE,
			       out[tid]);
		}
		current[tid] = 0;
	}
	entries[tid][current[tid]++] = *le;
}

void fillFnLog(fnlog *fnl, char fn_event_type, int functionId) {
	fnl->thread_id = __dinamite_gettid();
	fnl->fn_event_type = fn_event_type;
	fnl->function_id = functionId;
	fnl->fn_timestamp = (uint64_t) dinamite_time_nanoseconds();
}

void fillAccessLog(accesslog *acl, void *ptr, char value_type,
		   value_store value, int type, int file, int line,
		   int col, int typeId, int varId) {
	acl->thread_id = __dinamite_gettid();
	acl->ptr = ptr;
	acl->value_type = value_type;
	acl->value = value;
	acl->type = type;
	acl->file = file;
	acl->line = line;
	acl->col = col;
	acl->typeId = typeId;
	acl->varId = varId;
	acl->ac_timestamp = (uint64_t) dinamite_time_nanoseconds();
}

void fillAllocLog(alloclog *all, void *addr, uint64_t size, uint64_t num,
		  int type, int file, int line, int col) {
	all->thread_id = __dinamite_gettid();
	all->addr = addr;
	all->size = size;
	all->num = num;
	all->type = type;
	all->file = file;
	all->line = line;
	all->col = col;
	all->al_timestamp = (uint64_t) dinamite_time_nanoseconds();
}

/* Open a per-thread log file. */

void logInit(int functionId) {

	int ret = pthread_once(&dinamite_once_control, __dinamite_create_key);

	if(ret) {
		fprintf(stderr,
			"pthread_once: could not create "
			"a local-storage key: %s\n", strerror(ret));
		exit(-1);
	}
}

void logExit(int functionId) {

	int tid;
	for(tid = 0; tid < MAX_THREADS; tid++) {
		if (entries[tid] && current[tid] > 0) {
			if(__dinamite_ok_outfile(tid)) {
				fwrite(entries[tid], sizeof(logentry),
				       current[tid],
				       out[tid]);
				fflush(out[tid]);
				fclose(out[tid]);
				out[tid] = NULL;
			}
		}
	}
}


void logFnBegin(int functionId) {
    logentry le;
    le.entry_type = LOG_FN;
    fillFnLog(&(le.entry.fn), FN_BEGIN, functionId);
    insertOrWrite(&le);
}

void logFnEnd(int functionId) {
    logentry le;
    le.entry_type = LOG_FN;
    fillFnLog(&(le.entry.fn), FN_END, functionId);
    insertOrWrite(&le);
}

void logAlloc(void *addr, uint64_t size, uint64_t num, int type, int file,
	      int line, int col) {
    logentry le;
    le.entry_type = LOG_ALLOC;
    fillAllocLog(&(le.entry.alloc), addr, size, num, type, file, line, col);
    insertOrWrite(&le);
}

void logAccessPtr(void *ptr, void *value, int type, int file, int line, int col,
		  int typeId, int varId) {

    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.ptr = value;
    fillAccessLog(&(le.entry.access), ptr, PTR, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);

}

/* This function logs an access when we are sure that what we are acccessing is
 * a null-terminated string. A typical use-case is when we print a string passed
 * as an argument to the tracepoint function. Using this function when we are
 * not sure whether the address points to a null-terminated string is unsafe,
 * because we may crash when we try to print it later.
 *
 * Another crucial assumption we are making is that the strings being accessed
 * are static. Here is the reason: To avoid the runtime overhead associated
 * with string printing, this function simply stores the pointer when called,
 * and at the very end of the program goes over the pointers and prints them.
 * Here we are assuming that the pointers accessed earlier in the program are
 * still valid at the end of the program and that they are still pointing to the
 * same values as they did when they were actually accessed. This will be true
 * for static strings, but may not be true for dynamic strings. So this function
 * is not safe to use with dynamically allocated strings.
 */
void logAccessStaticString(void *ptr, void *value, int type, int file, int line,
			   int col, int typeId, int varId) {

    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.ptr = value;
    fillAccessLog(&(le.entry.access), ptr, PTR, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);
}

void logAccessI8(void *ptr, uint8_t value, int type, int file, int line,
		 int col, int typeId, int varId) {
    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.i8 = value;
    fillAccessLog(&(le.entry.access), ptr, I8, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);

}

void logAccessI16(void *ptr, uint16_t value, int type, int file, int line,
		  int col, int typeId, int varId) {
    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.i16 = value;
    fillAccessLog(&(le.entry.access), ptr, I16, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);

}

void logAccessI32(void *ptr, uint32_t value, int type, int file, int line,
		  int col, int typeId, int varId) {
    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.i32 = value;
    fillAccessLog(&(le.entry.access), ptr, I32, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);

}

void logAccessI64(void *ptr, uint64_t value, int type, int file, int line,
		  int col, int typeId, int varId) {
    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.i64 = value;
    fillAccessLog(&(le.entry.access), ptr, I64, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);

}

/* =============================
   These don't exist: */

void logAccessF8(void *ptr, uint8_t value, int type, int file, int line,
		 int col, int typeId, int varId) {
}

void logAccessF16(void *ptr, uint16_t value, int type, int file, int line,
		  int col, int typeId, int varId) {

}

/* ============================= */

void logAccessF32(void *ptr, float value, int type, int file, int line, int col,
		  int typeId, int varId) {
    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.f32 = value;
    fillAccessLog(&(le.entry.access), ptr, F32, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);
}

void logAccessF64(void *ptr, double value, int type, int file, int line,
		  int col, int typeId, int varId) {
    logentry le;
    le.entry_type = LOG_ACCESS;
    value_store vs;
    vs.f64 = value;
    fillAccessLog(&(le.entry.access), ptr, F64, vs, type, file, line, col,
		  typeId, varId);
    insertOrWrite(&le);
}
#endif
