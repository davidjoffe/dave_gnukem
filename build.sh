#!/bin/sh
# Very small build helper script
# This script is not important to the core source and it is suggested to avoid building anything based on this script as it may change or move etc. in future - it's just to save time for dev

@echo Make sure you have first installed dependencies if necessary, e.g. "sudo apt install libsdl2-dev", "sudo apt install libsdl2-mixer-dev"

make clean && make -j8

# Still thinking about this DdjUNICODE_SUPPORT etc. dj2024:
#make clean && make -j8 CXXFLAGS="-DdjUNICODE_SUPPORT"
