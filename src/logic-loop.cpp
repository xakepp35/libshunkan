#include "../include/logic-loop.h"

// LINUX
#include <unistd.h>
#include <sys/epoll.h>

#include <array>
#include <cstring>
#include <stdexcept>


// each event size is 12 bytes, so whole buffer would be 3k and fits on stack; "256 events per 1 tick is suffient for all!"
#define EPOLL_EVENT_COUNT 256

namespace logic {

	
////////////////
// event_source
	
	event_source::event_source(event_source_dispatch_t dispatchDelegate, int fD):
		_dispatchDelegate(dispatchDelegate),
		_fD(fD)
	{}
	

	event_source::~event_source() {
		close(_fD);
	}


	int event_source::read_buf(void* dataBuf, size_t dataSize) {
		return read(_fD, dataBuf, dataSize);
	}


	int event_source::get_fd() const {
		return _fD;
	}
	
////////////////
// loop
	
	loop::loop():
		event_source(nullptr, epoll_create1(0)),
		_loopActive(false)
	{
		if (_fD == -1) // epoll fd create
			throw std::runtime_error("epoll_create1(0)");
	}
	
	
	loop::~loop() {
		_eventSources.clear(); // close(_eventSources[i]._fd);
	}


	void loop::main() {
		//_frameNumber = 1; // frame 0 is "initial state", so we start waiting frame 1..
		_loopActive = true;
		do { // frame event processing
			poll_events();
		} while (_loopActive); // loop finishes
	}
	
	
	void loop::poll_events() {
		// i'd like to keep that on stack, and in L1 cache, if possible!
		std::array< epoll_event, EPOLL_EVENT_COUNT > eEv; // = {0};
		//memset(&eEv, 0, sizeof(eEv));
		auto eventCount = epoll_wait(_epollFD, &eEv, EPOLL_EVENT_COUNT, -1); // wait indefinetely
		if (eventCount > 0) {
			int i = 0;
			do {
				auto eventSource = reinterpret_cast<event_source*>(eEv[i].data.ptr);
				(*this.*(eventSource->_dispatchDelegate))(*this, eventSource->fD);
			} while( (++i) < eventCount );
		}
		else // logic stucked, epoll fD closed or some other shit happened, cannot continue!
			throw std::runtime_error("epoll_wait()");
	}
	

	event_source& loop::add_fd(int fD, event_source_dispatch_t dispatch) {
		_eventSources.emplace_back(dispatch, this, fD);
		auto& eventSource = _eventSources.back();

		epoll_event eEv;
		memset(&eEv, 0, sizeof(eEv) );
		eEv.events = EPOLLIN;
		eEv.data.ptr = &eventSource;

		if (epoll_ctl(_fD, EPOLL_CTL_ADD, fD, &eEv) < 0) {
			_eventSources.pop_back();
			throw std::runtime_error("epoll_ctl(EPOLL_CTL_ADD)");
		}
		return eventSource;
	}


	bool loop::add_fd(const event_source& externalSource) {
		epoll_event eEv;
		memset(&eEv, 0, sizeof(eEv));
		eEv.events = EPOLLIN;
		eEv.data.ptr = &externalSource;
		if (epoll_ctl(_fD, EPOLL_CTL_ADD, externalSource.get_fd(), &eEv) < 0) {
			throw std::runtime_error("epoll_ctl(EPOLL_CTL_ADD)");
			return false;
		}
		return true;
	}
	

}
