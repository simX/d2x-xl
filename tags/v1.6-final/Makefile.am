DIST_SUBDIRS = 2d 3d arch console iff libmve main maths mem misc texmap cfile

SUBDIRS = ${DIST_SUBDIRS} @D2X_SUBDIRS@

EXTRA_SUBDIRS = ui include unused debian rpm VisualC VisualCE English.lproj d2x.xcode

bin_PROGRAMS = @TARGETS@

EXTRA_PROGRAMS = d2x d2x-xl d2x-sdl d2x-gl d2x-svga d2x-ggi

ARCH_LIBS = $(foreach subdir,${D2X_ARCH_SUBDIRS},arch/${subdir}/libarch_${subdir}.a)

if MINGW32
LD_KLUDGE = main/inferno.o
endif

if MACOSX
AM_LDFLAGS = -all_load
endif

if EDITOR
EDITOR_LIBS = main/editor/libeditor.a ui/libui.a
endif

if CONSOLE
CONSOLE_LIBS = console/libconsole.a
endif

d2x_SOURCES =
d2x_sdl_SOURCES =
d2x_xl_SOURCES =
d2x_gl_SOURCES =
d2x_svga_SOURCES =
d2x_ggi_SOURCES =

d2x_LDADD = ${LD_KLUDGE} main/libmain.a ${EDITOR_LIBS} 3d/lib3d.a 2d/lib2d.a ${ARCH_LIBS} libmve/libmve.a mem/libmem.a iff/libiff.a texmap/libtexmap.a misc/libmisc.a maths/libmaths.a cfile/libcfile.a ${CONSOLE_LIBS}

if MINGW32
if USE_NETWORK
d2x_sdl_LDADD = ${d2x_LDADD} -lwinmm -lwsock32
else
d2x_sdl_LDADD = ${d2x_LDADD} -lwinmm
endif
else
d2x_sdl_LDADD = ${d2x_LDADD} -lm
endif
d2x_xl_LDADD = ${d2x_sdl_LDADD} ${OGL_LIBS}
d2x_gl_LDADD = ${d2x_sdl_LDADD} ${OGL_LIBS}
d2x_svga_LDADD = ${d2x_sdl_LDADD} ${SVGA_LIBS}
d2x_ggi_LDADD = ${d2x_sdl_LDADD} ${GGI_LIBS}

dist-hook:
	rm -rf `find $(distdir) -name CVS`
#	rm -f `find $(distdir) -name Makefile`

pkgdatadir = @sharepath@
missiondir = ${pkgdatadir}/missions

CLEANFILES = @TARGETS@ Info.plist

EXTRA_DIST = ${EXTRA_SUBDIRS} \
ChangeLog ChangeLog-old \
D2X.make \
MVEPlayer-Info.plist d2x-Info.plist d2xgl-Info.plist \
README.utils \
djgpp.bat djgpp.sh \
autogen.sh \
cvshowto.txt readme.txt installation.txt compilerdefs.txt \
d2x-sdl.sh d2x-gl.sh  d2x-xl.sh
