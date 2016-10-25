#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cmdb], [0.3], [iain@thargoid.co.uk])
AC_CONFIG_SRCDIR([src/cmdb.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([include/config.h])
AM_INIT_AUTOMAKE

# Add m4 macros
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_AR

# Initialize libtool
LT_INIT

# Checks for libraries.
AX_LIB_MYSQL
AX_LIB_SQLITE3
AC_CHECK_LIB([sqlite3], [sqlite3_errstr],
  [AC_DEFINE([HAVE_SQLITE3_ERRSTR], [], [Have function sqlite3_errstr in sqlite3 library])])
AX_PATH_LIB_PCRE
AX_LIB_GCRYPT([auto])
AX_LIB_UUID

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stdlib.h string.h\
                  sys/socket.h unistd.h wordexp.h errno.h getopt.h\
		  stdbool.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostname memset mkdir strchr strdup strerror strndup\
                wordexp strrchr strstr strtol strtoul])

# Check for packages to enable
AX_USER_CHECKS
AX_CHECK_DNSA
AX_CHECK_CBC

AM_CONDITIONAL([HAVE_MYSQL], [$HAVE_MYSQL])
AM_CONDITIONAL([HAVE_PCRE], [$HAVE_PCRE])
AM_CONDITIONAL([HAVE_SQLITE3], [$HAVE_SQLITE3])
AM_CONDITIONAL([HAVE_LIBGCRYPT], [$HAVE_LIBGCRYPT])
AM_CONDITIONAL([HAVE_LIBUUID], [test x"$HAVE_LIBUUID" = "xtrue"])
AM_CONDITIONAL([HAVE_DNSA], [$HAVE_DNSA])
AM_CONDITIONAL([HAVE_CBC], [$HAVE_CBC])

dnl if --prefix is /usr, don't use /usr/var for localstatedir
dnl or /usr/etc for sysconfdir
dnl
dnl Also check if localstatedir set to /var/lib
dnl reset runstatedir to /var/run

if test "$prefix" = "/usr" && test "$localstatedir" = '${prefix}/var' ; then
    localstatedir='/var/lib'
fi
if test "$prefix" = "/usr" && test "$sysconfdir" = '${prefix}/etc' ; then
    sysconfdir='/etc'
fi
if test "$prefix" = "/usr/local" && test "$localstatedir" = "${prefix}/var" ; then
    localstatedir='/var/lib'
fi
if test "$localstatedir" = '/var/lib'; then
    runstatedir='/var/run'
fi

gl_CONFIGMAKE_PREP

AC_CONFIG_FILES([Makefile
                 daemon/Makefile
                 lib/Makefile
                 man/Makefile
                 scripts/Makefile
                 sql/Makefile
                 src/Makefile])
AC_OUTPUT
