# Makefile.in generated by automake 1.9.6 from Makefile.am.
# Makefile.  Generated from Makefile.in by configure.

# Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005  Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.



srcdir = .
top_srcdir = .

pkglibdir = $(libdir)/d2x-xl
pkgincludedir = $(includedir)/d2x-xl
top_builddir = .
am__cd = CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd
INSTALL = /usr/bin/install -c
install_sh_DATA = $(install_sh) -c -m 644
install_sh_PROGRAM = $(install_sh) -c
install_sh_SCRIPT = $(install_sh) -c
INSTALL_HEADER = $(INSTALL_DATA)
transform = $(program_transform_name)
NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_triplet = i686-suse-linux
host_triplet = i686-suse-linux
target_triplet = i686-suse-linux
bin_PROGRAMS = d2x-xl
EXTRA_PROGRAMS = d2x$(EXEEXT) d2x-xl$(EXEEXT) d2x-sdl$(EXEEXT) \
	d2x-gl$(EXEEXT) d2x-svga$(EXEEXT) d2x-ggi$(EXEEXT)
DIST_COMMON = README $(am__configure_deps) $(srcdir)/Makefile.am \
	$(srcdir)/Makefile.in $(srcdir)/conf.h.in \
	$(top_srcdir)/configure AUTHORS COPYING ChangeLog INSTALL NEWS \
	config.guess config.sub depcomp install-sh missing
subdir = .
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
am__aclocal_m4_deps = $(top_srcdir)/acinclude.m4 \
	$(top_srcdir)/configure.ac
am__configure_deps = $(am__aclocal_m4_deps) $(CONFIGURE_DEPENDENCIES) \
	$(ACLOCAL_M4)
am__CONFIG_DISTCLEAN_FILES = config.status config.cache config.log \
 configure.lineno configure.status.lineno
mkinstalldirs = $(install_sh) -d
CONFIG_HEADER = conf.h
CONFIG_CLEAN_FILES =
am__installdirs = "$(DESTDIR)$(bindir)"
binPROGRAMS_INSTALL = $(INSTALL_PROGRAM)
PROGRAMS = $(bin_PROGRAMS)
am_d2x_OBJECTS =
d2x_OBJECTS = $(am_d2x_OBJECTS)
#am__DEPENDENCIES_1 = main/inferno.o
#am__DEPENDENCIES_2 = main/editor/libeditor.a ui/libui.a
am__DEPENDENCIES_3 = $(foreach \
	subdir,${D2X_ARCH_SUBDIRS},arch/${subdir}/libarch_${subdir}.a)
#am__DEPENDENCIES_4 = console/libconsole.a
d2x_DEPENDENCIES = $(am__DEPENDENCIES_1) main/libmain.a \
	$(am__DEPENDENCIES_2) 3d/lib3d.a 2d/lib2d.a \
	$(am__DEPENDENCIES_3) libmve/libmve.a mem/libmem.a \
	iff/libiff.a texmap/libtexmap.a misc/libmisc.a \
	maths/libmaths.a cfile/libcfile.a $(am__DEPENDENCIES_4)
am_d2x_ggi_OBJECTS =
d2x_ggi_OBJECTS = $(am_d2x_ggi_OBJECTS)
am__DEPENDENCIES_5 = $(am__DEPENDENCIES_1) main/libmain.a \
	$(am__DEPENDENCIES_2) 3d/lib3d.a 2d/lib2d.a \
	$(am__DEPENDENCIES_3) libmve/libmve.a mem/libmem.a \
	iff/libiff.a texmap/libtexmap.a misc/libmisc.a \
	maths/libmaths.a cfile/libcfile.a $(am__DEPENDENCIES_4)
