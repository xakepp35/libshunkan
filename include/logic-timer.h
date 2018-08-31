/*
	Timer event for event loop
	Date: 2018.08.30
	Author: xakepp35
	License: FreeBSD 3-clause
*/
#pragma once

// stamp_t, low level timing and calculations
#include "logic-sheduler.h" 

// timer is an event source
#include "logic-loop.h"

namespace logic {

	class timer:
		public event_source
	{
	public:

		timer(event_source_dispatch_t dispatchDelegate = nullptr);
		~timer();

		// rearms to targetStamp (ABSOLUTE TIME)
		void rearm(stamp_t targetStamp);

		// reads number of expirations
		uint64_t read_expirations();
		

	protected:

		int _timerFD; // timer descriptor
	};


}