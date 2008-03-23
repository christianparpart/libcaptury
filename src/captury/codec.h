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
#ifndef sw_captury_codec_h
#define sw_captury_codec_h

#include "captury.h"
#include <capseo.h>

struct TCapturyFrame;

struct captury_video_encoder_t {
	uint8_t *buffer;
};

struct captury_decoder_t {
	int width;
	int height;

	int inputFd;
	bool autoCloseFd;

	captury_video_frame_t videoFrame;

	char errorString[256];
};

#if defined(__cplusplus)
extern "C" {
#endif

// --------------------------------------------------------------------------
// Encoder API

bool CapturyEncoderInitialize(captury_t *dev);
void CapturyEncoderWriteFrame(captury_t *dev, TCapturyFrame *frame);
void CapturyEncoderFinalize(captury_t *dev);

// --------------------------------------------------------------------------
// Decoder API

captury_decoder_t *CapturyDecoderCreate();
void CapturyDecoderDestroy(captury_decoder_t *dc);

int CapturyDecoderSetInputFileName(captury_decoder_t *dc, const char *AFileName);
int CapturyDecoderSetInputFd(captury_decoder_t *dc, int fd);

void CapturyDecoderGetGeometry(captury_decoder_t *dc, int *width, int *height);

captury_video_frame_t *CapturyDecodeFrame(captury_decoder_t *dc);

int CapturyDecoderErrorCode(captury_decoder_t *dc);
const char *CapturyDecoderErrorString(captury_decoder_t *dc);

#if defined(__cplusplus)
}
#endif

#endif
// vim:ai:noet:ts=4:nowrap
