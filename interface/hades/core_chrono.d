

extern(C) {

	extern float chrono_time,    chrono_delta;
	extern uint  chrono_time_us, chrono_delta_us;
	extern ulong chrono_time_ns, chrono_delta_ns;
	extern ulong chrono_realtime;


	void chrono_create();
	void chrono_destroy();

	ulong chrono_get_epoch();
	void  chrono_set_epoch(ulong _epoch);

	void chrono_update();

}

