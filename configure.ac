#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([cmdb], [0.2.6], [iain-at-thargoid-dot-co-dot-uk])
AC_CONFIG_SRCDIR([src/cmdb.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_MKDIR_P

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdlib.h string.h unistd.h])

AC_DEFUN([AX_LIB_MYSQL],
[
    AC_ARG_WITH([mysql],
        AC_HELP_STRING([--with-mysql=@<:@ARG@:>@],
            [use MySQL client library @<:@default=yes@:>@, optionally specify path to mysql_config]
        ),
        [
        if test "$withval" = "no"; then
            want_mysql="no"
        elif test "$withval" = "yes"; then
            want_mysql="yes"
        else
            want_mysql="yes"
            MYSQL_CONFIG="$withval"
        fi
        ],
        [want_mysql="yes"]
    )

    MYSQL_CFLAGS=""
    MYSQL_LDFLAGS=""
    MYSQL_VERSION=""

    dnl
    dnl Check MySQL libraries (libpq)
    dnl

    if test "$want_mysql" = "yes"; then

        if test -z "$MYSQL_CONFIG" -o test; then
            AC_PATH_PROG([MYSQL_CONFIG], [mysql_config], [no])
        fi

        if test "$MYSQL_CONFIG" != "no"; then
            AC_MSG_CHECKING([for MySQL libraries])

            MYSQL_CFLAGS="`$MYSQL_CONFIG --cflags`"
            MYSQL_LDFLAGS="`$MYSQL_CONFIG --libs`"

            MYSQL_VERSION=`$MYSQL_CONFIG --version`

            AC_DEFINE([HAVE_MYSQL], [1],
                [Define to 1 if MySQL libraries are available])

            found_mysql="yes"
            AC_MSG_RESULT([yes])
            HAVE_MYSQL="true"
        else
            found_mysql="no"
            AC_MSG_RESULT([no])
            HAVE_MYSQL="false"
        fi
    fi

    dnl
    dnl Check if required version of MySQL is available
    dnl


    mysql_version_req=ifelse([$1], [], [], [$1])

    if test "$found_mysql" = "yes" -a -n "$mysql_version_req"; then

        AC_MSG_CHECKING([if MySQL version is >= $mysql_version_req])

        dnl Decompose required version string of MySQL
        dnl and calculate its number representation
        mysql_version_req_major=`expr $mysql_version_req : '\([[0-9]]*\)'`
        mysql_version_req_minor=`expr $mysql_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        mysql_version_req_micro=`expr $mysql_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$mysql_version_req_micro" = "x"; then
            mysql_version_req_micro="0"
        fi

        mysql_version_req_number=`expr $mysql_version_req_major \* 1000000 \
                                   \+ $mysql_version_req_minor \* 1000 \
                                   \+ $mysql_version_req_micro`

        dnl Decompose version string of installed MySQL
        dnl and calculate its number representation
        mysql_version_major=`expr $MYSQL_VERSION : '\([[0-9]]*\)'`
        mysql_version_minor=`expr $MYSQL_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        mysql_version_micro=`expr $MYSQL_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$mysql_version_micro" = "x"; then
            mysql_version_micro="0"
        fi

        mysql_version_number=`expr $mysql_version_major \* 1000000 \
                                   \+ $mysql_version_minor \* 1000 \
                                   \+ $mysql_version_micro`

        mysql_version_check=`expr $mysql_version_number \>\= $mysql_version_req_number`
        if test "$mysql_version_check" = "1"; then
            AC_MSG_RESULT([yes])
        else
            AC_MSG_RESULT([no])
        fi
    fi

    AC_SUBST([MYSQL_VERSION])
    AC_SUBST([MYSQL_CFLAGS])
    AC_SUBST([MYSQL_LDFLAGS])
])

