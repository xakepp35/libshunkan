/*
	game_loop
*/

#include "../include/logic-gameloop.h"

namespace logic {


	gameloop::gameloop(uint64_t fixedFPS, void* userDataInitial) :
		sheduler(fixedFPS),
		loop(),
		_sTimer()
	{
		// feed some initial data (frame0) before even first tick occurs
		_frameDataCurrent->_userData = userDataInitial;

		// and we may use default inter-frame switching method to display and start processing frame 1
		advance_frame();
		add_fd(_sTimer.get_fd(), &gameloop::frame_finish_wrapper);
	}


	gameloop::~gameloop() {

	}


	void gameloop::advance_frame() {
		// write data to pass to render thread
		frame_end_atomic(qpc());

		auto targetStamp = target_frame_stamp();
		frame_start_atomic(targetStamp);

		// rearm timer for next timeslice
		_sTimer.rearm(targetStamp);
	}


	void gameloop::frame_finish_wrapper(int fD) {
		_sTimer.read_expirations();

		on_frame_finish(_frameDataCurrent->_userData); // do physics calculations, results are to be uploaded in _frameDataCurrent->_userData

		//auto lastDelta = static_cast<double>(qpc() - targetStamp) / hpF; // jitter
		//vLags.emplace_back(lastDelta);

		advance_frame();
	}



}