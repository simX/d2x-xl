INCLUDES = -I$(top_srcdir)/arch/include -I$(top_srcdir)/include -I$(top_srcdir)/main -I$(srcdir)/include -I$(top_srcdir)/3d

noinst_LIBRARIES = libarch_linux.a

if USE_NETWORK

NETWORK_SRCS = ipx_mcast4.c ipx_udp.c linuxnet.c

if USE_NATIVE_IPX
IPX_SRCS = ipx_bsd.c
endif

if USE_KALINIX
KALI_SRCS = ipx_kali.c ukali.c
endif

endif # USE_NETWORK

if USE_LINUX_JOY
  JOYSTICK_SRCS = joystick.c joydefs.c
endif

libarch_linux_a_SOURCES = \
	findfile.c \
	hmpfile.c \
	init.c \
	midi.c \
	${NETWORK_SRCS} \
	${IPX_SRCS} \
	${KALI_SRCS} \
	${JOYSTICK_SRCS}

EXTRA_DIST = \
	include \
	alsadigi.c \
	hmiplay.c \
	hmistub.c \
	mono.c \
	serial.c \
	timer.c
