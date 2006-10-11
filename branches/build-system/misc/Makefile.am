INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/main -I$(top_srcdir)/3d

noinst_LIBRARIES = libmisc.a

libmisc_a_SOURCES = \
	args.c \
	d_io.c \
	error.c \
	hash.c \
	strio.c \
	strutil.c \
	${PNG_SRCS}

noinst_HEADERS = \
	dos_disk.h \
	parsarg.h

EXTRA_DIST = \
	byteswap.c \
	errtest.c \
	parsarg.c \
	parstest.c
