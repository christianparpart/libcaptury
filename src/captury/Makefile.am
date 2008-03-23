INSTALL_HEADER = $(INSTALL_DATA) -p

INCLUDES = -I$(top_srcdir)/src

PTHREAD_CFLAGS = -pthread
PTHREAD_LIBS = -lpthread

ANSI_CXXFLAGS = -ansi -pedantic -Wall -Wno-long-long -Wno-unknown-pragmas

AM_CXXFLAGS = \
	$(ANSI_CXXFLAGS) $(PTHREAD_CFLAGS) $(CAPSEO_CFLAGS) \
	$(X11_CFLAGS) $(XFIXES_CFLAGS)

lib_LTLIBRARIES = libcaptury.la

libcaptury_la_LDFLAGS = \
	-version-info $(LIBCAPTURY_VERSION_INFO) \
	$(PTHREAD_LIBS) $(CAPSEO_LIBS) \
	$(X11_LIBS) $(XFIXES_LIBS)

libcaptury_la_SOURCES = \
	captury.cpp \
	TCapturyFrame.cpp TCapturyFrame.h \
	TCapturyHandle.cpp TCapturyHandle.h \
	TVideoCaptureSource.cpp TVideoCaptureSource.h \
	TOpenGLCaptureSource.cpp TOpenGLCaptureSource.h \
	TAudioCaptureSource.cpp TAudioCaptureSource.h \
	codec.cpp codec.h

capturydir = @includedir@/captury
captury_HEADERS = captury.h

# vim:ai:noet:ts=4:nowrap
