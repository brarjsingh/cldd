bin_PROGRAMS = cldd

VALAFLAGS = \
	-H $(srcdir)/libcldd.h -C -c --thread \
	--vapidir=$(srcdir)/../vapi -X -g --target-glib=2.32 \
	@VALA_PACKAGES@ --pkg posix --pkg config

cldd_CFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@ \
	$(AM_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GEE_CFLAGS) \
	$(CLD_CFLAGS) \
	$(XML_CFLAGS) \
	$(ZMQ_CFLAGS)

cldd_LDADD = \
	@PACKAGE_LIBS@ \
	$(GLIB_LIBS) \
	$(GEE_LIBS) \
	$(CLD_LIBS) \
	$(XML_LIBS) \
	$(ZMQ_LIBS)

cldd_headers = \
	common.h \
	libcldd.h

cldd_SOURCES = \
	cldd.vala \
	config.vala \
	daemon.vala \
	settings.vala

CLEANFILES = \
	*.stamp
	stamp-h1
	$(srcdir)/libcldd.h
