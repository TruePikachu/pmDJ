bin_PROGRAMS = smdDump smd2mid
noinst_PROGRAMS = checkSmds
check_PROGRAMS = test_midiFile
smdDump_SOURCES = smdDump.cpp smdRead.cpp
smd2mid_SOURCES = smd2mid.cpp smdRead.cpp midiWrite.cpp instMap.cpp
checkSmds_SOURCES = checkSmds.cpp smdRead.cpp
test_midiFile_SOURCES = midiFile__test.cpp midiFile.cpp
test_midiFile_LDADD = -ltap++
noinst_HEADERS = smdRead.hpp midiWrite.hpp instMap.hpp midiFile.hpp

AM_CXXFLAGS=-DPKGDIR=\"$(top_srcdir)\"
TESTS = test_midiFile
EXTRA_DIST = drum_table.txt prog_table.txt README.md test_data/totaka.mid
