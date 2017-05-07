#
# Autoconf Macro to search for (required) libodbc
#
# User may have to provide the path to the install if it cannot be found
#
AC_DEFUN([AX_LIB_ODBC],[dnl
AC_ARG_WITH([libodbc], 
[  --with-libodbc[[=prefix]] compile using libodbc],,
[    with_libodbc="yes"])
if test ".$with_libodbc" = ".no"; then
	AC_MSG_ERROR([libodbc is required for this program])
else
	AC_CHECK_LIB([odbc], [SQLAllocHandle])
	if test "x$ac_cv_lib_odbc_SQLAllocHandle" = "xyes" ; then
		AC_CHECK_HEADER([sql.h], [AC_DEFINE([HAVE_ODBC_H], [1],
			[define to true if we find the sql.h header file])],
				[AC_MSG_ERROR([Cannot locate sql.h file. Please install odbc development package])], [])
		ODBC_LIBS="-lodbc"
		HAVE_LIBODBC="true"
		AC_MSG_CHECKING([lib odbc])
		AC_MSG_RESULT([$ODBC_LIBS])
		m4_ifval($1,$1)
	fi
	if test "x$HAVE_LIBODBC" = "x" && test "x$with_odbc" != "xyes" ; then
		AC_MSG_CHECKING([checking with supplied path $with_odbc])
		OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$OLDCPPFLAGS -I$with_odbc/include"
		OLDLDFLAGS="$LDFLAGS" ; LDFLAGS="OLDLDFLAGS -L $with_odbc/lib"
		AC_CHECK_LIB([odbc], [SQLAllocHandle])
		CPPFLAGS="$OLDCPPFLAGS"
		LDFLAGS="$OLDLDFLAGS"
		if test "x$ac_cv_lib_odbc_SQLAllocHandle" = "xyes" ; then
			AC_CHECK_HEADER([sql.h], [AC_DEFINE([HAVE_ODBC_H], [1],
				[define to true if we find the sql.h header file])],
					[AC_MSG_ERROR([Cannot locate sql.h file. Please install odbc development package])], [])
			ODBC_LIBS="-lodbc"
			ODBC_CPPFLAGS="$-I$with_odbc/include"
			HAVE_LIBODBC="true"
			AC_MSG_RESULT([$ODBC_LIBS])
			m4_ifval($1,$1)
		else
			AC_MSG_ERROR([supplied path $with_odbc does not contain libodbc])
		fi
	fi
	if test "x$HAVE_LIBODBC" = "x" ; then
		AC_MSG_ERROR([libodbc is required for this program])
	fi
fi
AC_SUBST([ODBC_LIBS])
AC_SUBST([ODBC_CPPFLAGS])
AC_SUBST([HAVE_LIBODBC])
])
