INCLUDES = \
	$(CLDD_CFLAGS)

bin_PROGRAMS = cldd

noinst_LIBRARIES =

AM_CFLAGS += -I$(srcdir)/src
AM_CFLAGS += -DSYSTEM_CONFIG_FILE_LOCATION='"$(sysconfdir)/cldd.conf"'

cldd_CFLAGS = \
	$(AM_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GLIBTOP_CFLAGS) \
	$(CLD_CFLAGS) \
	$(XML_CFLAGS) \
	$(CLDD_CFLAGS)

cldd_LDADD = \
	$(CLDD_LIBS) \
	$(GLIB_LIBS) \
	$(GLIBTOP_LIBS) \
	$(CLD_LIBS) \
	$(XML_LIBS) \
	$(INTLLIBS)

cldd_headers = \
	common.h \
	cldd.h \
	adt.h \
	client.h \
	cmdline.h \
	conf.h \
	daemon.h \
	error.h \
	log.h \
	server.h \
	stream.h \
	utils.h

cldd_SOURCES = \
	$(cldd_headers) \
	main.c \
	adt.c \
	client.c \
	cmdline.c \
	conf.c \
	daemon.c \
	error.c \
	log.c \
	server.c \
	stream.c \
	utils.c

noinst_HEADERS =

CLEANFILES =
