noinst_LTLIBRARIES = libsparse.la
pkgincludesubdir = $(includedir)/espresso/sparse
pkgincludesub_HEADERS = sparse.h  sparse_int.h

AM_CFLAGS = $(all_includes) -I$(srcdir) \
	      -I$(srcdir)/../main -I$(srcdir)/../mincov \
	      -I$(srcdir)/../util -I$(srcdir)/../sparse

libsparse_la_SOURCES = \
		       cols.c  matrix.c  rows.c


