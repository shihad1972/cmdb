# ===========================================================================
#      Start of DNSA test
# ===========================================================================
#
AC_DEFUN([AX_CHECK_DNSA],[dnl
  AC_MSG_CHECKING([to enable dnsa])
  AC_ARG_ENABLE([dnsa],
    [  --enable-dnsa            compile the dnsa module],,
         enable_dnsa="yes")
  if test ".$enable_dnsa" = ".no" ; then
    AC_MSG_RESULT([disabled])
    m4_ifval($2,$2)
  else
    AC_DEFINE([HAVE_DNSA], [1], [Compile the dnsa module])
    AC_MSG_RESULT([yes])
    m4_ifval($1,$1)
  fi
])
#
# ===========================================================================
#      End of DNSA Test
# ===========================================================================