AC_DEFUN([AX_PATH_LIB_PCRE],[dnl
AC_MSG_CHECKING([lib pcre])
AC_ARG_WITH(pcre,
[  --with-pcre[[=prefix]]    compile xmlpcre part (via libpcre check)],,
     with_pcre="yes")
if test ".$with_pcre" = ".no" ; then
  AC_MSG_RESULT([disabled])
  m4_ifval($2,$2)
else
  AC_MSG_RESULT([(testing)])
  AC_CHECK_LIB(pcre, pcre_study)
  if test "$ac_cv_lib_pcre_pcre_study" = "yes" ; then
     PCRE_LIBS="-lpcre"
     HAVE_PCRE="true"
     AC_MSG_CHECKING([lib pcre])
     AC_MSG_RESULT([$PCRE_LIBS])
     m4_ifval($1,$1)
  else
     OLDLDFLAGS="$LDFLAGS" ; LDFLAGS="$LDFLAGS -L$with_pcre/lib"
     OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$CPPFLAGS -I$with_pcre/include"
     AC_CHECK_LIB(pcre, pcre_compile)
     CPPFLAGS="$OLDCPPFLAGS"
     LDFLAGS="$OLDLDFLAGS"
     if test "$ac_cv_lib_pcre_pcre_compile" = "yes" ; then
        AC_MSG_RESULT(.setting PCRE_LIBS -L$with_pcre/lib -lpcre)
        PCRE_LIBS="-L$with_pcre/lib -lpcre"
        HAVE_PCRE="true"
        test -d "$with_pcre/include" && PCRE_CFLAGS="-I$with_pcre/include"
        AC_MSG_CHECKING([lib pcre])
        AC_MSG_RESULT([$PCRE_LIBS])
        m4_ifval($1,$1)
     else
        AC_MSG_CHECKING([lib pcre])
        AC_MSG_RESULT([no, (WARNING)])
        m4_ifval($2,$2)
        HAVE_PCRE="false"
     fi
  fi
fi
AC_SUBST([PCRE_LIBS])
AC_SUBST([PCRE_CFLAGS])
])
AC_DEFUN([AX_LIB_SQLITE3],
[
    AC_ARG_WITH([sqlite3],
        AS_HELP_STRING(
            [--with-sqlite3=@<:@ARG@:>@],
            [use SQLite 3 library @<:@default=yes@:>@, optionally specify the prefix for sqlite3 library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_SQLITE3="no"
        elif test "$withval" = "yes"; then
            WANT_SQLITE3="yes"
            ac_sqlite3_path=""
        else
            WANT_SQLITE3="yes"
            ac_sqlite3_path="$withval"
        fi
        ],
        [WANT_SQLITE3="yes"]
    )

    SQLITE3_CFLAGS=""
    SQLITE3_LDFLAGS=""
    SQLITE3_VERSION=""

    if test "x$WANT_SQLITE3" = "xyes"; then

        ac_sqlite3_header="sqlite3.h"

        sqlite3_version_req=ifelse([$1], [], [3.0.0], [$1])
        sqlite3_version_req_shorten=`expr $sqlite3_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
        sqlite3_version_req_major=`expr $sqlite3_version_req : '\([[0-9]]*\)'`
        sqlite3_version_req_minor=`expr $sqlite3_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        sqlite3_version_req_micro=`expr $sqlite3_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$sqlite3_version_req_micro" = "x" ; then
            sqlite3_version_req_micro="0"
        fi

        sqlite3_version_req_number=`expr $sqlite3_version_req_major \* 1000000 \
                                   \+ $sqlite3_version_req_minor \* 1000 \
                                   \+ $sqlite3_version_req_micro`

        AC_MSG_CHECKING([for SQLite3 library >= $sqlite3_version_req])

        if test "$ac_sqlite3_path" != ""; then
            ac_sqlite3_ldflags="-L$ac_sqlite3_path/lib"
            ac_sqlite3_cppflags="-I$ac_sqlite3_path/include"
        else
            for ac_sqlite3_path_tmp in /usr /usr/local /opt ; do
                if test -f "$ac_sqlite3_path_tmp/include/$ac_sqlite3_header" \
                    && test -r "$ac_sqlite3_path_tmp/include/$ac_sqlite3_header"; then
                    ac_sqlite3_path=$ac_sqlite3_path_tmp
                    ac_sqlite3_cppflags="-I$ac_sqlite3_path_tmp/include"
                    ac_sqlite3_ldflags="-L$ac_sqlite3_path_tmp/lib"
                    break;
                fi
            done
        fi

        ac_sqlite3_ldflags="$ac_sqlite3_ldflags -lsqlite3"

        saved_CPPFLAGS="$CPPFLAGS"
        CPPFLAGS="$CPPFLAGS $ac_sqlite3_cppflags"

        AC_LANG_PUSH(C)
        AC_COMPILE_IFELSE(
            [
            AC_LANG_PROGRAM([[@%:@include <sqlite3.h>]],
                [[
#if (SQLITE_VERSION_NUMBER >= $sqlite3_version_req_number)
/* Everything is okay */
#else
#  error SQLite version is too old
#endif
                ]]
            )
            ],
            [
            AC_MSG_RESULT([yes])
            success="yes"
            ],
            [
            AC_MSG_RESULT([not found])
            success="no"
            ]
        )
        AC_LANG_POP(C)

        CPPFLAGS="$saved_CPPFLAGS"

        if test "$success" = "yes"; then

            SQLITE3_CFLAGS="$ac_sqlite3_cppflags"
            SQLITE3_LDFLAGS="$ac_sqlite3_ldflags"

            ac_sqlite3_header_path="$ac_sqlite3_path/include/$ac_sqlite3_header"

            dnl Retrieve SQLite release version
            if test "x$ac_sqlite3_header_path" != "x"; then
                ac_sqlite3_version=`cat $ac_sqlite3_header_path \
                    | grep '#define.*SQLITE_VERSION.*\"' | sed -e 's/.* "//' \
                        | sed -e 's/"//'`
                if test $ac_sqlite3_version != ""; then
                    SQLITE3_VERSION=$ac_sqlite3_version
                else
                    AC_MSG_WARN([Cannot find SQLITE_VERSION macro in sqlite3.h header to retrieve SQLite version!])
                fi
            fi

            AC_SUBST(SQLITE3_CFLAGS)
            AC_SUBST(SQLITE3_LDFLAGS)
            AC_SUBST(SQLITE3_VERSION)
            AC_DEFINE([HAVE_SQLITE3], [], [Have the SQLITE3 library])
        else
            HAVE_SQLITE3="false"
        fi
    fi
])

AX_LIB_MYSQL
AX_PATH_LIB_PCRE
AX_LIB_SQLITE3

AM_CONDITIONAL([HAVE_MYSQL], [$HAVE_MYSQL])
AM_CONDITIONAL([HAVE_PCRE], [$HAVE_PCRE])
AM_CONDITIONAL([HAVE_SQLITE3], [$HAVE_SQLITE3])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([mkdir strchr strrchr strstr strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_CONFIG_SUBDIRS([cmdb-0.2.6])
AC_OUTPUT
