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

CPP = em++
CC = emcc
# todo - reconcile Makefile changes for emsdk
#CXX = ...
#FLAGS=-s WASM=1 -s USE_SDL_MIXER=1 --preload-file ./data -s ALLOW_MEMORY_GROWTH=1 --use-preload-plugins -s STB_IMAGE=1
CXXFLAGS=-s WASM=1 -s USE_SDL_MIXER=1 --preload-file ./data -s ALLOW_MEMORY_GROWTH=1 --use-preload-plugins -s STB_IMAGE=1

BIN = davegnukem.html
# 'version string history' here:
# "v1.0 - 3 Apr 2018" [version 1]
# "v1.0.1 - 25 Apr 2020"
# "v1.0.2 - 19 Nov 2022" [<- last version on SDL1 - about to update to SDL2]
# "v1.0.3 - 19 Nov 2022" [New version number for SDL2 version with Matteo Bini SDL2 commit merged in] (1.0.3-dev, working towards official new 1.0.3 stable)
# "v1.0.3 - 29 Nov 2022" First official stable version with Matteo Bini's updates to SDL2, and new improved Debian packaging files by Matteo Bini
V_NUM    = 1.0.3
V_DATE   = 29 Nov 2022
VERSION  = v$(V_NUM) - $(V_DATE)

# paths
PREFIX   = /usr/local
BIN_DIR  = $(PREFIX)/games
DATA_DIR = $(PREFIX)/share/games/$(BIN)/# the trailing slash is required for paths in the source

LIBS               = `sdl2-config --libs` -lSDL2_mixer
LDFLAGS_DAVEGNUKEM = $(LIBS) $(LDFLAGS)

CPPFLAGS_DAVEGNUKEM = -DDATA_DIR=\"$(DATA_DIR)\" -DVERSION=\"'$(VERSION)'\" $(CPPFLAGS)

OBJ = $(shell find src -iname '*.cpp' -type f | sed 's/\.cpp$$/.o/' | sort)

# debug
#CXXFLAGS_DAVEGNUKEM = -ggdb -DDEBUG -std=c++14 -Wall `sdl2-config --cflags` $(CPPFLAGS_DAVEGNUKEM) $(CXXFLAGS)
CXXFLAGS_DAVEGNUKEM = -Os -std=c++14 -Wall `sdl2-config --cflags` $(CPPFLAGS_DAVEGNUKEM) $(CXXFLAGS)

all: options davegnukem

options:
	@echo davegnukem build options:
	@echo "CXXFLAGS = $(CXXFLAGS_DAVEGNUKEM)"
	@echo "LDFLAGS  = $(LDFLAGS_DAVEGNUKEM)"
	@echo "CXX      = $(CXX)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS_DAVEGNUKEM) -c $< -o $@

davegnukem: $(OBJ)
	$(CXX) -o $(BIN) $(OBJ) $(LDFLAGS_DAVEGNUKEM)

clean:
	rm -f $(BIN) $(BIN)-$(V_NUM).tar.gz
	find src -name '*.o' | xargs rm -f

dist: clean
	mkdir $(BIN)-$(V_NUM)
	ls | sed '/data/d; /$(BIN)-$(V_NUM)/d' | xargs -I {} cp -R {} $(BIN)-$(V_NUM)
	tar cf $(BIN)-$(V_NUM).tar $(BIN)-$(V_NUM)
	gzip $(BIN)-$(V_NUM).tar
	rm -fr $(BIN)-$(V_NUM)

install: 
	# appstream file
	mkdir -p $(DESTDIR)$(PREFIX)/share/metainfo
	cp -f debian/appstream/com.djoffe.$(BIN).metainfo.xml $(DESTDIR)$(PREFIX)/share/metainfo
	chmod 644 $(DESTDIR)$(PREFIX)/share/metainfo/com.djoffe.$(BIN).metainfo.xml
	# binary
	mkdir -p $(DESTDIR)$(BIN_DIR)
	cp -f $(BIN) $(DESTDIR)$(BIN_DIR)
	chmod 755 $(DESTDIR)$(BIN_DIR)/$(BIN)
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
	# icons
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps
	cp -f debian/icons/hicolor/32x32/apps/$(BIN).png $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps
	chmod 644 $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/$(BIN).png
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/hicolor/128x128/apps
	cp -f debian/icons/hicolor/128x128/apps/$(BIN).png $(DESTDIR)$(PREFIX)/share/icons/hicolor/128x128/apps
	chmod 644 $(DESTDIR)$(PREFIX)/share/icons/hicolor/128x128/apps/$(BIN).png
	# manual page
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man6
	sed 's/VERSION/$(VERSION)/' < debian/$(BIN).6 > $(DESTDIR)$(PREFIX)/share/man/man6/$(BIN).6
	chmod 644 $(DESTDIR)$(PREFIX)/share/man/man6/$(BIN).6

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/share/metainfo/com.djoffe.$(BIN).metainfo.xml
	rm -f $(DESTDIR)$(BIN_DIR)/$(BIN)
	rm -fr $(DESTDIR)$(DATA_DIR)
	rm -fr $(DESTDIR)$(PREFIX)/share/doc/$(BIN)-data
	rm -f $(DESTDIR)$(PREFIX)/share/applications/$(BIN).desktop
	rm -fr $(DESTDIR)$(PREFIX)/share/doc/$(BIN)
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/32x32/apps/$(BIN).png	
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/128x128/apps/$(BIN).png
	rm -f $(DESTDIR)$(PREFIX)/share/man/man6/$(BIN).6

.PHONY: all options clean dist install uninstall
