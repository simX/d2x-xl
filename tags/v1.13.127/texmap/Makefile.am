noinst_LIBRARIES = libtexmap.a

INCLUDES = -I $(top_srcdir)/include -I $(top_srcdir)/input/include -I $(top_srcdir)/network/linux/include -I $(top_srcdir)/audio/linux/include -I /usr/local/include/SDL

if !USE_OPENGL
NOGL_SRCS = tmapflat.cpp
if USE_ASM
ASM_SRCS = tmappent.S tmapppro.S # more hacked in with LIBADD below
endif
endif

libtexmap_a_SOURCES = ${NOGL_SRCS} ${ASM_SRCS} \
ntmap.cpp scanline.cpp

SUFFIXES = .asm
%.o: %.asm
	$(NASM) $(NASMFLAGS) $< -o $@

if !USE_OPENGL
if USE_ASM
libtexmap_a_LIBADD = tmap_flt.o tmap_lin.o tmap_ll.o tmap_per.o tmapfade.o
endif
endif

EXTRA_libtexmap_a_SOURCES = tmapflat.cpp tmappent.S tmapppro.S

EXTRA_DIST = \
tmap_flt.asm tmap_inc.asm tmap_lin.asm tmap_ll.asm tmap_per.asm tmapfade.asm \
tmappent.S tmapppro.S \
scanline.h texmapl.h \
scanline.cpp
