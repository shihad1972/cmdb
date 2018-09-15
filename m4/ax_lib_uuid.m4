
dnl
dnl M4 Macro test for configure to find and test for libuuid headers and libs.
dnl
dnl (C) Iain M Conochie 2016
dnl
dnl See LICENSE in top level source dir for copying details
dnl

AC_DEFUN([AX_LIB_UUID],[dnl
    AC_ARG_WITH([libuuid],
      [  --with-libuuid[[=prefix]] compile using libuuid],,
    with_uuid="yes")
if test "x$with_libuuid" = "xno"; then
	AC_MSG_RESULT([disabled])
	m4_ifval($2,$2)
else
	for incdirs in /usr/include /usr/local/include
	do
	    OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="-I$incdirs/uuid"
	    AC_CHECK_LIB([uuid], [uuid_generate])
	    if test "x$ac_cv_lib_uuid_uuid_generate" = "xyes" ; then
		UUID_CPPFLAGS="-I$incdirs/uuid"
		UUID_LIBS="-luuid"
		HAVE_LIBUUID="true"
	        CPPFLAGS="$OLDCPPFLAGS"
		break
	    fi
	done
	if test "x$HAVE_LIBUUID" = "x" && test "x$with_uuid" != "xyes" ; then
	    AC_MSG_CHECKING([checking supplied path $with_uuid])
	    OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$OLDCPPFLAGS -I$with_uuid/include/uuid"
	    OLDLDFLAGS="$LDFLAGS" ; LDFLAGS="OLDLDFLAGS -L$with_uuid/lib"
	    AC_CHECK_LIB([uuid], [uuid_generate])
	    CPPFLAGS="$OLDCPPFLAGS"
	    LDFLAGS="OLDLDFLAGS"
	    if test "x$ac_cv_lib_uuid_uuid_generate" = "xyes" ; then
		UUID_CPPFLAGS="-I$with_uuid/include/uuid"
		UUID_LIBS="-luuid"
		UUID_LDFLAGS="-L$with_uuid/lib"
		HAVE_LIBUUID="true"
	    else
		AC_MSG_CHECKING([lib uuid])
		AC_MSG_RESULT([no, (WARNING)])
		m4_ifval($2,$2)
		HAVE_LIBUUUID="false"
	    fi
	fi
	if test "x$HAVE_LIBUUID" = "x" ; then
		AC_MSG_CHECKING([lib uuid])
		AC_MSG_RESULT([no, (WARNING)])
		m4_ifval($2,$2)
		HAVE_LIBUUID="false"
	fi
fi
AC_SUBST([UUID_LDFLAGS])
AC_SUBST([UUID_CPPFLAGS])
AC_SUBST([UUID_LIBS])
])
