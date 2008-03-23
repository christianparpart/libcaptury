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
#include "codec.h"
#include "TCapturyHandle.h"
#include "TCapturyFrame.h"

#include <capseo.h>

#include <string.h>				// memset()
#include <unistd.h>				// write()

bool CapturyEncoderInitialize(captury_t *dev) {
	capseo_info_t info;
	memset(&info, 0, sizeof(info));
	info.format = CAPSEO_FORMAT_BGRA;
	info.width = dev->FConfig.width;
	info.height = dev->FConfig.height;
	info.scale = dev->FConfig.scale;
	info.fps = dev->FConfig.fps;

	if (dev->FConfig.cursor)
		info.cursor_format = CAPSEO_FORMAT_ENCORE_QLZARGB;

	capseo_stream_t *stream;
	if (CapseoStreamCreateFd(CAPSEO_MODE_ENCODE, &info, dev->FOutputFd, &stream) != CAPSEO_SUCCESS)
		return false;

	dev->FVideoEncoder = new captury_video_encoder_t;
	dev->FStream = stream;

	return true;
}

void CapturyEncoderWriteFrame(captury_t *dev, TCapturyFrame *frame) {
	CapseoStreamEncodeFrame(dev->FStream, 
		frame->video.buffer, frame->video.id, 
		dev->FConfig.cursor && dev->FCursorChanged ? &frame->cursor : NULL
	);
}

void CapturyEncoderFinalize(captury_t *dev) {
	if (!dev->FVideoEncoder)
		return;

	CapseoStreamDestroy(dev->FStream);
	dev->FStream = 0;

	memset(dev->FVideoEncoder, 0, sizeof(*dev->FVideoEncoder));
	delete dev->FVideoEncoder;
	dev->FVideoEncoder = 0;
}

// vim:ai:noet:ts=4:nowrap
