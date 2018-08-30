/*
	Game loop backend
	Date: 2018.08.30
	Author: xakepp35
	License: FreeBSD 3-clause
*/
#pragma once

// C++
#include <cstdint> 
#include <vector>
#include <atomic>

namespace logic {
	
	typedef uint64_t stamp_t;

	// game loop backend
	class loop {
		
		public:
		
			loop(uint64_t fixedFPS, void* userDataInitial);
			~loop();
			
			// runs the loop
			void main();
			
			// returns current frame number
			stamp_t frame_number() const;
			
			// for most input events it would be "in the future", for timer event (on_frame_finish) it would be "in the past"
			uint64_t frame_stamp() const;
			
			virtual void on_frame_finish() = 0;
		
		// impl
		protected:
		
			void poll_events();
			
			void frame_finish_wrapper(int fd);
			
			void advance_frame();
			
			// frame_data manipulations: maintains sync to render thread
			void frame_start_atomic(stamp_t targetStamp);
			void frame_end_atomic(stamp_t actualStamp);
			
			// rearms timerfd
			void rearm_timer(stamp_t targetStamp);

			void rearm_timer(stamp_t targetStamp);	
			
			// write state to pass to render thread
			void write_frame_atomic(); 
			
		// epoll fd control
		protected:
		
			// libinput mimics: libinput-private.h
			void (loop::*event_source_dispatch_t)(int fd);
			
			struct event_source {
				//loop *thisLoop; // this
				event_source_dispatch_t _dispatch;
				int _fd; // emitter file descriptor for event
				
				event_source(event_source_dispatch_t dispatch = NULL, int fd = -1);
				~event_source();
			};
			
			event_source& add_fd(int fd, event_source_dispatch_t dispatch);
			
		// data
		protected:
		
			uint64_t _frameNumber;
			uint64_t _fixedFPS;
			
			int _epollFD; // epoll descriptor
			int _timerFD; // timer descriptor
			
			
			struct frame_data {
				void*		_userData;
				uint64_t	_frameNumber;
				stamp_t		_targetStamp;
				stamp_t		_actualStamp; // jitter could be calculated
			};
			
			
			
			std::atomic< frame_data* > _frameDataExchange; // used for lockless producer-consumer interthread sync(aka triple-buffering)
			frame_data* _frameDataCurrent; // populate this stuff in ctor()!
			
			std::vector<event_source> _eventSources;
			bool _loopActive;
		
	};
	
}
