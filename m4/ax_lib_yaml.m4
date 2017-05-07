#
# Autoconf Macro to search for (required) libyaml
#
# User may have to provide the path to the install if it cannot be found
#
AC_DEFUN([AX_LIB_YAML],[dnl
AC_ARG_WITH([libyaml], 
[  --with-libyaml[[=prefix]] compile using libyaml],,
[    with_libyaml="yes"])
if test ".$with_libyaml" = ".no"; then
	AC_MSG_ERROR([libyaml is required for this program])
else
	AC_MSG_RESULT([yes])
	AC_CHECK_LIB([yaml], [yaml_parser_initialize])
	if test "x$ac_cv_lib_yaml_yaml_parser_initialize" = "xyes" ; then
		AC_CHECK_HEADER([yaml.h], [AC_DEFINE([HAVE_YAML_H], [true],
			[define to true if we find the yaml.h header file])], [], [])
		YAML_LIBS="-lyaml"
		HAVE_LIBYAML="true"
		m4_ifval($1,$1)
	fi
	if test "x$HAVE_LIBYAML" = "x" && "x$with_yaml" != "xyes" ; then
		AC_MSG_CHECKING([checking with supplied path $with_yaml])
		OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$OLDCPPFLAGS -I$with_yaml/include"
		OLDLDFLAGS="$LDFLAGS" ; LDFLAGS="OLDLDFLAGS -L $with_yaml/lib"
		AC_CHECK_LIB([yaml], [yaml_parser_initialize])
		CPPFLAGS="$OLDCPPFLAGS"
		LDFLAGS="$OLDLDFLAGS"
		if test "x$ac_cv_lib_yaml_yaml_parser_initialize" = "xyes" ; then
			YAML_LIBS="-lyaml"
			YAML_CPPFLAGS="$-I$with_yaml/include"
			HAVE_LIBYAML="true"
			AC_MSG_RESULT([$YAML_LIBS])
			m4_ifval($1,$1)
		else
			AC_MSG_ERROR([supplied path $with_yaml does not contain libyaml])
		fi
	fi
	if test "x$HAVE_LIBYAML" = "x" ; then
		AC_MSG_ERROR([libyaml is required for this program])
	fi
fi
AC_SUBST([YAML_LIBS])
AC_SUBST([YAML_CPPFLAGS])
AC_SUBST([HAVE_LIBYAML])
])
