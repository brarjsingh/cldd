dnl ---------------------------------------------------------------------------
dnl Process this file with autoconf to produce a configure script. At this
dnl point this just uses very pedestrian settings.
dnl ---------------------------------------------------------------------------

AC_PREREQ(2.59)
AC_INIT([cldd],[0.2.0],[geoff.jay@gmail.com])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([dist-bzip2])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE
AC_ISC_POSIX
CFLAGS="$CFLAGS"

AC_DEFINE(PROTOCOL_VERSION, "0.2.0", [The CLD protocol version])

dnl ---------------------------------------------------------------------------
dnl Programs
dnl ---------------------------------------------------------------------------
dnl AC_PROG_CC_C99
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
#AM_PROG_CC_STDC
#AC_PROG_LN_S

dnl --------------------------------------------------------------------------
dnl Declare variables
dnl --------------------------------------------------------------------------
AC_SUBST(AM_CFLAGS,"")
AC_SUBST(CLDD_CFLAGS)
AC_SUBST(CLDD_LIBS)
CLDD_LIBS=""
CLDD_CFLAGS=""

dnl Installation paths - there's got to be a better way to do this
if test "x$prefix" = "xNONE" ; then
	prefix="/usr/local"
fi

if test "x$exec_prefix" = "xNONE" ; then
	exec_prefix=$prefix
fi

if test "x$sysconfdir" = "xNONE" ; then
	sysconfdir="$prefix/etc"
fi

PREFIX=$prefix
AC_SUBST(PREFIX)

SYSCONFDIR="$prefix/etc"
AC_SUBST(SYSCONFDIR)

AC_DEFINE_UNQUOTED(DATADIR, "$SYSCONFDIR/$PACKAGE", [
	Define the read-only architecture-independent data directory.
	])

dnl --------------------------------------------------------------------------
dnl --with-<foo> options, pass --without-<foo> to disable
dnl --------------------------------------------------------------------------
AC_ARG_WITH([distro],
	[AS_HELP_STRING([--with-distro],
					[Configure for a specific distribution (supported values: generic, redhat, suse, debian, gentoo, slackware; default is to autodetect)])],
	[DISTRO=$withval])
AC_ARG_WITH([initdir],
	[AS_HELP_STRING([--with-initdir],
					[Override directory for init scripts (default is distribution-specific)])],
	[INITDIR=$withval])

dnl --------------------------------------------------------------------------
dnl check which distribution we're running on in order to correctly handle
dnl installation of the init.d/rc.d script
dnl --------------------------------------------------------------------------
if test -z $DISTRO; then
	AC_CHECK_FILE(/etc/arch-release,   [DISTRO="arch"])
	AC_CHECK_FILE(/etc/gentoo-release, [DISTRO="gentoo"])
	AC_CHECK_FILE(/etc/redhat-release, [DISTRO="redhat"])
	AC_CHECK_FILE(/etc/debian_version, [DISTRO="debian"])
fi
case "$DISTRO" in
	arch)
		AC_MSG_NOTICE([configured for Arch.])
		;;
	gentoo)
		AC_MSG_NOTICE([configured for Gentoo.])
		;;
	redhat)
		test -z $INITDIR && INITDIR="$sysconfdir/rc.d/init.d"
		AC_MSG_NOTICE([configured for Red Hat (includes Fedora, RHEL, CentOS).])
		;;
	debian)
		AC_MSG_NOTICE([configured for Debian (includes Ubuntu).])
		;;
	"")
		AC_MSG_WARN([Unable to determine what distribution we are running on.
					 Distribution-specific features will be disabled.])
		;;
esac

dnl INITDIR may be set with --with-initdir, or set in the distro
dnl detection magic above. If unset down to here, use a sensible
dnl default.
test -z $INITDIR && INITDIR="$sysconfdir/init.d"

AC_SUBST(DISTRO)
AC_SUBST(INITDIR)

