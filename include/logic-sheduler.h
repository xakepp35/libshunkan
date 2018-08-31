/*
	Fixed timestep logic sheduler
	Date: 2018.08.30
	Author: xakepp35
	License: FreeBSD 3-clause
*/
#pragma once

#include <cstdint>
#include <atomic>

namespace logic {


	typedef uint64_t stamp_t;


	stamp_t qpf();
	stamp_t qpc();


	class sheduler
	{
	public:

		sheduler(uint64_t fixedFPS);
		~sheduler();

		// returns current frame number
		stamp_t frame_number() const;

		// for most input events it would be "in the future", for timer event (on_frame_finish) it would be "in the past"
		uint64_t target_frame_stamp() const;

		// frame_data manipulations: maintains sync to render thread
		void frame_start_atomic(stamp_t targetStamp);
		void frame_end_atomic(stamp_t actualStamp);


	protected:

		uint64_t _frameNumber;
		uint64_t _fixedFPS;

		stamp_t _startStamp;

		struct frame_data {
			void*		_userData;
			uint64_t	_frameNumber;
			stamp_t		_targetStamp;
			stamp_t		_actualStamp; // jitter could be calculated
		};

		std::atomic< frame_data* > _frameDataExchange; // used for lockless producer-consumer interthread sync(aka triple-buffering)
		frame_data* _frameDataCurrent; // populate this stuff in ctor()!

	};


}