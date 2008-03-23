/////////////////////////////////////////////////////////////////////////////
//
//  Captury - http://rm-rf.in/captury
//  $Id$
//  (example program for capturing by region)
//
//  Copyright (c) 2007 by Christian Parpart <trapni@gentoo.org>
//
//  This file as well as its whole library is licensed under
//  the terms of GPL. See the file COPYING.
//
/////////////////////////////////////////////////////////////////////////////
#include <captury/captury.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <GL/glx.h>

// {{{ X11/GLX
static Bool waitMapNotify(Display *dpy, XEvent *e, char *arg) {
	return dpy && (e->type == MapNotify) && (e->xmap.window == (Window) arg);
}

static Bool waitConfigureNotify(Display *dpy, XEvent *e, char *arg) {
	return dpy && (e->type == ConfigureNotify) && (e->xconfigure.window == (Window) arg);
}

static Window createWindow(Display *dpy, int width, int height) {
	GLXFBConfig *fbc;
	XVisualInfo *vi;
	Colormap cmap;
	XSetWindowAttributes swa;
	GLXContext cx;
	XEvent event;
	int nElements;

	int attr[] = { GLX_DOUBLEBUFFER, True, None };

	fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), attr, &nElements);
	vi = glXGetVisualFromFBConfig(dpy, fbc[0]);

	cx = glXCreateNewContext(dpy, fbc[0], GLX_RGBA_TYPE, 0, GL_FALSE);
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);

	swa.colormap = cmap;
	swa.border_pixel = 0;
	swa.event_mask = 0;
	Window win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
	XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask | KeyReleaseMask);
	XMapWindow(dpy, win);
	XIfEvent(dpy, &event, waitMapNotify, (char *)win);

	XMoveWindow(dpy, win, 100, 100);
	XIfEvent(dpy, &event, waitConfigureNotify, (char *)win);

	glXMakeCurrent(dpy, win, cx);
	return win;
}
// }}}

int die(const char *msg) {
	fprintf(stderr, "error: %s\n", msg);
	return 1;
}

int main(int argc, char *argv[]) {
	Display *dpy = XOpenDisplay(NULL);
	Window win = createWindow(dpy, 400, 400);

	captury_config_t config;
	bzero(&config, sizeof(config));

	config.x = 0;
	config.y = 0;
	config.width = 400;
	config.height = 400;
	config.scale = 0; // do not scale
	config.fps = 25.0; // average fps

	config.deviceType = CAPTURY_DEVICE_GLX;
	config.deviceHandle = dpy;
	config.windowHandle = win;
	config.cursor = true;

	captury_t *cd = CapturyOpen(&config);
	if (!cd)
		return die("cannot open capture device");

	if (CapturySetOutputFileName(cd, "example.cps") == -1)
		return die(strerror(errno));

	const int max = 10000;
	const float step = (float) 1.0 / max;

	float color[3] = { 0.0, 1.0, 0.5 };

	for (int i = 0; i < max; ++i) {
		color[0] += step;
		color[1] -= step;

		glClearColor(color[0], color[1], color[2], 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		const int in = 50;

		// XXX please note, you may (but it is no must) capture full frames before
		// XXX afterwards, or between. You can switch your capturing method at runtime
		// XXX just the way you feel like.
#if 0
		if (i < 400) {
			// capture completely
			CapturyProcessFrame(cd);
		}
		else
#endif
		if (CapturyProcessRegionStart(cd) == CAPTURY_SUCCESS) {
			// the core grants our wish: we may send him new regions to be captured.

			if (i < max / 2) {
				// first 50% we just re-capture the inner set
				CapturyProcessRegion(cd, config.x + in, config.y + in, config.width - 2*in, config.height - 2*in);
				CapturyProcessRegionCommit(cd);
			}
			else {
				// second 50% of our show we capture 2 regions
				const int in2 = 20;
				CapturyProcessRegion(cd, config.x + in + in2, config.y + in + in2, in2, in2);
				CapturyProcessRegion(cd, config.width - in - 2*in2, config.height - in - 2*in2, in2, in2);

				CapturyProcessRegionCommit(cd);
			}
		}
		else {
			// CapturyProcessRegionStart() returned CAPTURY_DENIED
			// which merely means, that either the the new frame incoming would be too fast (user specified lower fps)
			// or because the encoder doesn't get enough CPU power to follow us. (the user should specify lower fps then)
		}

		glXSwapBuffers(dpy, win);
	}

	CapturyClose(cd);

	return 0;
}
