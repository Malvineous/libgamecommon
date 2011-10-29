lib_LTLIBRARIES = libgamecommon.la

libgamecommon_la_SOURCES = iostream_helpers.cpp
libgamecommon_la_SOURCES += bitstream.cpp
libgamecommon_la_SOURCES += lzw.cpp
libgamecommon_la_SOURCES += filter.cpp
libgamecommon_la_SOURCES += metadata.cpp
libgamecommon_la_SOURCES += stream.cpp
libgamecommon_la_SOURCES += stream_file.cpp
libgamecommon_la_SOURCES += stream_filtered.cpp
libgamecommon_la_SOURCES += stream_seg.cpp
libgamecommon_la_SOURCES += stream_string.cpp
libgamecommon_la_SOURCES += stream_sub.cpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(BOOST_CPPFLAGS) -I $(top_srcdir)/include $(WARNINGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS)

libgamecommon_la_LDFLAGS = $(AM_LDFLAGS) -version-info 0:0:0
libgamecommon_la_LIBADD = $(BOOST_SYSTEM_LIBS)
