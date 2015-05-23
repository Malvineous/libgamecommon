check_PROGRAMS = tests stdtests

tests_SOURCES = tests.cpp
tests_SOURCES += test-bitstream.cpp
tests_SOURCES += test-filter-crop.cpp
tests_SOURCES += test-filter-lzss.cpp
tests_SOURCES += test-filter-lzw.cpp
tests_SOURCES += test-filter-pad.cpp
tests_SOURCES += test-iff.cpp
tests_SOURCES += test-iostream_helpers.cpp
tests_SOURCES += test-stream.cpp
tests_SOURCES += test-stream_file.cpp
tests_SOURCES += test-stream_filtered.cpp
tests_SOURCES += test-stream_seg.cpp
tests_SOURCES += test-stream_string.cpp
tests_SOURCES += test-stream_sub.cpp
tests_SOURCES += test-util.cpp

EXTRA_tests_SOURCES = tests.hpp

stdtests_SOURCES = tests.cpp
stdtests_SOURCES += test-byteorder.cpp
EXTRA_stdtests_SOURCES = tests.hpp

TESTS = tests stdtests

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDFLAGS  = $(top_builddir)/src/libgamecommon.la
AM_LDFLAGS += $(BOOST_LDFLAGS)
AM_LDFLAGS += $(BOOST_SYSTEM_LIB)
AM_LDFLAGS += $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
