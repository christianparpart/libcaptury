INSTALL_HEADER = $(INSTALL_DATA) -p

INCLUDES = -I$(top_srcdir)/src

AM_CXXFLAGS = -ansi -pedantic

AM_LDFLAGS = $(top_builddir)/src/captury/libcaptury.la -lGL $(X11_LIBS)

if EXAMPLES

noinst_PROGRAMS = capture-screen capture-region

capture_screen_SOURCES = capture-screen.cpp
capture_region_SOURCES = capture-region.cpp

endif

# vim:ai:noet:ts=4:nowrap
