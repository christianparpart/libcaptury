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
#include "TCapturyHandle.h"
#include "TCapturyFrame.h"
#include "TVideoCaptureSource.h"
#include "codec.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------
/*! \brief initializes a captury device and returns its handle
 *  \param AConfig parameters to configure your device
 *  \return handle to the captury device handle
 *  \see CapturyClose(), CapturyProcessFrame()
 */
captury_t *CapturyOpen(captury_config_t *AConfig) {
	// {{{ sanity checks

	// validate width/height
	int w = AConfig->width & ~((1 << (AConfig->scale + 1)) - 1);
	int h = AConfig->height & ~((1 << (AConfig->scale + 1)) - 1);

	if (w != AConfig->width || h != AConfig->height) {
		fprintf(stderr, "CAPTURY: Invalid width/height values: %d:%d\n", AConfig->width, AConfig->height);
		fprintf(stderr, "CAPTURY: Reassigning to %d:%dn", w, h);

		AConfig->width = w;
		AConfig->height = h;
	}

	switch (AConfig->deviceType) {
		case CAPTURY_DEVICE_GLX:
			break;
		case CAPTURY_DEVICE_XSHM:
			// not supported
		default:
			// illegal value
			return 0;
	}
	// }}}

	TCapturyHandle *dev = new TCapturyHandle(*AConfig);

	return dev;
}

/*! \brief safely closes captury device
 *  \param dev device handle to close
 *  \see CapturyOpen()
 */
void CapturyClose(captury_t *dev) {
	TCapturyFrame *marker = dev->FFrames.head();
	marker->video.length = -marker->video.length;
	dev->FFrames.head_advance();

	pthread_join(dev->FThread, 0);

	CapturyEncoderFinalize(dev);

	delete dev;
}

/*! \brief assigned a file stream to this capture device to encode the captured movie data to.
 *  \param dev the captury device handle
 *  \param filename the corresponding file name to open. This file gets created and/or truncated if needed.
 *  \return 0 on success, -1 otherwise; errno indicates its respective error code.
 */
int CapturySetOutputFileName(captury_t *dev, const char *filename) {
#if defined(O_LARGEFILE)
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_LARGEFILE, 0666);
#else
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
#endif
	if (fd < 0)
		return CAPTURY_E_SYSTEM;

	dev->FOutputFd = fd;
	dev->FAutoCloseFd = true;

	return CAPTURY_SUCCESS;
}

/*! \brief assigned a file descriptor to the capturing device to write the encoded data to
 *  \param dev the captury device handle
 *  \param fd the system file descriptor. (must be opened for write)
 *  \see CapturySetOutputFileName()
 */
void CapturySetOutputFd(captury_t *dev, int fd) {
	dev->FOutputFd = fd;
	dev->FAutoCloseFd = false;
}

/*! \brief returns pointer to device configuration structure (readonly).
 *  \param dev device handle for which you want the configuration structure.
 *  \return (read only) pointer to configuration structure.
 */
captury_config_t *CapturyGetConfig(captury_t *dev) {
	return &dev->FConfig;
}

/*! \brief captures the screen
 *  \param dev handle to capture device
 */
void CapturyProcessFrame(captury_t *dev) {
	if (!dev->FRunning)
		dev->prepareProcessing();

	dev->FCompositing = false;

	if (dev->FVideoSource->initializeFrameProcessing() == CAPTURY_DENIED)
		return;

	dev->FVideoSource->captureScreen();

	if (dev->FConfig.cursor)
		dev->FVideoSource->captureCursor();

	dev->FHeadFrame->video.id = CapseoStreamCreateFrameID(dev->FStream);

	dev->FFrames.head_advance();
}

// {{{ Incremental Frame Processing
/*! initiates region based capturing.
 *  \param dev capture device handle which we're about to capture the frame for
 *  \retval CAPTURY_SUCCESS passing regions (and finally committing them) is allowed)
 *  \retval CAPTURY_DENIED passing regions and committing them is not permitted 
 *                         Mostly due to performance shortcomings. 
 *                         So try again on next frame.
 *
 *  \see CapturyProcessRegion(), CapturyProcessRegionCommit(), CapturyProcessFrame()
 */
int CapturyProcessRegionStart(captury_t *dev) {
	int rv = dev->FVideoSource->initializeFrameProcessing();
	if (dev->FCompositing || rv != CAPTURY_SUCCESS)
		return rv;

	if (!dev->FRunning)
		dev->prepareProcessing();

	dev->FCompositing = true;

	dev->FVideoSource->captureScreen();

	dev->FHeadFrame->video.id = CapseoStreamCreateFrameID(dev->FStream);
	memcpy(dev->FCompositeBuffer, dev->FHeadFrame->video.buffer, dev->FFrameLength);

	dev->FFrames.head_advance();

	return CAPTURY_DENIED;
}

/*! \brief processes a frame incremental.
 *  \param cd the captury device handle.
 *  \param x lower absolute x capture coordinate.
 *  \param y lower absolute y capture coordinate.
 *  \param width upper x capture coordinate (relative to x)
 *  \param height upper y capture coordinat (relative to y)
 *  \retval CAPTURY_SUCCESS success.
 *
 * Suppose the case you know exactly what regions of your screen have been
 * damaged, than you can use CapturyProcessFrameRegion() to exactly pass
 * the coordinates of which parts have been damaged.
 * 
 * You may invoke this method on more damaged regions per frame.
 * use CapturyProcessFrameCommit() to actually pass final frame to the encoder.
 *
 * \remarks The very first frame <b>must</b> be captured fully by 
 *          CapturyProcessFrame(). As CapturyProcessFrameRegion() will always 
 *          operate on the last frame.
 *
 * \see CapturyProcessRegionStart(), CapturyProcessRegionCommit(), CapturyProcessFrame()
 */
int CapturyProcessRegion(captury_t *dev, int x, int y, int width, int height) {
	if (!dev->FCompositing)
		return CAPTURY_E_INVALID;

	dev->FVideoSource->captureRegion(x, y, width, height);

	return CAPTURY_SUCCESS;
}

/*! \brief commits the incremental captured screen to the encoder
 *  \param dev the captury device handle
 *
 *  When having captured all frame regions that have been damaged,
 *  use this method to actually pass this frame to the encoder.
 *
 *  \see CapturyProcessRegion(), CapturyProcessRegionStart(), CapturyProcessFrame()
 */
void CapturyProcessRegionCommit(captury_t *dev) {
	TCapturyFrame *frame = dev->FHeadFrame;

	if (dev->FConfig.cursor)
		dev->FVideoSource->captureCursor();

	memcpy(frame->video.buffer, dev->FCompositeBuffer, dev->FFrameLength);
	frame->video.id = CapseoStreamCreateFrameID(dev->FStream);

	dev->FFrames.head_advance();
}
// }}}

// vim:ai:noet:ts=4:nowrap
