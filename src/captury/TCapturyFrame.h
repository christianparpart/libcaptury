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
#ifndef sw_captury_TCapturyFrame_h
#define sw_captury_TCapturyFrame_h

#include "fifo.h"
#include "captury.h"
#include <capseo.h>
#include <pthread.h>

struct TCapturyFrame {
	captury_video_frame_t video;
	capseo_cursor_t cursor;

	// and audio, some time later

	explicit TCapturyFrame();
	~TCapturyFrame();
};

#endif
