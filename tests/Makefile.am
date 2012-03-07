if ENABLE_TESTS

TEST_PROGS =
noinst_PROGRAMS = $(TEST_PROGS)

TEST_PROGS += queue llist

progs_cflags = \
	-I$(top_srcdir)/src \
	$(AM_CFLAGS) \
	$(CLDD_CFLAGS)

# there's probably a better way to achieve this, like sticking common code
# in a shared library... later
progs_ldadd = \
	$(top_srcdir)/src/cldd-adt.o \
	$(top_srcdir)/src/cldd-client.o \
	$(top_srcdir)/src/cldd-cmdline.o \
	$(top_srcdir)/src/cldd-conf.o \
	$(top_srcdir)/src/cldd-daemon.o \
	$(top_srcdir)/src/cldd-error.o \
	$(top_srcdir)/src/cldd-log.o \
	$(top_srcdir)/src/cldd-server.o \
	$(top_srcdir)/src/cldd-utils.o \
	$(CLDD_LIBS)

# queue
queue_CFLAGS = $(progs_cflags)
queue_LDADD = $(progs_ldadd)

queue_headers = \
	$(top_srcdir)/src/common.h \
	$(top_srcdir)/src/cldd.h \
	$(top_srcdir)/src/adt.h

queue_SOURCES = \
	$(queue_headers) \
	test-queue.c

# llist
llist_CFLAGS = $(progs_cflags)
llist_LDADD = $(progs_ldadd)

llist_headers = \
	$(top_srcdir)/src/common.h \
	$(top_srcdir)/src/cldd.h \
	$(top_srcdir)/src/adt.h

llist_SOURCES = \
	$(llist_headers) \
	test-llist.c

endif
