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
#include "TCapturyFrame.h"

#include <strings.h>

TCapturyFrame::TCapturyFrame() {
	bzero(&video, sizeof(video));
	bzero(&cursor, sizeof(cursor));

	cursor.buffer = new unsigned char[256 * 256 * 4]; // allow a maximum extend of 256x256 cursors, at ARGB
}

TCapturyFrame::~TCapturyFrame() {
	delete[] cursor.buffer;
}

// vim:ai:noet:ts=4:nowrap
