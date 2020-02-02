

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/chrono.h"
#include "core/logger.h"


float chrono_time,    chrono_delta;
u32   chrono_time_us, chrono_delta_us;
u64   chrono_time_ns, chrono_delta_ns;
u64   chrono_realtime;


static u64 epoch;
static u64 now;



void chrono_create()
{

	chrono_time    = 0.0f;
	chrono_time_us = 0;
	chrono_time_ns = 0;

	chrono_delta    = 0.0f;
	chrono_delta_us = 0;
	chrono_delta_ns = 0;

	chrono_realtime = 0;

	epoch = 0;

	chrono_set_epoch(0);

	now = 0;

}



void chrono_destroy()
{
}



u64 chrono_get_epoch()
{

	return epoch;

}



void chrono_set_epoch(u64 _epoch)
{

	now += epoch;

	if (_epoch == 0) {

		struct timespec time;

		clock_gettime(CLOCK_MONOTONIC_RAW, &time);

		epoch = time.tv_sec * 1000000000ULL + time.tv_nsec;

	} else
		epoch = _epoch;

	now  -= epoch;

	log_i("chrono: Epoch is now %lu", epoch);

}



void chrono_update()
{

	struct timespec time;

	clock_gettime(CLOCK_MONOTONIC_RAW, &time);

	const u64 clock = (time.tv_sec * 1000000000ULL + time.tv_nsec) - epoch;

	chrono_delta    = (float)(clock - now) / 1.0e9;
	chrono_delta_us = (clock - now) / 1000000UL;
	chrono_delta_ns = clock - now;

	chrono_time    = (float)clock / 1.0e9;
	chrono_time_us = clock / 1000000UL;
	chrono_time_ns = clock;

	now             = clock;
	chrono_realtime = clock + epoch;

}

