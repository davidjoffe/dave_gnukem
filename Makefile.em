#
# David Joffe
# Copyright 1998-2017 David Joffe
# Created 1998/12
# Makefile for Dave Gnukem
#
# 2016-10: Get this working on Mac OS X [dj]
# 2017-07-29: Remove obsolete standalone-editor-related stuff, and add new thing_monsters.o
#
# ~2019 - ~2023 In-progress work by Donovan Hutchence to get this working with Emscripten
#

CPP = em++
CC = emcc


# dj2016-10 Add L -I/usr/local/include/SDL in process of getting this working on Mac OS X - not sure if this is 'bad' to just have both /usr/include and /usr/local/include??
#INCLUDEDIRS= -I/usr/include/SDL -I/usr/local/include/SDL 

#CCFLAGS = -O -Wall $(INCLUDEDIRS)

# Un/recomment as needed for removing sound support, optimizations etc
# If you don't -DDATA_DIR to a valid dir, then data files will be assumed
# to be in current directory
#CCFLAGS = -Wall -I/usr/local/include -DHAVE_SOUND -DDEBUG -O -m486
CCFLAGS = -Wall -Wno-switch -DDEBUG $(INCLUDEDIRS)
#Release version:
#CCFLAGS = -O -Wall -I/usr/local/include -DHAVE_SOUND $(INCLUDEDIRS)

#LIBS = -lSDL -lSDLmain -lSDL_mixer -lpthread 
LIBS = -lSDL -lpthread 
BIN = davegnukem.html


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
           src/thing_monsters.o src/gameending.o \
           src/level.o    src/settings.o src/keys.o \
           src/djtypes.o  src/bullet.o \
           src/ed.o src/ed_DrawBoxContents.o src/ed_common.o src/ed_lvled.o \
           src/ed_macros.o src/ed_spred.o \
           src/sdl/djgraph.o src/sdl/djinput.o src/sdl/djsound.o \
           src/sdl/djtime.o \
           src/sys_error.o src/sys_log.o src/m_misc.cpp

FLAGS=-s WASM=1 -s USE_SDL_MIXER=1 --preload-file ./data -s ALLOW_MEMORY_GROWTH=1 --use-preload-plugins -s STB_IMAGE=1

default: gnukem

gnukem: $(OBJFILES)
	$(CPP) -o $(BIN) $(OBJFILES) $(LIBS) $(FLAGS)
clean:
	rm -f $(BIN) *~ core \#*
	find src -name '*.o' | xargs rm -f

dist:
	rm -f core *~ \#*
	find src -name '*.o' | xargs rm -f

linecount:
	cat src/*.cpp src/*.h src/sdl/*.h src/sdl/*.cpp | wc -l

fixme:
	ls src/*.c src/*.cpp src/*.h | xargs grep -i fixme

%.o: %.c
	$(CPP) $(CCFLAGS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CCFLAGS) $(FLAGS) -c $< -o $@

