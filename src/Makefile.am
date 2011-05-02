lib_LTLIBRARIES = libgamecommon.la

libgamecommon_la_SOURCES = iostream_helpers.cpp
libgamecommon_la_SOURCES += segmented_stream.cpp
libgamecommon_la_SOURCES += substream.cpp
libgamecommon_la_SOURCES += bitstream.cpp
libgamecommon_la_SOURCES += lzw.cpp
libgamecommon_la_SOURCES += filteredstream.cpp
libgamecommon_la_SOURCES += exceptions.cpp
libgamecommon_la_SOURCES += metadata.cpp
libgamecommon_la_SOURCES += util.cpp

AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(BOOST_CPPFLAGS) -I $(top_srcdir)/include
AM_CXXFLAGS = $(DEBUG_CXXFLAGS)

libgamecommon_la_LDFLAGS = $(AM_LDFLAGS) -version-info 0:0:0
libgamecommon_la_LIBADD = $(BOOST_SYSTEM_LIBS)
