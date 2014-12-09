#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cmdb], [0.2.19], [iain@thargoid.co.uk])
AC_CONFIG_SRCDIR([src/cmdb.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Add m4 macros
AC_CONFIG_MACRO_DIRS([m4])

# Checks for libraries.
AX_LIB_MYSQL
AX_LIB_SQLITE3
AX_PATH_LIB_PCRE

# Check for packages to enable
AX_USER_CHECKS
AX_CHECK_DNSA
AX_CHECK_CBC

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stdlib.h string.h\
                  sys/socket.h unistd.h wordexp.h errno.h])

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
AC_CHECK_LIB([sqlite3], [sqlite3_errstr],
  [AC_DEFINE([HAVE_SQLITE3_ERRSTR], [], [Have function sqlite3_errstr in sqlite3 library])])

AM_CONDITIONAL([HAVE_MYSQL], [$HAVE_MYSQL])
AM_CONDITIONAL([HAVE_PCRE], [$HAVE_PCRE])
AM_CONDITIONAL([HAVE_SQLITE3], [$HAVE_SQLITE3])
AM_CONDITIONAL([HAVE_DNSA], [$HAVE_DNSA])
AM_CONDITIONAL([HAVE_CBC], [$HAVE_CBC])

AC_CONFIG_FILES([Makefile
                 man/Makefile
                 scripts/Makefile
                 sql/Makefile
                 src/Makefile])
AC_OUTPUT
