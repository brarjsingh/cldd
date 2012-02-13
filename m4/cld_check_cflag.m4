AC_DEFUN([CLD_CHECK_FLAG],[
  var=`echo "$1" | tr "=-" "__"`
  AC_CACHE_CHECK([whether the C compiler accepts $1],
    [cld_check_cflag_$var],[
    save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $1"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
     int main(void) { return 0; }
     ])], [ eval "cld_check_cflag_$var=yes"
     ], [ eval "cld_check_cflag_$var=no" ])
    CFLAGS="$save_CFLAGS"
  ])
  if eval "test x`echo '$cld_check_cflag_'$var` = xyes"
  then
    AM_CFLAGS="$AM_CFLAGS $1"
  fi
  ])
])