dnl ---------------------------------------------------------------------------
dnl Headers/Libraries
dnl ---------------------------------------------------------------------------
AC_CHECK_FUNCS(daemon fork syslog)
if test $ac_cv_func_syslog = no; then
	# syslog is not in the default libraries.  See if it's in some other.
	for lib in bsd socket inet; do
		AC_CHECK_LIB($lib, syslog,
			[AC_DEFINE(HAVE_SYSLOG)
			LIBS="$LIBS -l$lib"; break])
	done
fi

dnl Some examples check this so might want to come back to this later for
dnl cross platform support.
dnl AC_CHECK_FUNS(vsnprintf)

AC_CHECK_LIB(socket, socket, CLDD_LIBS="$CLDD_LIBS -lsocket",)
AC_CHECK_LIB(nsl, gethostbyname, CLDD_LIBS="$CLDD_LIBS -lnsl",)
AC_CHECK_LIB(pthread, pthread_create, CLDD_LIBS="$CLDD_LIBS -pthread")
AC_CHECK_LIB(m, cos, CLDD_LIBS="$CLDD_LIBS -lm")
AC_CHECK_LIB(rt, clock_gettime, CLDD_LIBS="$CLDD_LIBS -lrt")
dnl AC_CHECK_LIB(gthread-2.0, g_thread_init)

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([valgrind/memcheck.h])

PKG_CHECK_MODULES([GLIB], [glib-2.32 gobject-2.0 gthread-2.0],
	[AC_DEFINE(HAVE_GLIB232, [], [GLib 2.32 available])],
	[PKG_CHECK_MODULES([GLIB], [glib-2.0 gobject-2.0 gthread-2.0],,
		[AC_MSG_ERROR([glib-2.0 is required])])
	])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl PKG_CHECK_MODULES([GLIBTOP], [libgtop-2.0],,
dnl 	[AC_MSG_ERROR([libgtop-2.0 is required])])

PKG_CHECK_MODULES([GEE], [gee-1.0],,
	[AC_MSG_ERROR([gee-1.0 is required])])
AC_SUBST(GEE_CFLAGS)
AC_SUBST(GEE_LIBS)

PKG_CHECK_MODULES([XML], [libxml-2.0],,
	[AC_MSG_ERROR([libxml-2.0 is required])])
AC_SUBST(XML_CFLAGS)
AC_SUBST(XML_LIBS)

PKG_CHECK_MODULES([CLD], [cldd-0.2 cld-0.2],,
	[AC_MSG_ERROR([libcldd-0.2 and libcld-0.2 are required])])
AC_SUBST(CLD_CFLAGS)
AC_SUBST(CLD_LIBS)

dnl ---------------------------------------------------------------------------
dnl Comedi - requires comedi.pc
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(comedi,
[  --with-comedi		  use Comedi drivers],
	[use_comedi=$withval],
	[use_comedi=no])
AC_MSG_RESULT(checking whether Comedi drivers were selected... $use_comedi)

if test "x$use_comedi" != "xno" ; then
	PKG_CHECK_MODULES([COMEDI], [comedi],,
		[AC_MSG_ERROR([Comedi is required])])
	AC_DEFINE(USE_COMEDI, [1], [Comedi Drivers])
fi

dnl ---------------------------------------------------------------------------
dnl Allow tools to be specifically built
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],
		[enable debugging (default: disabled)]),,
	[enable_debug=no])

AC_ARG_ENABLE(tests,
	AS_HELP_STRING([--enable-tests],
		[build test programs (default: disabled)]),,
	[enable_tests=no])

AM_CONDITIONAL(ENABLE_TESTS, test "x$enable_tests" = xyes)

if test "x$enable_tests" = xyes; then
	CLDD_FLAGS="-I/src"
fi

AC_ARG_ENABLE(ipv6,
	AS_HELP_STRING([--disable-ipv6],
		[disable IPv6 support (default: enable)]),,
	[enable_ipv6=yes])

