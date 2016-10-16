#
# David Joffe
# Copyright 1998-2016 David Joffe
# Created 1998/12
# makefile for Dave Gnukem
#
# 2016-10: Get this working on Mac OS X [dj]
#

CPP = g++
CC = gcc


# dj2016-10 Add L -I/usr/local/include/SDL in process of getting this working on Mac OS X - not sure if this is 'bad' to just have both /usr/include and /usr/local/include??
INCLUDEDIRS= -I/usr/include/SDL -I/usr/local/include/SDL 

#CCFLAGS = -O -Wall $(INCLUDEDIRS)

# Un/recomment as needed for removing sound support, optimizations etc
# If you don't -DDATA_DIR to a valid dir, then data files will be assumed
# to be in current directory
#CCFLAGS = -Wall -I/usr/local/include -DHAVE_SOUND -DDEBUG -O -m486
CCFLAGS = -Wall -Wno-switch -DUSESDL -DDEBUG $(INCLUDEDIRS)
#Release version:
#CCFLAGS = -O -Wall -I/usr/local/include -DHAVE_SOUND $(INCLUDEDIRS)

LIBS = -lSDL -lSDLmain -lSDL_mixer -lpthread 
BIN = davegnukem
BINED = ed


ifeq ($(OS),Windows_NT)
    #CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        #CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            #CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            #CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        #CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        LIBS += -framework Cocoa 
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        #CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        #CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        #CCFLAGS += -D ARM
    endif
endif


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

default: gnukem

gnukem: $(OBJFILES)
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
	$(CPP) $(CCFLAGS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CCFLAGS) -c $< -o $@

