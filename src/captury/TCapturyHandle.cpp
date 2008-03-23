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
#define _LARGEFILE64_SOURCE (1)

#include "captury.h"
#include "TCapturyFrame.h"
#include "TCapturyHandle.h"
#include "codec.h"

#include "TOpenGLCaptureSource.h"
#include "TAudioCaptureSource.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>

#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

/*! \brief merges an image at certain coordinates into another image.
 *  \param dst destination image which gets the source image merged into
 *  \param dstwidth destination image width
 *  \param src source image to be merged into the other image
 *  \param x x-coordinate into the destination image to merge the source image to
 *  \param y y-coordinate into the destination image
 *  \param width width of the source image
 *  \param height height of the source image
 */
inline void mergeImage(uint32_t *dst, int dstwidth, uint32_t *src, int x, int y, int width, int height) {
	dst += y * dstwidth + x;

	for (; height--; src += width, dst += dstwidth)
		memcpy(dst, src, width * 4); // copy row
}

TCapturyHandle::TCapturyHandle(const captury_config_t& AConfig) : 
	FConfig(AConfig), 
	FOutputFd(-1), FAutoCloseFd(false),
	FFrames(8), FVideoEncoder(0), FVideoSource(0),
	FAudioSource(0),
	FRegionBuffer(0),
	FCompositeBuffer(0),
	FCompositing(0),
	FFrameLength(FConfig.width * FConfig.height * 4),
	FLastFrame(0),
	FInterval(1000000.0 / (1.0 * FConfig.fps)), 
	FEncodeInterval(FInterval), FCaptureInterval(FInterval),
	FThread(), FRunning(false) {

	for (int i = 0; i < FFrames.capacity(); ++i) {
		FFrames[i] = new TCapturyFrame();
		FFrames[i]->video.buffer = new uint8_t[FFrameLength];
		FFrames[i]->video.length = FFrameLength;
	}

	FRegionBuffer = new uint8_t[FFrameLength];
	FCompositeBuffer = new uint8_t[FFrameLength];

	FVideoSource = new TOpenGLCaptureSource(this);
	FAudioSource = 0;
}

TCapturyHandle::~TCapturyHandle() {
	delete FAudioSource;
	delete FVideoSource;

	if (FAutoCloseFd)
		close(FOutputFd);

	for (int i = 0; i < FFrames.capacity(); ++i) {
		delete[] FFrames[i]->video.buffer;
		FFrames[i]->video.buffer = 0;
		FFrames[i]->video.length = 0;

		delete FFrames[i];
	}

	delete[] FRegionBuffer;
	delete[] FCompositeBuffer;
}

unsigned long long TCapturyHandle::utime() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (unsigned long long)(tv.tv_sec * 1000000 + tv.tv_usec);
}

void *TCapturyHandle::encodeThreaded(void *arg) {
	captury_t *dev = (captury_t *)arg;

	TCapturyFrame *frame = dev->FFrames.tail();
	const unsigned length = frame->video.length;

	for (; frame->video.length > 0; frame = dev->FFrames.tail()) { 
		unsigned long long start = utime();
		CapturyEncoderWriteFrame(dev, frame);

		dev->FFrames.tail_advance();

		// adjust encode interval
		const double DECAY = 1.0 / 60.0;

		dev->FVideoSource->lock();
			const double encodeInterval = dev->FEncodeInterval;
			dev->FEncodeInterval = encodeInterval * (1.0 - DECAY) + (utime() - start) * DECAY;
		dev->FVideoSource->unlock();
	}

	// we received a non-positive frame length, which we use to
	// inform us to stop capturing/encoding.

	// restore length for further reuse and eat up marker-frame
	frame->video.length = length;
	dev->FFrames.tail_advance();

	pthread_exit(0);
	return 0;
}

void TCapturyHandle::prepareProcessing() {
	FLastFrame = 0;
	FCaptureDelay = 0;

	CapturyEncoderInitialize(this);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

	pthread_create(&FThread, &attr, &TCapturyHandle::encodeThreaded, this);

	FCursorChanged = true;
	FCursorX = -1;
	FCursorY = -1;
	FCursorSerial = -1;

	FRunning = true;
}

// vim:ai:noet:ts=4:nowrap
