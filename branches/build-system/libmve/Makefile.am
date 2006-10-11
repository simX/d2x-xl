INCLUDES = -I$(top_srcdir)/include

noinst_LIBRARIES = libmve.a

libmve_a_SOURCES = \
	decoder16.c \
	decoder8.c \
	mve_audio.c \
	mvelib.c \
	mveplay.c

EXTRA_PROGRAMS = mveplayer

mveplayer_SOURCES = \
	mve_main.c

mveplayer_LDADD = libmve.a

noinst_HEADERS = \
	decoders.h \
	mve_audio.h \
	mvelib.h
