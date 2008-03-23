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
#ifndef sw_captury_TCapturyHandle_h
#define sw_captury_TCapturyHandle_h

#include "fifo.h"
#include "captury.h"
#include <capseo.h>
#include <pthread.h>

struct captury_video_encoder_t;
struct TCapturyFrame;
class TVideoCaptureSource;
class TAudioCaptureSource;

struct TCapturyHandle {
public:
	captury_config_t FConfig;	//!< user configuration defining attributes like width/height/fps/...

	int FOutputFd;				//!< system file descriptor to write encoded data to.
	bool FAutoCloseFd;			//!< true, if outputFd shall be closed on CapturyClose()
	capseo_stream_t *FStream;	//!< capseo output stream

	TFifo<TCapturyFrame *> FFrames;			//!< frame-pool to bridge communication between capturer and encoder
	TCapturyFrame *FHeadFrame;				//!< fast access to FFrames.head(); -- only where we need no syncing
	captury_video_encoder_t *FVideoEncoder;	//!< video encoder private structure

	TVideoCaptureSource *FVideoSource;		//!< video source capture device
	TAudioCaptureSource *FAudioSource;		//!< audio source capture device (not yet impelmented/used)

	uint8_t *FRegionBuffer;		//!< destination buffer for glReadPixels
	uint8_t *FCompositeBuffer;	//!< destination buffer for compositing the new frame via CapturyProcessRegion()
	bool FCompositing;			//!< true when incremental frame processing detected

	int FFrameWidth;			//!< result (scaled) frame width
	int FFrameHeight;			//!< result (scaled) frame height
	int FFrameLength;			//!< frame buffer length for the unencoded scaled frame

	uint64_t FLastFrame;		//!< time-id of last frame being captured
	double FInterval;			//!< expected capture interval
	double FEncodeInterval;		//!< dynamic interval as the encoder actually encodes its incoming frames in
	double FCaptureInterval;	//!< dynamic interval we're actually capturing with
	double FCaptureDelay;		//!< current capture delay, a time we lag behind in capturing

	pthread_t FThread;			//!< thread-id from threaded encoder
	bool FRunning;				//!< true, if at least one frame has been captured already

	bool FCursorChanged;
	short FCursorX;
	short FCursorY;
	long FCursorSerial;

public:
	explicit TCapturyHandle(const captury_config_t& AConfig);
	~TCapturyHandle();

	static unsigned long long utime();
	static inline bool ensureCaptureFunction();
	static void *encodeThreaded(void *);

	int initializeFrameProcessing();
	void prepareProcessing();
};

#endif
