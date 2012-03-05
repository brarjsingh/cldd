#ifndef _CLDC_COMMON_H
#define _CLDC_COMMON_H

/* for mixing c and c++ */
#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <asm/types.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/resource.h>

#if STDC_HEADERS
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#  include <stdbool.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif

#ifdef HAVE_SYSLOG
#  include <syslog.h>
#endif

#ifdef HAVE_LIBWRAP
#  include <tcpd.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include <linux/limits.h>

/* gave up and included glib for top and adt implementations */
#include <glib.h>

#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS  0
#  define EXIT_FAILURE  1
#endif

#endif /* !_CLDC_COMMON_H */
