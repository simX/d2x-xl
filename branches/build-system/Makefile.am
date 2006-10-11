AUTOMAKE_OPTIONS = dist-bzip2

if CONSOLE
OPT_SUBDIRS = console
endif

SUBDIRS = 2d 3d arch cfile iff libmve main maths mem misc texmap ${OPT_SUBDIRS}

bin_PROGRAMS = @TARGETS@
EXTRA_PROGRAMS = d2x-xl

ARCH_LIBS = $(foreach subdir,${D2X_ARCH_SUBDIRS},arch/${subdir}/libarch_${subdir}.a)

if CONSOLE
CONSOLE_LIBS = console/libconsole.a
endif

d2x_LDADD = main/libmain.a 2d/lib2d.a 3d/lib3d.a ${ARCH_LIBS} \
	libmve/libmve.a mem/libmem.a iff/libiff.a texmap/libtexmap.a \
	misc/libmisc.a maths/libmaths.a cfile/libcfile.a ${CONSOLE_LIBS}

if MINGW32
if USE_NETWORK
d2x_sdl_LDADD = ${d2x_LDADD} -lwinmm -lwsock32
else
d2x_sdl_LDADD = ${d2x_LDADD} -lwinmm
endif
else
d2x_sdl_LDADD = ${d2x_LDADD} -lm
endif

d2x_xl_SOURCES =
d2x_xl_LDADD = ${d2x_sdl_LDADD} ${OGL_LIBS}

pkgdatadir = @sharepath@
missiondir = ${pkgdatadir}/missions

EXTRA_DIST = \
	d2x-xl.xcodeproj \
	include \
	shaders \
	src \
	unused \
	VisualC \
	autogen.sh \
	readme.txt

dist-hook:
	rm -rf `find $(distdir) -name .svn`
