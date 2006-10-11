EXTRA_SUBDIRS = include

noinst_LIBRARIES = libarch_sdl.a

INCLUDES = -I$(top_srcdir)/arch/include -I$(top_srcdir)/include -I$(top_srcdir)/main -I$(top_srcdir)/3d

if !USE_LINUX_JOY
JOYSTICK_SRCS = joy.c joydefs.c
endif

INPUT_SRCS = event.c key.c mouse.c

if !USE_OPENGL
VIDEO_SRCS = gr.c
endif

libarch_sdl_a_SOURCES = ${JOYSTICK_SRCS} ${VIDEO_SRCS} ${INPUT_SRCS} \
digi.c init.c rbaudio.c timer.c

EXTRA_libarch_sdl_a_SOURCES = \
event.c gr.c joy.c joydefs.c key.c mouse.c
