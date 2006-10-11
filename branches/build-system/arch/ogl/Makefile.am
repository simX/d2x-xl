INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/include -I$(top_srcdir)/arch/include -I$(top_srcdir)/main -I$(top_srcdir)/3d

noinst_LIBRARIES = libarch_ogl.a

libarch_ogl_a_SOURCES = \
	gr.c \
	ogl.c \
	ogl_render.c \
	sdlgl.c \
	pbuffer.c \
	fbuffer.c

noinst_HEADERS = \
	extgl.h \
	fog.h

EXTRA_DIST = \
	fog.c \
	glx.c \
	ogl_new.c \
	sdlgl.c \
	wgl.c
