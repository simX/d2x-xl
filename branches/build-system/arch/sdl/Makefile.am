INCLUDES = -I$(top_srcdir)/arch/include -I$(top_srcdir)/include -I$(top_srcdir)/main -I$(top_srcdir)/3d

noinst_LIBRARIES = libarch_sdl.a

if !USE_LINUX_JOY
JOYSTICK_SRCS = joy.c joydefs.c
endif

INPUT_SRCS = \
	event.c \
	key.c \
	mouse.c

if !USE_OPENGL
VIDEO_SRCS = gr.c
endif

libarch_sdl_a_SOURCES = \
	digi.c \
	init.c \
	rbaudio.c \
	timer.c \
	${JOYSTICK_SRCS} \
	${VIDEO_SRCS} \
	${INPUT_SRCS}
