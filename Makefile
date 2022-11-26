#
# David Joffe
# Copyright 1998-2022 David Joffe
# Created 1998/12
# Makefile for Dave Gnukem
#
# 2016-10: Get this working on Mac OS X [dj]
# 2017-07-29: Remove obsolete standalone-editor-related stuff, and add new thing_monsters.o
# 2022-11-25: Cleanup and fix targets for new debian dir (dist, install and uninstall)
#

BIN = davegnukem
# 'version string history' here:
# "v1.0 - 3 Apr 2018" [version 1]
# "v1.0.1 - 25 Apr 2020"
# "v1.0.2 - 19 Nov 2022" [<- last version on SDL1 - about to update to SDL2]
# "v1.0.3 - 19 Nov 2022" [New version number for SDL2 version with Matteo Bini SDL2 commit merged in]
V_NUM    = 1.0.3
V_DATE   = 19 Nov 2022
VERSION  = v$(V_NUM) - $(V_DATE)

# paths
PREFIX   = /usr/local
DATA_DIR = $(PREFIX)/share/games/$(BIN)/# the trailing slash is required for paths in the source

LIBS = `sdl2-config --libs` -lSDL2_mixer
LDFLAGS = $(LIBS)

CPPFLAGS = -DDATA_DIR=\"$(DATA_DIR)\" -DVERSION=\"'$(VERSION)'\"

CXX = g++

OBJ != find src -iname *.cpp -type f | sed --posix '/src\/ed_standalone_original.cpp/d; s/\.cpp$$/.o/'

ifeq ($(OS),Windows_NT)
    #CPPFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        #CPPFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            #CPPFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            #CPPFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        #CPPFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        LIBS += -framework Cocoa 
	# dj2022-11 add c++17 min here for Mac (might change this later to c++17)
	CXXFLAGS += -std=c++17
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        #CPPFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        #CPPFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        #CPPFLAGS += -D ARM
    endif
endif

# debug
#CXXFLAGS = -ggdb -DDEBUG -std=c++14 -Wall `sdl2-config --cflags` $(CPPFLAGS)
CXXFLAGS = -O2 -std=c++14 -Wall `sdl2-config --cflags` $(CPPFLAGS)

all: options davegnukem

options:
	@echo davegnukem build options:
	@echo "CXXFLAGS = $(CXXFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CXX      = $(CXX)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

davegnukem: $(OBJ)
	$(CXX) -o $(BIN) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(BIN) $(BIN)-$(V_NUM).tar.gz
	find src -name '*.o' | xargs rm -f

dist: clean
	mkdir $(BIN)-$(V_NUM)
	ls | sed --posix '/data/d; /$(BIN)-$(V_NUM)/d' | xargs -I {} cp -R {} $(BIN)-$(V_NUM)
	tar cf $(BIN)-$(V_NUM).tar $(BIN)-$(V_NUM)
	gzip $(BIN)-$(V_NUM).tar
	rm -fr $(BIN)-$(V_NUM)

install: 
	# binary
	mkdir -p $(DESTDIR)$(PREFIX)/games
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/games
	chmod 755 $(DESTDIR)$(PREFIX)/games/$(BIN)
	# data
	mkdir -p $(DESTDIR)$(DATA_DIR)
	cp -fR data/* $(DESTDIR)$(DATA_DIR)
	rm -f $(DESTDIR)$(DATA_DIR)README.md
	find $(DESTDIR)$(DATA_DIR) -type d | xargs chmod 755
	find $(DESTDIR)$(DATA_DIR) -type f | xargs chmod 644
	# data doc
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/$(BIN)-data
	cp -f data/README.md $(DESTDIR)$(PREFIX)/share/doc/$(BIN)-data
	chmod 644 $(DESTDIR)$(PREFIX)/share/doc/$(BIN)-data/README.md
	# desktop file
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	cp -f debian/desktop/$(BIN).desktop $(DESTDIR)$(PREFIX)/share/applications
	chmod 644 $(DESTDIR)$(PREFIX)/share/applications/$(BIN).desktop
	# doc
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/$(BIN)
	cp -f HISTORY.txt README.md $(DESTDIR)$(PREFIX)/share/doc/$(BIN)
	chmod 644 $(DESTDIR)$(PREFIX)/share/doc/$(BIN)/HISTORY.txt \
		$(DESTDIR)$(PREFIX)/share/doc/$(BIN)/README.md
	# icon
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps
	cp -f debian/icons/hicolor/32x32/apps/$(BIN).png $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps
	chmod 644 $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/$(BIN).png
	# manual page
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man6
	sed --posix 's/VERSION/$(VERSION)/' < debian/$(BIN).6 > $(DESTDIR)$(PREFIX)/share/man/man6/$(BIN).6
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man6/$(BIN).6

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/games/$(BIN) 
	rm -fr $(DESTDIR)$(PREFIX)/share/games/$(BIN) 
	rm -fr $(DESTDIR)$(PREFIX)/share/doc/$(BIN)-data
	rm -f $(DESTDIR)$(PREFIX)/share/applications/$(BIN).desktop
	rm -fr $(DESTDIR)$(PREFIX)/share/doc/$(BIN)
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/$(BIN).png	
	rm -f $(DESTDIR)$(PREFIX)/share/man/man6/$(BIN).6