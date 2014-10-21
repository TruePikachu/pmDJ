bin_PROGRAMS = smdDump smd2mid
noinst_PROGRAMS = checkSmds
check_PROGRAMS = test_midiFile test_smdFile
smdDump_SOURCES = smdDump.cpp smdFile.cpp
smd2mid_SOURCES = smd2mid.cpp smdRead.cpp midiWrite.cpp instMap.cpp
checkSmds_SOURCES = checkSmds.cpp smdRead.cpp
test_midiFile_SOURCES = midiFile__test.cpp midiFile.cpp
test_midiFile_LDADD = -ltap++
test_smdFile_SOURCES = smdFile__test.cpp smdFile.cpp
test_smdFile_LDADD = -ltap++
noinst_HEADERS = smdRead.hpp midiWrite.hpp instMap.hpp midiFile.hpp smdFile.hpp

AM_CXXFLAGS=-DPKGDIR=\"$(top_srcdir)\"
TESTS = test_midiFile test_smdFile
EXTRA_DIST = drum_table.txt prog_table.txt README.md test_data/totaka.mid test_data/bgm0020.smd
