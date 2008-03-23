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
#ifndef sw_captury_h
#define sw_captury_h

#include <X11/Xlib.h>						// Window, Display*
#include <netinet/in.h> 					// FIXME: need uint8_t type only

/* return codes */
#define CAPTURY_SUCCESS			(0)			/*!< function performed as expected */
#define CAPTURY_E_SYSTEM		(-1)		/*!< system error (errno) */
#define CAPTURY_E_GENERAL		(-2)		/*!< general application error */
#define CAPTURY_E_INVALID		(-1)		/*!< invalid state while for this call */

#define CAPTURY_DENIED			(0x1101)	/*!< this time no frames may be passed as
												 neither regions nor fullscreen to
												 the device. */

/* deviceType */
#define CAPTURY_DEVICE_GLX		(0x1201)	/*!< capture from GLX context */
#define CAPTURY_DEVICE_XSHM		(0x1202) 	/*!< capture from XSHM, X11 root window 
												 (currently not yet supported) */

typedef struct TCapturyHandle captury_t;

typedef struct TCapturyVideoFrame {
	uint64_t id;					/*!< frame id */
	uint8_t *buffer;				/*!< BGRA buffer containing the frame data */
	int length;						/*!< buffer length */
} captury_video_frame_t;

typedef struct TCapturyConfig {
	// video
	int x;							/*!< lower y coordinate */
	int y;							/*!< lower x coordinate */
	int width;						/*!< capture width (relative to x) */
	int height;						/*!< capture height (relative to y) */
	double fps;						/*!< ideal fps */
	int scale;						/*!< frame down scaling (0=disabled, 1=half, ...) */
	int cursor;						/*!< explicitely draw cursor */

	int deviceType;					/*!< device type to capture from */
	Display *deviceHandle;			/*!< X11 connection handle */
	Window windowHandle;			/*!< X11 Window you want to capture on */
} captury_config_t;

#if defined(__cplusplus)
extern "C" {
#endif

// --------------------------------------------------------------------------
// core API

captury_t *CapturyOpen(captury_config_t *cfg);
void CapturyClose(captury_t *d);

int CapturySetOutputFileName(captury_t *d, const char *filename);
void CapturySetOutputFd(captury_t *d, int fd);

captury_config_t *CapturyGetConfig(captury_t *d);

void CapturyProcessFrame(captury_t *);

// --------------------------------------------------------------------------
// incremental frame processing

int CapturyProcessRegionStart(captury_t *);
int CapturyProcessRegion(captury_t *, int x, int y, int width, int height);
void CapturyProcessRegionCommit(captury_t *);

#if defined(__cplusplus)
}
#endif

#endif
// vim:ai:noet:ts=4:nowrap
