noinst_LIBRARIES = libmisc.a
INCLUDES = -I $(top_srcdir)/include -I $(top_srcdir)/main -I $(top_srcdir)/3d

if USE_LIBPNG
PNG_SRCS = pngfile.c
endif

libmisc_a_SOURCES = ${PNG_SRCS} \
args.c      error.c    strio.c    hash.c     strutil.c	d_io.c

EXTRA_libmisc_a_SOURCES = pngfile.c

# checker.c

#checker.o: checker.c
#	gcc -c -g $< -o $@ $(INCLUDES) $(CFLAGS) -I/usr/lib/checker/i686-pc-linux-gnu/include -I.. -I.

EXTRA_DIST = \
dos_disk.h parsarg.h \
byteswap.c errtest.c parsarg.c parstest.c
