dnl
dnl This is to give the user a warning if they do not have libpcre installed
dnl or if they choose to disable it
dnl
dnl This could lead to security issues with user input so it is advisable
dnl to have libpcre enabled on this build
dnl
#==========================================================
#        Start of user checks function
#==========================================================

AC_DEFUN([AX_USER_CHECKS], [dnl
  AC_MSG_CHECKING([for user checks enabled])
  AC_ARG_ENABLE([user-checks],
    [  --enable-user-checks    enable user input checks],,
        [enable_user_checks="yes"])
  AS_IF([test ".$enable_user_checks" = ".yes"],
    [AC_MSG_RESULT([enabled])
    AS_IF([test ".$with_pcre" = ".no" ],
      [AC_MSG_ERROR([ \
*************************************************************************
*** WARNING WARNING WARNING
*** You have chosen not to use libpcre.
*** This is the primary method of checking user input in these programs
*** With no user input checks, you could be at risk of security
*** breaches. If you know what you are doing, and do NOT want to use
*** libpcre please re-run configure with the --disable-user-checks flag
***
*** Again, you lower the security of these programs if you do this!
*** YOU HAVE BEEN WARNED ])])
    AS_IF([test ".$ac_cv_lib_pcre_pcre_study" != ".yes"],
      [AC_MSG_ERROR([ \
*************************************************************************
*** WARNING WARNING WARNING
*** You do not have libpcre, or the headers.
*** This is the primary method of checking user input in these programs
*** With no user input checks, you could be at risk of security
*** breaches. If you know what you are doing, and do NOT want to use
*** libpcre please re-run configure with the --disable-user-checks flag
***
*** Again, you lower the security of these programs if you do this!
*** YOU HAVE BEEN WARNED ])])
  ], [AC_MSG_RESULT([disabled])])
])

