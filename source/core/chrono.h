

#ifndef CORE_CHRONO_H
#define CORE_CHRONO_H


extern float chrono_time,    chrono_delta;
extern u32   chrono_time_us, chrono_delta_us;
extern u64   chrono_time_ns, chrono_delta_ns;
extern u64   chrono_realtime;


void chrono_create();
void chrono_destroy();

u64  chrono_get_epoch();
void chrono_set_epoch(u64 _epoch);

void chrono_update();


#endif

