# Some of these are from pmd2mid, which I'm just going to toss into here for now
bin_PROGRAMS = smdDump smd2mid
noinst_PROGRAMS = checkSmds
#pmDJ_SOURCES = pmDJ.cpp smdRead.cpp midiWrite.cpp
smdDump_SOURCES = smdDump.cpp smdRead.cpp
smd2mid_SOURCES = smd2mid.cpp smdRead.cpp midiFile.cpp instMap.cpp
checkSmds_SOURCES = checkSmds.cpp smdRead.cpp
noinst_HEADERS = smdRead.hpp midiFile.hpp instMap.hpp

EXTRA_DIST = drum_table.txt prog_table.txt README.md
