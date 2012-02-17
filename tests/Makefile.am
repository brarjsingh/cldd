if ENABLE_TESTS

TEST_PROGS =
noinst_PROGRAMS = $(TEST_PROGS)

TEST_PROGS += queue

queue_CFLAGS = \
	$(AM_CFLAGS) \
	$(CLDD_CFLAGS)

queue_LDADD = \
	$(CLDD_LIBS)

queue_headers = \
	$(top_srcdir)/src/common.h \
	$(top_srcdir)/src/cldd.h \
	$(top_srcdir)/src/adt.h

queue_SOURCES = \
	$(queue_headers) \
	$(top_srcdir)/src/adt.c \
	test-queue.c

endif
