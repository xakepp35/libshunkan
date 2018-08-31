#pragma once

#include "logic-timer.h"

namespace logic {


	class gameloop :
		public loop,
		public sheduler
	{
	public:

		gameloop(uint64_t fixedFPS, void* userDataInitial);
		~gameloop();

		

		virtual void on_frame_finish(void* userData) = 0;

		// write state to pass to render thread
		void advance_frame();
		void frame_finish_wrapper(int fd);

		// shared_ptr<timer> ?
		timer _sTimer;

	};


}