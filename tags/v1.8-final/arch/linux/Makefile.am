EXTRA_SUBDIRS = include

noinst_LIBRARIES = libarch_linux.a

INCLUDES = -I$(top_srcdir)/arch/include -I$(top_srcdir)/include -I$(top_srcdir)/main -I$(srcdir)/include -I$(top_srcdir)/3d

if USE_NETWORK
NETWORK_SRCS = ipx_mcast4.c ipx_udp.c linuxnet.c
if USE_NATIVE_IPX
IPX_SRCS = ipx_bsd.c
endif
if USE_KALINIX
KALI_SRCS = ipx_kali.c ukali.c
endif
endif

if USE_LINUX_JOY
JOYSTICK_SRCS = joystick.c joydefs.c
endif

libarch_linux_a_SOURCES = ${NETWORK_SRCS} ${IPX_SRCS} ${KALI_SRCS} ${JOYSTICK_SRCS} init.c findfile.c midi.c hmpfile.c

EXTRA_libarch_linux_a_SOURCES = \
ipx_bsd.c ipx_kali.c ipx_mcast4.c ipx_udp.c linuxnet.c ukali.c \
joydefs.c joystick.c

EXTRA_DIST = ${EXTRA_SUBDIRS} \
alsadigi.c hmistub.c hmiplay.c mono.c serial.c timer.c