AC_ARG_ENABLE(tcp,
	AS_HELP_STRING([--disable-tcp],
		[disable support for clients connecting via TCP (default: enable)]),,
	[enable_tcp=yes])

AC_ARG_ENABLE(un,
	AS_HELP_STRING([--disable-un],
		[disable support for clients connecting via unix domain sockets (default: enable)]),,
	[enable_un=yes])

AC_ARG_ENABLE(werror,
	AS_HELP_STRING([--enable-werror],
		[treat warnings as errors (default: disabled)]),,
	[enable_werror=no])

dnl ---------------------------------------------------------------------------
dnl Protocol Options
dnl ---------------------------------------------------------------------------

if test x$enable_tcp = xno; then
	# if we don't support TCP, we don't need IPv6 either
	enable_ipv6=no
fi

if test x$enable_ipv6 = xyes; then
	AC_MSG_CHECKING(for ipv6)
	AC_EGREP_CPP([AP_maGiC_VALUE],
	[
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#ifdef PF_INET6
#ifdef AF_INET6
AP_maGiC_VALUE
#endif
#endif
	],
	AC_DEFINE(HAVE_IPV6, 1, [Define if IPv6 support present])
	AC_MSG_RESULT([yes]),
	AC_MSG_RESULT([no])
)
fi

if test x$enable_tcp = xyes; then
	AC_DEFINE(HAVE_TCP, 1, [Define if TCP socket support is enabled])
fi

if test x$enable_un = xyes; then
	AC_DEFINE(HAVE_UN, 1, [Define if unix domain socket support is enabled])
	STRUCT_UCRED
	AC_CHECK_FUNCS(getpeereid)
fi

if
	test x$enable_tcp = xno &&
	test x$enable_un = xno; then
	AC_MSG_ERROR([No client interfaces configured!])
fi

if test x$enable_largefile != xno; then
	AC_DEFINE([ENABLE_LARGEFILE], 1, [Define if large file support is enabled])
fi

if test "x$enable_werror" = xyes; then
	AM_CFLAGS="$AM_CFLAGS -Werror -pedantic-errors"
fi

dnl ---------------------------------------------------------------------------
dnl Doxygen Support
dnl ---------------------------------------------------------------------------

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN([$PACKAGE_NAME],[doxygen.cfg])

dnl ---------------------------------------------------------------------------
dnl Additional build flags
dnl ---------------------------------------------------------------------------
if test x$GCC = xyes
then
	CLD_CHECK_FLAG([-g])
	CLD_CHECK_FLAG([-Wall])
	CLD_CHECK_FLAG([-Wextra])
	CLD_CHECK_FLAG([-Wno-deprecated-declarations])
	CLD_CHECK_FLAG([-Wmissing-prototypes])
	CLD_CHECK_FLAG([-Wshadow])
	CLD_CHECK_FLAG([-Wpointer-arith])
	CLD_CHECK_FLAG([-Wstrict-prototypes])
	CLD_CHECK_FLAG([-Wcast-qual])
	CLD_CHECK_FLAG([-Wwrite-strings])
	CLD_CHECK_FLAG([-pedantic])
fi

dnl ---------------------------------------------------------------------------
dnl Pretty-Print Results
dnl ---------------------------------------------------------------------------

echo ''
echo '########### CLDD CONFIGURATION ############'

printf '\nClient support:\n\t'
results(ipv6, "IPv6")
results(tcp, "TCP")
results(un,[UNIX Domain Sockets])

printf '\n\n##########################################\n\n'

echo 'Generating files needed for compilation'

dnl ---------------------------------------------------------------------------
dnl Generate files
dnl ---------------------------------------------------------------------------
AC_OUTPUT([
	Makefile
	src/Makefile
	tests/Makefile
])

echo 'CLDD is ready for compilation, type "make" to begin.'