am__DEPENDENCIES_6 = $(am__DEPENDENCIES_5)
##am__DEPENDENCIES_6 =  \
##	$(am__DEPENDENCIES_5)
#am__DEPENDENCIES_6 =  \
#	$(am__DEPENDENCIES_5)
am__DEPENDENCIES_7 =
d2x_ggi_DEPENDENCIES = $(am__DEPENDENCIES_6) $(am__DEPENDENCIES_7)
am_d2x_gl_OBJECTS =
d2x_gl_OBJECTS = $(am_d2x_gl_OBJECTS)
d2x_gl_DEPENDENCIES = $(am__DEPENDENCIES_6) $(am__DEPENDENCIES_7)
am_d2x_sdl_OBJECTS =
d2x_sdl_OBJECTS = $(am_d2x_sdl_OBJECTS)
d2x_sdl_DEPENDENCIES = $(am__DEPENDENCIES_5)
##d2x_sdl_DEPENDENCIES =  \
##	$(am__DEPENDENCIES_5)
#d2x_sdl_DEPENDENCIES =  \
#	$(am__DEPENDENCIES_5)
am_d2x_svga_OBJECTS =
d2x_svga_OBJECTS = $(am_d2x_svga_OBJECTS)
d2x_svga_DEPENDENCIES = $(am__DEPENDENCIES_6) $(am__DEPENDENCIES_7)
am_d2x_xl_OBJECTS =
d2x_xl_OBJECTS = $(am_d2x_xl_OBJECTS)
d2x_xl_DEPENDENCIES = $(am__DEPENDENCIES_6) $(am__DEPENDENCIES_7)
DEFAULT_INCLUDES = -I. -I$(srcdir) -I.
COMPILE = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@
SOURCES = $(d2x_SOURCES) $(d2x_ggi_SOURCES) $(d2x_gl_SOURCES) \
	$(d2x_sdl_SOURCES) $(d2x_svga_SOURCES) $(d2x_xl_SOURCES)
DIST_SOURCES = $(d2x_SOURCES) $(d2x_ggi_SOURCES) $(d2x_gl_SOURCES) \
	$(d2x_sdl_SOURCES) $(d2x_svga_SOURCES) $(d2x_xl_SOURCES)
RECURSIVE_TARGETS = all-recursive check-recursive dvi-recursive \
	html-recursive info-recursive install-data-recursive \
	install-exec-recursive install-info-recursive \
	install-recursive installcheck-recursive installdirs-recursive \
	pdf-recursive ps-recursive uninstall-info-recursive \
	uninstall-recursive
ETAGS = etags
CTAGS = ctags
DISTFILES = $(DIST_COMMON) $(DIST_SOURCES) $(TEXINFOS) $(EXTRA_DIST)
distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)
am__remove_distdir = \
  { test ! -d $(distdir) \
    || { find $(distdir) -type d ! -perm -200 -exec chmod u+w {} ';' \
         && rm -fr $(distdir); }; }
