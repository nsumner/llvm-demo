#include <time.h>
#include "dinamite/dinamite_time.h"

#define NS_IN_SECOND 1000000000

#ifdef __MACH__
#include <mach/mach_time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
static int clock_gettime(int clk_id, struct timespec *t){
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	uint64_t time;
	time = mach_absolute_time();
	double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
	double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
	t->tv_sec = seconds;
	t->tv_nsec = nseconds;
	return 0;
}
#endif


uint64_t
dinamite_time_nanoseconds(void) {

	struct timespec ts;
	if( clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
		return ts.tv_sec * NS_IN_SECOND + ts.tv_nsec;
	else
		return 0;
}
