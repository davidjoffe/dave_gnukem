#!/bin/sh
# Very small build helper script
# This script is not important to the core source and it is suggested to avoid building anything based on this script as it may change or move etc. in future - it's just to save time for dev

make clean && make -j8

# Still thinking about this DdjUNICODE_SUPPORT etc. dj2024:
#make clean && make -j8 CXXFLAGS="-DdjUNICODE_SUPPORT"
