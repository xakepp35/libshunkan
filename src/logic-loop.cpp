#include "../include/logic-loop.h"

// LINUX
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>

#include <stdexcept>
//#include <cmath>
//#include <algorithm>
//#include <numeric>
//#include <iostream>

// nanoseconds in second
#define QPF_TICKS_IN_SECOND 1000000000ULL

// each event size is 12 bytes, so whole buffer would be 3k and fits on stack; "256 events per 1 tick is suffient for all!"
#define EPOLL_EVENT_COUNT 256


namespace logic {
	
////////////////
// timespamp-related 

	static stamp_t qpf() {
		return QPF_TICKS_IN_SECOND;
	}

	static void s2ts(struct timespec& destTS, stamp_t srcStamp) {
		destTS.tv_sec = srcStamp / qpf();
		destTS.tv_nsec = srcStamp % qpf();
	}
	
	static stamp_t ts2s(struct timespec& srcTS) {
		return static_cast< stamp_t >(srcTS.tv_sec) * qpf() + static_cast< stamp_t >(srcTS.tv_nsec);
	}
	
	static stamp_t qpc() {
		struct timespec currts;
		clock_gettime(CLOCK_MONOTONIC, &currts);
		return ts2s(currts);
	}
	
////////////////
// loop::event_source
	
	loop::event_source::event_source(event_source_dispatch_t dispatch = NULL, int fd = -1):
		_dispatch(dispatch),
		_fd(fd)
	{}
	
	loop::event_source::~event_source() {
		close(_fd);
	}
	
////////////////
// loop


	loop::loop(uint64_t fixedFPS, void* userDataInitial):
		_fixedFPS(fixedFPS),
		_frameNumber(0),
		_epollFD(-1),
		_timerFD(-1),
		_frameDataExchange(nullptr),
		_frameDataCurrent(nullptr),
		_loopActive(false)
	{
		_epollFD=epoll_create1(0);	//_epollFD=epoll_create(EPOLL_EVENT_COUNT);
        if (_epollFD == -1) // epoll fd create
            throw std::runtime_error("epoll_create1(0)");
			
		_timerFD=timerfd_create(CLOCK_MONOTONIC, 0);
		if (_timerFD == -1) // timer fd create
            throw std::runtime_error("timerfd_create(CLOCK_MONOTONIC)");
			
		add_fd(_timerFD, &loop::frame_finish_wrapper);
		
		_startStamp = qpc();
		
		// feed some initial data (frame0) before even first tick occurs
		frame_start_atomic(_startStamp);
		_frameDataCurrent->_userData = userDataInitial;
		
		// and we may use default inter-frame switching method to display and start processing frame 1
		advance_frame();
	}
	
	
	loop::~loop() {
		_eventSources.clear(); // close(_eventSources[i]._fd);
		close(_epollFD);
	}
	
	
	void loop::poll_events() {
		// i'd like to keep that on stack, and in L1 cache, if possible!
		struct epoll_event eEv[EPOLL_EVENT_COUNT]; // = {0};
		memset(&eEv, 0, sizeof(eEv));
		auto eventCount = epoll_wait(_epollFD, &eEv, EPOLL_EVENT_COUNT, -1); // wait indefinetely
		if (eventCount > 0) {
			int i = 0;
			do {
				auto eventSource = reinterpret_cast<event_source*>(eEv[i].data.ptr);
				(*this.*(eventSource->_dispatch))(eventSource->fd);
			} while( (++i) < eventCount );
		}
		else // logic stucked, epoll fd closed or some other shit happened, cannot continue!
			throw std::runtime_error("epoll_wait()");
	}
	
	void loop::main() {	
		//_frameNumber = 1; // frame 0 is "initial state", so we start waiting frame 1..
		_loopActive = true;
        do { // frame event processing
			poll_events();
        } while (_loopActive); // loop finishes
	}
	
	event_source& loop::add_fd(int fd, libinput_source_dispatch_t dispatch) {
		auto& eventSource = _eventSources.emplace_back(dispatch, this, fd);
	
		struct epoll_event eEv;
		memset(&eEv, 0, sizeof(eEv) ); // method is called rarely, better be sure fields are zeroed!
		eEv.events = EPOLLIN;
		eEv.data.ptr = &eventSource;

		if (epoll_ctl(_epollFD, EPOLL_CTL_ADD, fd, &eEv) < 0) {
			_eventSources.pop_back();
			throw std::runtime_error("epoll_ctl(EPOLL_CTL_ADD)");
		}
		return eventSource;
	}

	stamp_t loop::target_frame_stamp() const {
		auto hpF = qpf();
		auto stampDelta = (hpF * _frameNumber) / _fixedFPS;
        return _startStamp + stampDelta;
	}
	
	void loop::advance_frame(){
		// write data to pass to render thread
		frame_end_atomic(qpc());
		
		auto targetStamp = target_frame_stamp();
		frame_start_atomic(targetStamp);
		
		// rearm timer for next timeslice
		rearm_timer(targetStamp);
	}


	void loop::frame_start_atomic(stamp_t targetStamp) {
		if( !_frameDataCurrent ) // _frameDataCurrent may be nullptr, avoid that
			_frameDataCurrent = new frame_data;
		_frameDataCurrent->_frameNumber = _frameNumber; // frame number is stored in next frame
		_frameDataCurrent->_targetStamp = targetStamp;
	}
	
	
	void loop::frame_end_atomic(stamp_t actualStamp) {
		_frameDataCurrent->_actualStamp = actualStamp; // actual stamp is when frame data is passed to render thread
		_frameDataCurrent = _ptrExchange.exchange(_frameDataCurrent); // _ptrExchange would store most actual result
		++_frameNumber; // frame number increases
	}
	
	
	void loop::rearm_timer(stamp_t targetStamp) {
		 // store event timer expiration stamp, at which frame is considered finished
		struct itimerspec iTS;
		memset(&iTS.it_interval, 0, sizeof(iTS.it_interval));
		s2ts(iTS.it_value, targetStamp);
		auto ret = timerfd_settime(_timerFD, TFD_TIMER_ABSTIME, &iTS, NULL);
		if(ret < 0 )
			throw std::runtime_error("timerfd_settime(TFD_TIMER_ABSTIME)");
	}
	
	
	void loop::frame_finish_wrapper(int fd) {
		if(fd != __timerFD) 
			throw std::runtime_error("loop::frame_finish_wrapper(fd != __timerFD)");
		uint64_t res = 0;
		ret = read(_timerFD, &res, sizeof(res)); // reads number of expirations
		
		on_frame_finish(_frameDataCurrent); // do physics calculations, results are to be uploaded in _frameDataCurrent->_userData
		
		//auto lastDelta = static_cast<double>(qpc() - targetStamp) / hpF; // jitter
		//vLags.emplace_back(lastDelta);
		
		advance_frame();
	}
	

}
