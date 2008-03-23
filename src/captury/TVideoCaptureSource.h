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
#ifndef sw_captury_TVideoCaptureSource_h
#define sw_captury_TVideoCaptureSource_h

#include <pthread.h>

struct TCapturyHandle;

class TVideoCaptureSource {
protected:
	TCapturyHandle *dev;
	pthread_mutex_t FMutex;		//!< access synchronization (capturer vs encoder)

public:
	explicit TVideoCaptureSource(TCapturyHandle *dev);
	virtual ~TVideoCaptureSource();

	void lock();
	void unlock();

	int initializeFrameProcessing();

	virtual void captureScreen() = 0;
	virtual void captureRegion(int x, int y, int width, int height) = 0;
	virtual void captureCursor() = 0;
};

#endif