DIST_ARCHIVES = $(distdir).tar.gz
GZIP_ENV = --best
distuninstallcheck_listfiles = find . -type f -print
distcleancheck_listfiles = find . -type f -print
#	rm -f `find $(distdir) -name Makefile`
pkgdatadir = /usr/local/share/d2x-xl
ACLOCAL = ${SHELL} /home/dmali/d2x-xl/src/missing --run aclocal-1.9
AMDEP_FALSE = #
AMDEP_TRUE = 
AMTAR = ${SHELL} /home/dmali/d2x-xl/src/missing --run tar
AR = ar
AUTOCONF = ${SHELL} /home/dmali/d2x-xl/src/missing --run autoconf
AUTOHEADER = ${SHELL} /home/dmali/d2x-xl/src/missing --run autoheader
AUTOMAKE = ${SHELL} /home/dmali/d2x-xl/src/missing --run automake-1.9
AWK = gawk
CC = gcc
CCAS = gcc
CCASFLAGS = -I $(top_srcdir)/arch/linux/include -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -pipe -O2 -Wall -Wno-char-subscripts  
CCDEPMODE = depmode=gcc3
CFLAGS = -I $(top_srcdir)/arch/linux/include -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -pipe -O2 -Wall -Wno-char-subscripts  
CONSOLE_FALSE = 
CONSOLE_TRUE = #
CPP = gcc -E
CPPFLAGS = 
CXX = g++
CXXDEPMODE = depmode=gcc3
CXXFLAGS = -g -O2
CYGPATH_W = echo
D2X_ARCH_SUBDIRS = linux ogl sdl
D2X_MAIN_SUBDIRS = 
D2X_SUBDIRS = 
DEFS = -DHAVE_CONFIG_H
DEPDIR = .deps
ECHO_C = 
ECHO_N = -n
ECHO_T = 
EDITOR_FALSE = 
EDITOR_TRUE = #
EGREP = grep -E
EXEEXT = 
GGI_LIBS = 
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_STRIP_PROGRAM = ${SHELL} $(install_sh) -c -s
LDFLAGS = 
LIBOBJS = 
LIBS = -lSDL_mixer -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL -lpthread 
LTLIBOBJS = 
MACDATA_FALSE = 
MACDATA_TRUE = #
MACOSX_FALSE = 
MACOSX_TRUE = #
MAINT = #
MAINTAINER_MODE_FALSE = 
MAINTAINER_MODE_TRUE = #
MAKEINFO = ${SHELL} /home/dmali/d2x-xl/src/missing --run makeinfo
MINGW32_FALSE = 
MINGW32_TRUE = #
NASM = no
NASMFLAGS = 
OBJEXT = o
OGL_LIBS =  -lGL -lGLU
PACKAGE = d2x-xl
PACKAGE_BUGREPORT = karx11erx@hotmail.com
PACKAGE_NAME = d2x-xl
PACKAGE_STRING = d2x-xl 1.6.98
PACKAGE_TARNAME = d2x-xl
PACKAGE_VERSION = 1.6.98
PATH_SEPARATOR = :
RANLIB = ranlib
SDL_CFLAGS = -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDL_CONFIG = /usr/local/bin/sdl-config
SDL_LIBS = -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lSDL -lpthread
SET_MAKE = 
SHELL = /bin/sh
STRIP = strip
SVGA_LIBS = 
TARGETS = d2x-xl
USE_ASM_FALSE = 
USE_ASM_TRUE = #
USE_GGI_FALSE = 
USE_GGI_TRUE = #
USE_KALINIX_FALSE = #
USE_KALINIX_TRUE = 
USE_LIBPNG_FALSE = 
USE_LIBPNG_TRUE = #
USE_LINUX_JOY_FALSE = 
USE_LINUX_JOY_TRUE = #
USE_NATIVE_IPX_FALSE = #
USE_NATIVE_IPX_TRUE = 
USE_NETWORK_FALSE = #
USE_NETWORK_TRUE = 
USE_OPENGL_FALSE = #
USE_OPENGL_TRUE = 
USE_SVGALIB_FALSE = 
USE_SVGALIB_TRUE = #
VERSION = 1.6.98
ac_ct_AR = ar
ac_ct_CC = gcc
ac_ct_CXX = g++
ac_ct_RANLIB = ranlib
ac_ct_STRIP = strip
am__fastdepCC_FALSE = #
am__fastdepCC_TRUE = 
am__fastdepCXX_FALSE = #
am__fastdepCXX_TRUE = 
am__include = include
am__leading_dot = .
am__quote = 
am__tar = ${AMTAR} chof - "$$tardir"
am__untar = ${AMTAR} xf -
bindir = ${exec_prefix}/bin
build = i686-suse-linux
build_alias = 
build_cpu = i686
build_os = linux
build_vendor = suse
datadir = ${prefix}/share
exec_prefix = /usr/local
host = i686-suse-linux
host_alias = 
host_cpu = i686
host_os = linux
host_vendor = suse
includedir = ${prefix}/include
infodir = ${prefix}/info
install_sh = /home/dmali/d2x-xl/src/install-sh
libdir = ${exec_prefix}/lib
libexecdir = ${exec_prefix}/libexec
localstatedir = ${prefix}/var
mandir = ${prefix}/man
mkdir_p = mkdir -p --
oldincludedir = /usr/include
prefix = /usr/local
program_transform_name = s,x,x,
sbindir = ${exec_prefix}/sbin
sharedstatedir = ${prefix}/com
sharepath = /usr/local/share/d2x-xl
sysconfdir = ${prefix}/etc
target = i686-suse-linux
target_alias = 
target_cpu = i686
target_os = linux
target_vendor = suse
DIST_SUBDIRS = 2d 3d arch console iff libmve main maths mem misc texmap cfile
SUBDIRS = ${DIST_SUBDIRS} 
EXTRA_SUBDIRS = ui include unused debian rpm VisualC VisualCE English.lproj d2x.xcode
ARCH_LIBS = $(foreach subdir,${D2X_ARCH_SUBDIRS},arch/${subdir}/libarch_${subdir}.a)
#LD_KLUDGE = main/inferno.o
#AM_LDFLAGS = -all_load
#EDITOR_LIBS = main/editor/libeditor.a ui/libui.a
#CONSOLE_LIBS = console/libconsole.a
d2x_SOURCES = 
d2x_sdl_SOURCES = 
d2x_xl_SOURCES = 
d2x_gl_SOURCES = 
d2x_svga_SOURCES = 
d2x_ggi_SOURCES = 
d2x_LDADD = ${LD_KLUDGE} main/libmain.a ${EDITOR_LIBS} 3d/lib3d.a 2d/lib2d.a ${ARCH_LIBS} libmve/libmve.a mem/libmem.a iff/libiff.a texmap/libtexmap.a misc/libmisc.a maths/libmaths.a cfile/libcfile.a ${CONSOLE_LIBS}
d2x_sdl_LDADD = ${d2x_LDADD} -lm
##d2x_sdl_LDADD = ${d2x_LDADD} -lwinmm
#d2x_sdl_LDADD = ${d2x_LDADD} -lwinmm -lwsock32
d2x_xl_LDADD = ${d2x_sdl_LDADD} ${OGL_LIBS}
d2x_gl_LDADD = ${d2x_sdl_LDADD} ${OGL_LIBS}
d2x_svga_LDADD = ${d2x_sdl_LDADD} ${SVGA_LIBS}
d2x_ggi_LDADD = ${d2x_sdl_LDADD} ${GGI_LIBS}
missiondir = ${pkgdatadir}/missions
CLEANFILES = d2x-xl Info.plist
EXTRA_DIST = ${EXTRA_SUBDIRS} \
ChangeLog ChangeLog-old \
D2X.make \
MVEPlayer-Info.plist d2x-Info.plist d2xgl-Info.plist \
README.utils \
djgpp.bat djgpp.sh \
autogen.sh \
cvshowto.txt readme.txt installation.txt compilerdefs.txt \
d2x-sdl.sh d2x-gl.sh  d2x-xl.sh

