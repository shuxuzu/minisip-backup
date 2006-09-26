AC_PREREQ([2.50])

m4_MINISIP_PACKAGE_VERSION(libzrtpcpp, zrtpcpp, 0, 9, 0)
m4_MINISIP_PACKAGE_CONTACT([Werner Dittmann], [Werner.Dittmann@t-online.de])
dnl m4_MINISIP_PACKAGE_RELEASED
m4_MINISIP_LIBRARY_VERSION(0, 0, 0)

AC_INIT(MINISIP_PACKAGE_NAME, MINISIP_PACKAGE_VERSION, MINISIP_PACKAGE_CONTACT)
AM_MINISIP_PACKAGE_INIT
AC_CONFIG_HEADERS([config.h])

# checks for progs
AC_PROG_CXX
AM_MINISIP_LIBTOOL_EXTRAS
AC_PROG_LIBTOOL

# use "-no-undefined" on Cygwin to force (trigger) libtool to create
# the shared lib. If this is not set this library
# is not created. Be sure that the LIBS variable above contains _all_
# libraries necessary to build ours, Cygwin does not allow undefined
# symbols.
case $host in
#  *-*-msdos* | *-*-go32* | *-*-mingw32* | *-*-cygwin* | *-*-windows*)
  *-*-cygwin*)
	LDFLAGS="$LDFLAGS -no-undefined"
    ;;
  *)
    ;;
esac

dnl
dnl mingw support
dnl
AC_CHECK_LIB([wsock32], [main])

AC_CHECK_LIB([crypto],
             [EVP_CipherInit_ex],
             [
              CRYPTOBACKEND="openSSL >= 0.9.8"
              LIBS="-lcrypto $LIBS"
             ],
             [AC_MSG_ERROR([Cannot find libgcrypt or OpenSSL crypto library.])]
             )

AM_CONDITIONAL(ZRTP_GCRYPT, test "${ac_cv_lib_gcrypt_gcry_cipher_open}" = "yes")
AM_CONDITIONAL(ZRTP_OPENSSL, test "${ac_cv_lib_crypto_EVP_CipherInit_ex}" = "yes")

if test $GCC = yes ; then
        WARN_FLAGS="-Wall -ansi -pedantic"
else
        WARN_FLAGS=""
fi
AC_SUBST(WARN_FLAGS)

AM_MINISIP_CHECK_LIBMUTIL([0.3.1])
AM_MINISIP_CHECK_COMPLETE

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([srandom random srand rand])

AC_SUBST(CRYPTOBACKEND)
AC_CONFIG_FILES([Makefile m4/Makefile src/libzrtpcpp/Makefile src/libzrtpcpp/crypto/Makefile])
AC_OUTPUT(libzrtpcpp.pc)