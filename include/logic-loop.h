/*
	Event loop backend (epoll/kqueue/iocp)
	Date: 2018.08.30
	Author: xakepp35
	License: FreeBSD 3-clause
*/
#pragma once

// C++
#include <cstdint> 
#include <vector>


namespace logic {

	class loop;

	// really libinput mimics: see libinput-private.h  huh.. shouldn't i just reue libinput code? Okay, lest stay really sharp and mad :)
	typedef void (loop::*event_source_dispatch_t)(loop& thisLoop, int fD);


	class event_source
	{
	public:

		event_source(event_source_dispatch_t dispatchDelegate, int fD = -1);
		~event_source();

		int read_buf(void* dataBuf, size_t dataSize);
		int get_fd() const;

	public:

		event_source_dispatch_t _dispatchDelegate;
		int _fD; // emitter file descriptor for event

	};


	class loop:
		public event_source // epoll descriptor, couldn't it be registred in different epoll? :-)
	{
	public:

		loop();
		~loop();

		// runs the loop
		void main();

		event_source& add_fd(int fd, event_source_dispatch_t dispatch);
		bool add_fd(const event_source& externalSource);

		// impl
	protected:

		void poll_events();
		
	// data
	protected:

		std::vector<event_source> _eventSources;
		bool _loopActive;
		
	};

	
}
