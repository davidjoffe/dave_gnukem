#
# David Joffe
# Copyright 1998-2002 David Joffe
# 1998/12
# makefile for Dave Gnukem
#

CPP = g++
CC = gcc


INCLUDEDIRS= -I/usr/include/SDL 

#CFLAGS = -O -Wall $(INCLUDEDIRS)

# Un/recomment as needed for removing sound support, optimizations etc
# If you don't -DDATA_DIR to a valid dir, then data files will be assumed
# to be in current directory
#CFLAGS = -Wall -I/usr/local/include -DHAVE_SOUND -DDEBUG -O -m486
CFLAGS = -Wall -Wno-switch -DUSESDL -DDEBUG $(INCLUDEDIRS)
#Release version:
#CFLAGS = -O -Wall -I/usr/local/include -DHAVE_SOUND $(INCLUDEDIRS)

LIBS = -lSDL -lSDLmain -lSDL_mixer -lpthread 
BIN = davegnukem
BINED = ed

OBJFILES = src/main.o     src/graph.o   src/game.o         src/menu.o\
           src/block.o    src/credits.o src/instructions.o src/djstring.o \
           src/djimage.o  src/djlog.o   src/inventory.o    src/mission.o\
           src/hiscores.o src/mixins.o  src/thing.o        src/hero.o \
           src/level.o    src/settings.o src/keys.o \
           src/djtypes.o  src/bullet.o \
           src/ed.o src/ed_DrawBoxContents.o src/ed_common.o src/ed_lvled.o \
           src/ed_macros.o src/ed_spred.o \
           src/sdl/djgraph.o src/sdl/djinput.o src/sdl/djsound.o \
           src/sdl/djtime.o \
           src/sys_error.o src/sys_log.o src/m_misc.cpp

OBJFILESED = src/graph.o   src/ed_standalone_original.o  \
           src/block.o     src/djstring.o \
           src/djimage.o  src/djlog.o    src/mission.o\
           src/mixins.o  src/level.o \
           src/djtypes.o \
           src/sdl/djgraph.o src/sdl/djinput.o src/sdl/djsound.o \
           src/sdl/djtime.o \
           src/sys_error.o src/sys_log.o src/m_misc.o

default: djg

djg: $(OBJFILES)
	$(CPP) -o $(BIN) $(OBJFILES) $(LIBS)

ed: $(OBJFILESED)
	$(CPP) -o $(BINED) $(OBJFILESED) $(LIBS)

clean:
	rm -f $(BIN) $(BINED) *~ core \#*
	find src -name '*.o' | xargs rm -f

dist:
	rm -f core *~ \#*
	find src -name '*.o' | xargs rm -f

linecount:
	cat src/*.cpp src/*.h src/linux/*.cpp  | wc -l

fixme:
	ls src/*.c src/*.cpp src/linux/*.cpp src/*.h | xargs grep -i fixme

%.o: %.c
	$(CPP) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

