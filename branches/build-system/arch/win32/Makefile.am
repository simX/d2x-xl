INCLUDES = -I$(top_srcdir)/arch/include -I$(top_srcdir)/include -I$(top_srcdir)/main -I$(srcdir)/include

noinst_LIBRARIES = libarch_win32.a

if USE_NETWORK
NETWORK_SRCS = ipx_mcast4.c ipx_win.c ipx_udp.c winnet.c
endif

libarch_win32_a_SOURCES = \
	hmpfile.c \
	midi.c \
	mingw_init.c
	${NETWORK_SRCS}

EXTRA_DIST = \
	d3dframe \
	include \
	new \
	digi.c \
	findfile.c \
	glinit.c \
	gr.c \
	init.c \
	joydefs.c \
	joyhh.c \
	key.c \
	mono.c \
	mouse.c \
	palw32.c \
	serial.c \
	timer.c \
	win32.c \
	wsocket.c
