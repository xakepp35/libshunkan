#include "../include/logic-sheduler.h"

#include <time.h>

// nanoseconds in second
#define QPF_TICKS_IN_SECOND 1000000000ULL


namespace logic {

	stamp_t qpf() {
		return QPF_TICKS_IN_SECOND;
	}

	static void s2ts(struct timespec& destTS, stamp_t srcStamp) {
		destTS.tv_sec = srcStamp / qpf();
		destTS.tv_nsec = srcStamp % qpf();
	}

	static stamp_t ts2s(struct timespec& srcTS) {
		return static_cast<stamp_t>(srcTS.tv_sec) * qpf() + static_cast<stamp_t>(srcTS.tv_nsec);
	}

	stamp_t qpc() {
		struct timespec currts;
		clock_gettime(CLOCK_MONOTONIC, &currts);
		return ts2s(currts);
	}

	////////////////
	// sheduler

	sheduler::sheduler(uint64_t fixedFPS) :
		_fixedFPS(fixedFPS),
		_frameNumber(0),
		_frameDataExchange(nullptr),
		_frameDataCurrent(nullptr)
	{
		_startStamp = qpc();
		frame_start_atomic(_startStamp);
	}


	sheduler::~sheduler()
	{}


	stamp_t sheduler::target_frame_stamp() const {
		auto hpF = qpf();
		auto stampDelta = (hpF * _frameNumber) / _fixedFPS;
		return _startStamp + stampDelta;
	}


	void sheduler::frame_start_atomic(stamp_t targetStamp) {
		if (!_frameDataCurrent) // _frameDataCurrent may be nullptr, avoid that
			_frameDataCurrent = new frame_data;
		_frameDataCurrent->_frameNumber = _frameNumber; // frame number is stored in next frame
		_frameDataCurrent->_targetStamp = targetStamp;
	}


	void sheduler::frame_end_atomic(stamp_t actualStamp) {
		_frameDataCurrent->_actualStamp = actualStamp; // actual stamp is when frame data is passed to render thread
		_frameDataCurrent = _frameDataExchange.exchange(_frameDataCurrent); // _ptrExchange would store most actual result
		++_frameNumber; // frame number increases
	}


	stamp_t sheduler::frame_number() const {
		return _frameNumber;
	}


}