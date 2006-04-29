dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

dnl AC_PREREQ(2.57)

m4_MINISIP_PACKAGE_VERSION(milbc, milbc, 0, 1, 0)
m4_MINISIP_PACKAGE_CONTACT([Erik Eliasson], [eliasson@imit.kth.se])

AC_INIT(MINISIP_PACKAGE_NAME, MINISIP_PACKAGE_VERSION, MINISIP_PACKAGE_CONTACT)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE(MINISIP_PACKAGE_NAME, MINISIP_PACKAGE_VERSION)
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(include/compilation_config.h)
AM_MINISIP_PACKAGE_INIT

AM_MINISIP_LIBTOOL_EXTRAS
AC_PROG_LIBTOOL

AM_MINISIP_CHECK_LIBMUTIL([0.3.1])
AM_MINISIP_CHECK_LIBMCRYPTO([0.3.1])
AM_MINISIP_CHECK_LIBMNETUTIL([0.3.1])
AM_MINISIP_CHECK_LIBMSIP([0.3.1])
AM_MINISIP_CHECK_LIBMIKEY([0.4.1])
AM_MINISIP_CHECK_LIBMINISIP([0.3.1])
AM_MINISIP_CHECK_COMPLETE

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([malloc.h stdlib.h string.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

AC_CONFIG_FILES([
		Makefile
	])
AC_OUTPUT