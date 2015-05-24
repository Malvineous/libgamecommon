lib_LTLIBRARIES = libgamecommon.la

libgamecommon_la_SOURCES = iostream_helpers.cpp
libgamecommon_la_SOURCES += bitstream.cpp
libgamecommon_la_SOURCES += error.cpp
libgamecommon_la_SOURCES += filter.cpp
libgamecommon_la_SOURCES += filter-crop.cpp
libgamecommon_la_SOURCES += filter-dummy.cpp
libgamecommon_la_SOURCES += filter-lzss.cpp
libgamecommon_la_SOURCES += filter-lzw.cpp
libgamecommon_la_SOURCES += filter-pad.cpp
libgamecommon_la_SOURCES += iff.cpp
libgamecommon_la_SOURCES += metadata.cpp
libgamecommon_la_SOURCES += stream.cpp
libgamecommon_la_SOURCES += stream_file.cpp
libgamecommon_la_SOURCES += stream_filtered.cpp
libgamecommon_la_SOURCES += stream_seg.cpp
libgamecommon_la_SOURCES += stream_string.cpp
libgamecommon_la_SOURCES += stream_sub.cpp
libgamecommon_la_SOURCES += suppitem.cpp

WARNINGS = -Wall -Wextra -Wpedantic -Wno-unused-parameter

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(WARNINGS)

AM_CXXFLAGS  = $(DEBUG_CXXFLAGS)

libgamecommon_la_LDFLAGS  = $(AM_LDFLAGS)
libgamecommon_la_LDFLAGS += -version-info 2:0:0
