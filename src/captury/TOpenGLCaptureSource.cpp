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
#include "TOpenGLCaptureSource.h"

#include "captury.h"
#include "TCapturyFrame.h"
#include "TCapturyHandle.h"
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

#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

typedef void (*GL_READPIXELS_FN)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
static GL_READPIXELS_FN glReadPixels_ = NULL;

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

TOpenGLCaptureSource::TOpenGLCaptureSource(TCapturyHandle *AHandle) :
	TVideoCaptureSource(AHandle) {
}

TOpenGLCaptureSource::~TOpenGLCaptureSource() {
}

bool TOpenGLCaptureSource::ensureCaptureFunction() {
	return glReadPixels_
		? true
		: (glReadPixels_ = (GL_READPIXELS_FN)dlsym(RTLD_DEFAULT, "glReadPixels")) != NULL;
}

void TOpenGLCaptureSource::captureScreen() {
	if (!ensureCaptureFunction())
		return;

	(*glReadPixels_)(dev->FConfig.x, dev->FConfig.y, dev->FConfig.width, dev->FConfig.height, GL_BGRA, GL_UNSIGNED_BYTE, dev->FHeadFrame->video.buffer);
}

void TOpenGLCaptureSource::captureRegion(int x, int y, int width, int height) {
	(*glReadPixels_)(x, y, width, height, GL_BGRA, GL_UNSIGNED_BYTE, dev->FRegionBuffer);

	mergeImage((uint32_t *)dev->FCompositeBuffer, dev->FConfig.width, (uint32_t *)dev->FRegionBuffer, x, y, width, height);
}

/*! \brief captures cursor
 */
void TOpenGLCaptureSource::captureCursor() {
	Display *dpy = dev->FConfig.deviceHandle;
	if (!dpy) {
		dev->FConfig.cursor = false;
		return;
	}

	Window src_w = DefaultRootWindow(dpy);
	Window dst_w = dev->FConfig.windowHandle;
	if (!src_w || !dst_w) {
		dev->FConfig.cursor = false;
		return;
	}

	XFixesCursorImage *cursor = XFixesGetCursorImage(dpy);
	if (!cursor) {
		dev->FConfig.cursor = false;
		return;
	}

	int cx, cy;
	Window target_unused;
	XTranslateCoordinates(dpy, src_w, dst_w, cursor->x, cursor->y, &cx, &cy, &target_unused);

	if (cx < 0 || cy < 0 || cx >= dev->FConfig.width || cy >= dev->FConfig.height) {
		XFree(cursor);
		return; // cursor out of our window
	}

	if (cx == dev->FCursorX && cy == dev->FCursorY && cursor->cursor_serial == dev->FCursorSerial) {
		XFree(cursor);
		dev->FCursorChanged = false;
		return; // cursor didn't move or changed shape
	}

	dev->FCursorChanged = true;
	dev->FCursorX = cx;
	dev->FCursorY = cy;
	dev->FCursorSerial = cursor->cursor_serial;

	// XXX pass cursor buffer to the codec
	dev->FHeadFrame->cursor.x = cx;
	dev->FHeadFrame->cursor.y = dev->FConfig.height - cy; // correct y-value (top-down)
	dev->FHeadFrame->cursor.width = cursor->width;
	dev->FHeadFrame->cursor.height = cursor->height;

	uint32_t *dptr = (uint32_t *)dev->FHeadFrame->cursor.buffer;
	for (unsigned long *lptr = cursor->pixels; lptr < cursor->pixels + cursor->width * cursor->height;)
		*dptr++ = *lptr++;

	XFree(cursor);
}

// vim:ai:noet:ts=4:nowrap
