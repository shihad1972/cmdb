dnl
dnl Check to see if cbc is to be built and it's
dnl associated programs
dnl
#===============================================
#        Start of cbc test
#===============================================

AC_DEFUN([AX_CHECK_CBC], [dnl
  AC_MSG_CHECKING([to enable cbc])
  AC_ARG_ENABLE([cbc],
    [  --enable-cbc            enable cbc and associated programs],,
        [enable_cbc="yes"])
  if test ".$enable_cbc" = ".no" ; then
    AC_MSG_RESULT([disabled])
    m4_ifval($2,$2)
    HAVE_CBC="false"
  else
    AC_DEFINE([HAVE_CBC], [1], [Compile cbc and associated programs])
    AC_MSG_RESULT([yes])
    m4_ifval($1, $1)
    HAVE_CBC="true"
  fi
])

#===============================================
#        End of cbc test
#===============================================
