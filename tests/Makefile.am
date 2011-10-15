check_PROGRAMS = tests

tests_SOURCES = tests.cpp
tests_SOURCES += test-byteorder.cpp
tests_SOURCES += test-iostream_helpers.cpp
tests_SOURCES += test-stream_file.cpp
tests_SOURCES += test-streamMove.cpp
tests_SOURCES += test-substream.cpp
tests_SOURCES += test-segmented_stream.cpp
tests_SOURCES += test-bitstream.cpp
tests_SOURCES += test-lzw.cpp
tests_SOURCES += test-filteredstream.cpp

EXTRA_tests_SOURCES = tests.hpp

TESTS = tests

AM_CPPFLAGS = $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
AM_LDFLAGS = $(BOOST_SYSTEM_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(top_builddir)/src/libgamecommon.la
