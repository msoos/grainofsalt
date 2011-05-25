noinst_LTLIBRARIES = libmincov.la
pkgincludesubdir = $(includedir)/espresso/mincov
pkgincludesub_HEADERS = mincov.h  mincov_int.h

AM_CFLAGS = $(all_includes) -I$(srcdir) \
	      -I$(srcdir)/../main -I$(srcdir)/../mincov \
	      -I$(srcdir)/../util -I$(srcdir)/../sparse


libmincov_la_SOURCES = \
	dominate.c  gimpel.c  indep.c  mincov.c  part.c  solution.c

