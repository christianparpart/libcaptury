/////////////////////////////////////////////////////////////////////////////
//
//  Captury - http://rm-rf.in/captury
//  $Id$
//  (example program for capturing full application window)
//
//  Copyright (c) 2007 by Christian Parpart <trapni@gentoo.org>
//
//  This file as well as its whole library is licensed under
//  the terms of GPL. See the file COPYING.
//
/////////////////////////////////////////////////////////////////////////////
#include <captury/captury.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

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

int span = 400;
int scale = 0;

int main(int argc, char *argv[]) {
	for (int ch; (ch = getopt(argc, argv, "c:s:")) != -1; ) {
		switch (ch) {
			case 'c':
				scale = atoi(optarg);
				break;
			case 's':
				span = atoi(optarg);
				break;
			default:
				break;
		}
	}

	Display *dpy = XOpenDisplay(NULL);
	Window win = createWindow(dpy, span, span);

	captury_config_t config;
	bzero(&config, sizeof(config));

	config.x = 0;
	config.y = 0;
	config.width = span;
	config.height = span;
	config.scale = scale;
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
	float step = (float) 1.0 / max;

	float color[3] = { 0.0, 1.0, 0.5 };
	for (;;) {
//	for (time_t start = time(0); time(0) - start < 30; ) { // let's play for 30 seconds
		if (color[0] > 1.0)
			step = -step;
		else if (color[0] < 0.0)
			step = -step;

		color[0] += step;
		color[1] -= step;

		glClearColor(color[0], color[1], color[2], 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		CapturyProcessFrame(cd);

		glXSwapBuffers(dpy, win);
	}

	CapturyClose(cd);

	return 0;
}
