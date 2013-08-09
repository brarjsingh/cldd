ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign 1.9 dist-bzip2 subdir-objects

# Doxygen support
include $(top_srcdir)/aminclude.am

# add these in later (this needs additional checking for distro)
#init_d_dir = $(sysconfdir)/rc.d
#init_d_SCRIPTS = $(PACKAGE)

#
# Process sub directories
#

SUBDIRS = \
	src \
	tests

#
# Data files for eg. configuration
#

data_cldddir = $(sysconfdir)/@PACKAGE@
data_cldd_DATA = \
	data/cldd.conf \
	data/cld.xml

#
# Distribution
#

EXTRA_DIST = \
	autogen.sh \
	doxygen.cfg
