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
#ifndef sw_captury_TOpenGLCaptureSource_h
#define sw_captury_TOpenGLCaptureSource_h

#include "fifo.h"
#include <capseo.h>
#include <pthread.h>
#include "TVideoCaptureSource.h"

struct captury_video_encoder_t;
struct TCapturyFrame;

class TOpenGLCaptureSource : public TVideoCaptureSource {
public:
	explicit TOpenGLCaptureSource(TCapturyHandle *);
	~TOpenGLCaptureSource();

	bool ensureCaptureFunction();

	void prepareProcessing();

public: // TVideoCaptureSource API
	virtual void captureScreen();
	virtual void captureRegion(int x, int y, int width, int height);

	virtual void captureCursor();
};

#endif
