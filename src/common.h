#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef STDC_HEADERS
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdarg.h>
#  include <stdbool.h>
#  include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#ifdef USE_COMEDI
#  include <comedilib.h>
#endif

#include <cld-0.2/cld.h>

#endif