all: conf.h
	$(MAKE) $(AM_MAKEFLAGS) all-recursive

.SUFFIXES:
am--refresh:
	@:
$(srcdir)/Makefile.in: # $(srcdir)/Makefile.am  $(am__configure_deps)
	@for dep in $?; do \
	  case '$(am__configure_deps)' in \
	    *$$dep*) \
	      echo ' cd $(srcdir) && $(AUTOMAKE) --gnu '; \
	      cd $(srcdir) && $(AUTOMAKE) --gnu  \
		&& exit 0; \
	      exit 1;; \
	  esac; \
	done; \
	echo ' cd $(top_srcdir) && $(AUTOMAKE) --gnu  Makefile'; \
	cd $(top_srcdir) && \
	  $(AUTOMAKE) --gnu  Makefile
.PRECIOUS: Makefile
Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status
	@case '$?' in \
	  *config.status*) \
	    echo ' $(SHELL) ./config.status'; \
	    $(SHELL) ./config.status;; \
	  *) \
	    echo ' cd $(top_builddir) && $(SHELL) ./config.status $@ $(am__depfiles_maybe)'; \
	    cd $(top_builddir) && $(SHELL) ./config.status $@ $(am__depfiles_maybe);; \
	esac;

$(top_builddir)/config.status: $(top_srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	$(SHELL) ./config.status --recheck

$(top_srcdir)/configure: # $(am__configure_deps)
	cd $(srcdir) && $(AUTOCONF)
$(ACLOCAL_M4): # $(am__aclocal_m4_deps)
	cd $(srcdir) && $(ACLOCAL) $(ACLOCAL_AMFLAGS)

conf.h: stamp-h1
	@if test ! -f $@; then \
	  rm -f stamp-h1; \
	  $(MAKE) stamp-h1; \
	else :; fi

stamp-h1: $(srcdir)/conf.h.in $(top_builddir)/config.status
	@rm -f stamp-h1
	cd $(top_builddir) && $(SHELL) ./config.status conf.h
$(srcdir)/conf.h.in: # $(am__configure_deps) 
	cd $(top_srcdir) && $(AUTOHEADER)
	rm -f stamp-h1
	touch $@

distclean-hdr:
	-rm -f conf.h stamp-h1
install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	test -z "$(bindir)" || $(mkdir_p) "$(DESTDIR)$(bindir)"
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  if test -f $$p \
	  ; then \
	    f=`echo "$$p1" | sed 's,^.*/,,;$(transform);s/$$/$(EXEEXT)/'`; \
	   echo " $(INSTALL_PROGRAM_ENV) $(binPROGRAMS_INSTALL) '$$p' '$(DESTDIR)$(bindir)/$$f'"; \
	   $(INSTALL_PROGRAM_ENV) $(binPROGRAMS_INSTALL) "$$p" "$(DESTDIR)$(bindir)/$$f" || exit 1; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  f=`echo "$$p" | sed 's,^.*/,,;s/$(EXEEXT)$$//;$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " rm -f '$(DESTDIR)$(bindir)/$$f'"; \
	  rm -f "$(DESTDIR)$(bindir)/$$f"; \
	done

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)
d2x$(EXEEXT): $(d2x_OBJECTS) $(d2x_DEPENDENCIES) 
	@rm -f d2x$(EXEEXT)
	$(LINK) $(d2x_LDFLAGS) $(d2x_OBJECTS) $(d2x_LDADD) $(LIBS)
d2x-ggi$(EXEEXT): $(d2x_ggi_OBJECTS) $(d2x_ggi_DEPENDENCIES) 
	@rm -f d2x-ggi$(EXEEXT)
	$(LINK) $(d2x_ggi_LDFLAGS) $(d2x_ggi_OBJECTS) $(d2x_ggi_LDADD) $(LIBS)
d2x-gl$(EXEEXT): $(d2x_gl_OBJECTS) $(d2x_gl_DEPENDENCIES) 
	@rm -f d2x-gl$(EXEEXT)
	$(LINK) $(d2x_gl_LDFLAGS) $(d2x_gl_OBJECTS) $(d2x_gl_LDADD) $(LIBS)
d2x-sdl$(EXEEXT): $(d2x_sdl_OBJECTS) $(d2x_sdl_DEPENDENCIES) 
	@rm -f d2x-sdl$(EXEEXT)
	$(LINK) $(d2x_sdl_LDFLAGS) $(d2x_sdl_OBJECTS) $(d2x_sdl_LDADD) $(LIBS)
d2x-svga$(EXEEXT): $(d2x_svga_OBJECTS) $(d2x_svga_DEPENDENCIES) 
	@rm -f d2x-svga$(EXEEXT)
	$(LINK) $(d2x_svga_LDFLAGS) $(d2x_svga_OBJECTS) $(d2x_svga_LDADD) $(LIBS)
d2x-xl$(EXEEXT): $(d2x_xl_OBJECTS) $(d2x_xl_DEPENDENCIES) 
	@rm -f d2x-xl$(EXEEXT)
	$(LINK) $(d2x_xl_LDFLAGS) $(d2x_xl_OBJECTS) $(d2x_xl_LDADD) $(LIBS)

mostlyclean-compile:
	-rm -f *.$(OBJEXT)

distclean-compile:
	-rm -f *.tab.c
uninstall-info-am:

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.
$(RECURSIVE_TARGETS):
	@failcom='exit 1'; \
	for f in x $$MAKEFLAGS; do \
	  case $$f in \
	    *=* | --[!k]*);; \
	    *k*) failcom='fail=yes';; \
	  esac; \
	done; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	  || eval $$failcom; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@failcom='exit 1'; \
	for f in x $$MAKEFLAGS; do \
	  case $$f in \
	    *=* | --[!k]*);; \
	    *k*) failcom='fail=yes';; \
	  esac; \
	done; \
	dot_seen=no; \
	case "$@" in \
	  distclean-* | maintainer-clean-*) list='$(DIST_SUBDIRS)' ;; \
	  *) list='$(SUBDIRS)' ;; \
	esac; \
	rev=''; for subdir in $$list; do \
	  if test "$$subdir" = "."; then :; else \
	    rev="$$subdir $$rev"; \
	  fi; \
	done; \
	rev="$$rev ."; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	  || eval $$failcom; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done
ctags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) ctags); \
	done

ID: $(HEADERS) $(SOURCES) $(LISP) $(TAGS_FILES)
	list='$(SOURCES) $(HEADERS) $(LISP) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	mkid -fID $$unique
tags: TAGS

TAGS: tags-recursive $(HEADERS) $(SOURCES) conf.h.in $(TAGS_DEPENDENCIES) \
		$(TAGS_FILES) $(LISP)
	tags=; \
	here=`pwd`; \
	if ($(ETAGS) --etags-include --version) >/dev/null 2>&1; then \
	  include_option=--etags-include; \
	  empty_fix=.; \
	else \
	  include_option=--include; \
	  empty_fix=; \
	fi; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  if test "$$subdir" = .; then :; else \
	    test ! -f $$subdir/TAGS || \
	      tags="$$tags $$include_option=$$here/$$subdir/TAGS"; \
	  fi; \
	done; \
	list='$(SOURCES) $(HEADERS) conf.h.in $(LISP) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	if test -z "$(ETAGS_ARGS)$$tags$$unique"; then :; else \
	  test -n "$$unique" || unique=$$empty_fix; \
	  $(ETAGS) $(ETAGSFLAGS) $(AM_ETAGSFLAGS) $(ETAGS_ARGS) \
	    $$tags $$unique; \
	fi
ctags: CTAGS
CTAGS: ctags-recursive $(HEADERS) $(SOURCES) conf.h.in $(TAGS_DEPENDENCIES) \
		$(TAGS_FILES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS) conf.h.in $(LISP) $(TAGS_FILES)'; \
	unique=`for i in $$list; do \
	    if test -f "$$i"; then echo $$i; else echo $(srcdir)/$$i; fi; \
	  done | \
	  $(AWK) '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(CTAGS_ARGS)$$tags$$unique" \
	  || $(CTAGS) $(CTAGSFLAGS) $(AM_CTAGSFLAGS) $(CTAGS_ARGS) \
	     $$tags $$unique

GTAGS:
	here=`$(am__cd) $(top_builddir) && pwd` \
	  && cd $(top_srcdir) \
	  && gtags -i $(GTAGS_ARGS) $$here

distclean-tags:
	-rm -f TAGS ID GTAGS GRTAGS GSYMS GPATH tags

distdir: $(DISTFILES)
	$(am__remove_distdir)
	mkdir $(distdir)
	@srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's|.|.|g'`; \
	list='$(DISTFILES)'; for file in $$list; do \
	  case $$file in \
	    $(srcdir)/*) file=`echo "$$file" | sed "s|^$$srcdirstrip/||"`;; \
	    $(top_srcdir)/*) file=`echo "$$file" | sed "s|^$$topsrcdirstrip/|$(top_builddir)/|"`;; \
	  esac; \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  dir=`echo "$$file" | sed -e 's,/[^/]*$$,,'`; \
	  if test "$$dir" != "$$file" && test "$$dir" != "."; then \
	    dir="/$$dir"; \
	    $(mkdir_p) "$(distdir)$$dir"; \
	  else \
	    dir=''; \
	  fi; \
	  if test -d $$d/$$file; then \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -pR $(srcdir)/$$file $(distdir)$$dir || exit 1; \
	    fi; \
	    cp -pR $$d/$$file $(distdir)$$dir || exit 1; \
	  else \
	    test -f $(distdir)/$$file \
	    || cp -p $$d/$$file $(distdir)/$$file \
	    || exit 1; \
	  fi; \
	done
	list='$(DIST_SUBDIRS)'; for subdir in $$list; do \
	  if test "$$subdir" = .; then :; else \
	    test -d "$(distdir)/$$subdir" \
	    || $(mkdir_p) "$(distdir)/$$subdir" \
	    || exit 1; \
	    distdir=`$(am__cd) $(distdir) && pwd`; \
	    top_distdir=`$(am__cd) $(top_distdir) && pwd`; \
	    (cd $$subdir && \
	      $(MAKE) $(AM_MAKEFLAGS) \
	        top_distdir="$$top_distdir" \
	        distdir="$$distdir/$$subdir" \
	        distdir) \
	      || exit 1; \
	  fi; \
	done
	$(MAKE) $(AM_MAKEFLAGS) \
	  top_distdir="$(top_distdir)" distdir="$(distdir)" \
	  dist-hook
	-find $(distdir) -type d ! -perm -777 -exec chmod a+rwx {} \; -o \
	  ! -type d ! -perm -444 -links 1 -exec chmod a+r {} \; -o \
	  ! -type d ! -perm -400 -exec chmod a+r {} \; -o \
	  ! -type d ! -perm -444 -exec $(SHELL) $(install_sh) -c -m a+r {} {} \; \
	|| chmod -R a+r $(distdir)
dist-gzip: distdir
	tardir=$(distdir) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	$(am__remove_distdir)

dist-bzip2: distdir
	tardir=$(distdir) && $(am__tar) | bzip2 -9 -c >$(distdir).tar.bz2
	$(am__remove_distdir)

dist-tarZ: distdir
	tardir=$(distdir) && $(am__tar) | compress -c >$(distdir).tar.Z
	$(am__remove_distdir)

dist-shar: distdir
	shar $(distdir) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).shar.gz
	$(am__remove_distdir)

dist-zip: distdir
	-rm -f $(distdir).zip
	zip -rq $(distdir).zip $(distdir)
	$(am__remove_distdir)

dist dist-all: distdir
	tardir=$(distdir) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(distdir).tar.gz
	$(am__remove_distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	case '$(DIST_ARCHIVES)' in \
	*.tar.gz*) \
	  GZIP=$(GZIP_ENV) gunzip -c $(distdir).tar.gz | $(am__untar) ;;\
	*.tar.bz2*) \
	  bunzip2 -c $(distdir).tar.bz2 | $(am__untar) ;;\
	*.tar.Z*) \
	  uncompress -c $(distdir).tar.Z | $(am__untar) ;;\
	*.shar.gz*) \
	  GZIP=$(GZIP_ENV) gunzip -c $(distdir).shar.gz | unshar ;;\
	*.zip*) \
	  unzip $(distdir).zip ;;\
	esac
	chmod -R a-w $(distdir); chmod a+w $(distdir)
	mkdir $(distdir)/_build
	mkdir $(distdir)/_inst
	chmod a-w $(distdir)
	dc_install_base=`$(am__cd) $(distdir)/_inst && pwd | sed -e 's,^[^:\\/]:[\\/],/,'` \
	  && dc_destdir="$${TMPDIR-/tmp}/am-dc-$$$$/" \
	  && cd $(distdir)/_build \
	  && ../configure --srcdir=.. --prefix="$$dc_install_base" \
	    $(DISTCHECK_CONFIGURE_FLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) dvi \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) uninstall \
	  && $(MAKE) $(AM_MAKEFLAGS) distuninstallcheck_dir="$$dc_install_base" \
	        distuninstallcheck \
	  && chmod -R a-w "$$dc_install_base" \
	  && ({ \
	       (cd ../.. && umask 077 && mkdir "$$dc_destdir") \
	       && $(MAKE) $(AM_MAKEFLAGS) DESTDIR="$$dc_destdir" install \
	       && $(MAKE) $(AM_MAKEFLAGS) DESTDIR="$$dc_destdir" uninstall \
	       && $(MAKE) $(AM_MAKEFLAGS) DESTDIR="$$dc_destdir" \
	            distuninstallcheck_dir="$$dc_destdir" distuninstallcheck; \
	      } || { rm -rf "$$dc_destdir"; exit 1; }) \
	  && rm -rf "$$dc_destdir" \
	  && $(MAKE) $(AM_MAKEFLAGS) dist \
	  && rm -rf $(DIST_ARCHIVES) \
	  && $(MAKE) $(AM_MAKEFLAGS) distcleancheck
	$(am__remove_distdir)
	@(echo "$(distdir) archives ready for distribution: "; \
	  list='$(DIST_ARCHIVES)'; for i in $$list; do echo $$i; done) | \
	  sed -e '1{h;s/./=/g;p;x;}' -e '$${p;x;}'
distuninstallcheck:
	@cd $(distuninstallcheck_dir) \
	&& test `$(distuninstallcheck_listfiles) | wc -l` -le 1 \
	   || { echo "ERROR: files left after uninstall:" ; \
	        if test -n "$(DESTDIR)"; then \
	          echo "  (check DESTDIR support)"; \
	        fi ; \
	        $(distuninstallcheck_listfiles) ; \
	        exit 1; } >&2
distcleancheck: distclean
	@if test '$(srcdir)' = . ; then \
	  echo "ERROR: distcleancheck can only run from a VPATH build" ; \
	  exit 1 ; \
	fi
	@test `$(distcleancheck_listfiles) | wc -l` -eq 0 \
	  || { echo "ERROR: files left in build directory after distclean:" ; \
	       $(distcleancheck_listfiles) ; \
	       exit 1; } >&2
check-am: all-am
check: check-recursive
all-am: Makefile $(PROGRAMS) conf.h
installdirs: installdirs-recursive
installdirs-am:
	for dir in "$(DESTDIR)$(bindir)"; do \
	  test -z "$$dir" || $(mkdir_p) "$$dir"; \
	done
install: install-recursive
install-exec: install-exec-recursive
install-data: install-data-recursive
uninstall: uninstall-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am

installcheck: installcheck-recursive
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) INSTALL_PROGRAM="$(INSTALL_STRIP_PROGRAM)" \
	  install_sh_PROGRAM="$(INSTALL_STRIP_PROGRAM)" INSTALL_STRIP_FLAG=-s \
	  `test -z '$(STRIP)' || \
	    echo "INSTALL_PROGRAM_ENV=STRIPPROG='$(STRIP)'"` install
mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	@echo "This command is intended for maintainers to use"
	@echo "it deletes files that may require special tools to rebuild."
clean: clean-recursive

clean-am: clean-binPROGRAMS clean-generic mostlyclean-am

distclean: distclean-recursive
	-rm -f $(am__CONFIG_DISTCLEAN_FILES)
	-rm -f Makefile
distclean-am: clean-am distclean-compile distclean-generic \
	distclean-hdr distclean-tags

dvi: dvi-recursive

dvi-am:

html: html-recursive

info: info-recursive

info-am:

install-data-am:

install-exec-am: install-binPROGRAMS

install-info: install-info-recursive

install-man:

installcheck-am:

maintainer-clean: maintainer-clean-recursive
	-rm -f $(am__CONFIG_DISTCLEAN_FILES)
	-rm -rf $(top_srcdir)/autom4te.cache
	-rm -f Makefile
maintainer-clean-am: distclean-am maintainer-clean-generic

mostlyclean: mostlyclean-recursive

mostlyclean-am: mostlyclean-compile mostlyclean-generic

pdf: pdf-recursive

pdf-am:

ps: ps-recursive

ps-am:

uninstall-am: uninstall-binPROGRAMS uninstall-info-am

uninstall-info: uninstall-info-recursive

.PHONY: $(RECURSIVE_TARGETS) CTAGS GTAGS all all-am am--refresh check \
	check-am clean clean-binPROGRAMS clean-generic clean-recursive \
	ctags ctags-recursive dist dist-all dist-bzip2 dist-gzip \
	dist-hook dist-shar dist-tarZ dist-zip distcheck distclean \
	distclean-compile distclean-generic distclean-hdr \
	distclean-recursive distclean-tags distcleancheck distdir \
	distuninstallcheck dvi dvi-am html html-am info info-am \
	install install-am install-binPROGRAMS install-data \
	install-data-am install-exec install-exec-am install-info \
	install-info-am install-man install-strip installcheck \
	installcheck-am installdirs installdirs-am maintainer-clean \
	maintainer-clean-generic maintainer-clean-recursive \
	mostlyclean mostlyclean-compile mostlyclean-generic \
	mostlyclean-recursive pdf pdf-am ps ps-am tags tags-recursive \
	uninstall uninstall-am uninstall-binPROGRAMS uninstall-info-am


dist-hook:
	rm -rf `find $(distdir) -name CVS`
# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
