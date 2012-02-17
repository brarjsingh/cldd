INCLUDES = \
	$(CLDD_CFLAGS)

bin_PROGRAMS = cldd

noinst_LIBRARIES =

AM_CFLAGS += -I$(srcdir)/src
AM_CFLAGS += -DSYSTEM_CONFIG_FILE_LOCATION='"$(sysconfdir)/cldd.conf"'

cldd_CFLAGS = \
	$(AM_CFLAGS) \
	$(CLDD_CFLAGS)

cldd_LDADD = \
	$(CLDD_LIBS) \
	$(INTLLIBS)

cldd_headers = \
	common.h \
	cldd.h \
	adt.h \
	client.h \
	conf.h \
	daemon.h \
	error.h \
	utils.h

cldd_SOURCES = \
	$(cldd_headers) \
	main.c \
	adt.c \
	client.c \
	conf.c \
	daemon.c \
	error.c \
	utils.c

noinst_HEADERS =

CLEANFILES =
