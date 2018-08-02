#ifndef BINARY_INSTRUMENTATION_H
#define BINARY_INSTRUMENTATION_H

#define TID_TYPE char

enum fn_events {
    FN_BEGIN, FN_END
};

typedef struct _fnlog {
	TID_TYPE thread_id;
	char fn_event_type;
	int function_id;
	uint64_t fn_timestamp;
} fnlog;

enum value_type {
    I8, I16, I32, I64,
    F32, F64, PTR,
    MAX_VALUE_TYPE
};

typedef union {
    void *ptr;
    uint8_t i8;
    uint16_t i16;
    uint32_t i32;
    uint64_t i64;
    float f32;
    double f64;
} value_store;

typedef struct _accesslog {
	void *ptr; // 8
	char value_type; // 1
	value_store value; // 8
	TID_TYPE thread_id; // 1
	char type; // 1
	uint16_t file; // 2 // Is this enough bits for the file ID?
	uint16_t line; // 2
	uint16_t col; // 2
	uint16_t typeId; // 2
	uint16_t varId; // 2
	uint64_t ac_timestamp; // 8
} accesslog;

typedef struct _alloclog {
	void *addr; // 8
	uint64_t size; // 8
	uint64_t num; // 8
	uint16_t type; // 2
	uint16_t file; // 2
	uint16_t line; // 2
	uint16_t col; // 2
	TID_TYPE thread_id; // 1
	uint64_t al_timestamp; // 8
} alloclog;

enum entry_types {
	LOG_FN, LOG_ALLOC, LOG_ACCESS
};

typedef struct _logentry {
	char entry_type;
	union {
		fnlog fn;
		accesslog access;
		alloclog alloc;
	} entry;
} logentry;

#endif
