#include "../include/logic-timer.h"

#include <sys/timerfd.h>

#include <cstring>
#include <stdexcept>

namespace logic {

	////////////////
	// timer

	timer::timer(event_source_dispatch_t dispatchDelegate) :
		event_source(dispatchDelegate, timerfd_create(CLOCK_MONOTONIC, 0))
	{
		if (_fD == -1) // timer fd create
			throw std::runtime_error("timerfd_create(CLOCK_MONOTONIC)");
	}

	timer::~timer() 
	{}

	void timer::rearm(stamp_t targetStamp) {
		// store event timer expiration stamp, at which frame is considered finished
		struct itimerspec iTS;
		memset(&iTS.it_interval, 0, sizeof(iTS.it_interval));
		s2ts(iTS.it_value, targetStamp);
		auto ret = timerfd_settime(_timerFD, TFD_TIMER_ABSTIME, &iTS, NULL);
		if (ret < 0)
			throw std::runtime_error("timerfd_settime(TFD_TIMER_ABSTIME)");
	}

	uint64_t timer::read_expirations() {
		uint64_t numExpirations = 0;
		auto ret = read_buf(&numExpirations, sizeof(numExpirations));
		return numExpirations;
	}

	


}