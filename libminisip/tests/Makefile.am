AM_CPPFLAGS = -I$(top_srcdir)/include $(MINISIP_CFLAGS)
LDADD = $(top_builddir)/libminisip.la $(MINISIP_LIBS)

MINISIP_TESTS = 000_compile

TESTS = $(MINISIP_TESTS)
noinst_PROGRAMS = $(MINISIP_TESTS)

000_compile_SOURCES = 000_compile.cxx

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
