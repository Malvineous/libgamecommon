check_PROGRAMS = tests

tests_SOURCES = tests.cpp
tests_SOURCES += test-byteorder.cpp
tests_SOURCES += test-bitstream.cpp
tests_SOURCES += test-filter-lzss.cpp
tests_SOURCES += test-iff.cpp
tests_SOURCES += test-iostream_helpers.cpp
tests_SOURCES += test-stream.cpp
tests_SOURCES += test-stream_file.cpp
tests_SOURCES += test-stream_filtered.cpp
tests_SOURCES += test-stream_seg.cpp
tests_SOURCES += test-stream_string.cpp
tests_SOURCES += test-stream_sub.cpp
tests_SOURCES += test-lzw.cpp

EXTRA_tests_SOURCES = tests.hpp

TESTS = tests

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDFLAGS  = $(top_builddir)/src/libgamecommon.la
AM_LDFLAGS += $(BOOST_SYSTEM_LIBS)
AM_LDFLAGS += $(BOOST_UNIT_TEST_FRAMEWORK_LIBS)
