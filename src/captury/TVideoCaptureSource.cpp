/////////////////////////////////////////////////////////////////////////////
//
//  Captury - http://rm-rf.in/captury
//  $Id$
//  (libcaptury - captury framework library)
//
//  Copyright (c) 2007 by Christian Parpart <trapni@gentoo.org>
//
//  This file as well as its whole library is licensed under
//  the terms of GPL. See the file COPYING.
//
/////////////////////////////////////////////////////////////////////////////
#include "TVideoCaptureSource.h"
#include "TCapturyHandle.h"

TVideoCaptureSource::TVideoCaptureSource(TCapturyHandle *AHandle) : dev(AHandle) {
	pthread_mutex_init(&FMutex, 0);
}

TVideoCaptureSource::~TVideoCaptureSource() {
	pthread_mutex_destroy(&FMutex);
}

void TVideoCaptureSource::lock() {
	pthread_mutex_lock(&FMutex);
}

void TVideoCaptureSource::unlock() {
	pthread_mutex_unlock(&FMutex);
}

int TVideoCaptureSource::initializeFrameProcessing() {
	const unsigned long long _now = dev->utime();
	const unsigned long long _elapsed = _now - dev->FLastFrame;

	// recompute capture interval
	lock();
	const double _encodeInterval = dev->FEncodeInterval;
	unlock();

	const int _bufferLoad = dev->FFrames.capacity() / 2 - dev->FFrames.size();
	double _captureInterval = dev->FCaptureInterval;
	const double _correction = (_encodeInterval + _bufferLoad * 100) - _captureInterval;
	_captureInterval = _captureInterval * 0.9 + (_captureInterval + _correction) * 0.1;

	dev->FCaptureInterval = _captureInterval < dev->FInterval
		? dev->FInterval
		: _captureInterval;

	const double _captureDelayMargin = _captureInterval / 10.0; // may not delay more then 10% over limit
	double _captureDelay = dev->FCaptureDelay - _elapsed;

	dev->FLastFrame = _now;

	if (_captureDelay < _captureDelayMargin) {
		if (!dev->FFrames.full()) {
			dev->FHeadFrame = dev->FFrames.head();

			if (_captureDelay >= 0) // frame to early
				dev->FCaptureDelay = _captureInterval + _captureDelay;
			else if (_captureInterval + _captureDelay < 0) // lag
				dev->FCaptureDelay = dev->FCaptureInterval;
			else
				dev->FCaptureDelay = dev->FCaptureInterval + _captureDelay;

			return CAPTURY_SUCCESS;
		} else if (_captureDelay < 0)
			dev->FCaptureDelay = 0;
		else
			dev->FCaptureDelay = _captureDelay;
	} else {
		dev->FCaptureDelay = _captureDelay;
	}

	return CAPTURY_DENIED;
}

// vim:noet:ts=4
