#!/bin/bash
# Dave Gnukem
# https://github.com/davidjoffe/dave_gnukem
# dj2016-10 simple quick n dirty script to try automatically build a
# release DMG etc. for Mac OS X
set -e
set -u 

OUTFOLDER=MacReleases/DaveGnukem
TMPDMG=MacReleases/tmp.dmg
FINALDMG=${OUTFOLDER}.dmg

##########
# Actually build the Dave Gnukem binary
make clean
make


##########
# Create temporary output folder to store release files/folders
rm -rf MacReleases
mkdir -p ${OUTFOLDER}/DaveGnukem


##########
# COPY REQUIRED APPLICATION FILES/FOLDERS
cp -r data ${OUTFOLDER}/DaveGnukem
cp davegnukem ${OUTFOLDER}/DaveGnukem
cp COPYING ${OUTFOLDER}/DaveGnukem
cp README.md ${OUTFOLDER}/DaveGnukem
cp History.txt ${OUTFOLDER}/DaveGnukem
# Throw in the source as well I guess? May encourage someone to play around with it, and it isn't big
cp Makefile ${OUTFOLDER}/DaveGnukem
cp -r src ${OUTFOLDER}/DaveGnukem

# Make sure all files that will go into the DMG are world-readable (otherwise if one user installs the app, and another runs it, some things might not work properly as the other user can't read those files if they're 'user-readable' only)
find ${OUTFOLDER} -print0 | xargs -0 chmod a+r 
# Not sure if this will be effective in DMG? But idea here is want levels etc. editable if someone copies the folder out to elsewhere .. so make data folder writable
find ${OUTFOLDER}/DaveGnukem/data -print0 | xargs -0 chmod u+w 

##########
# Now create DMG; this is a two-step process

# Remove these first if there's some lying around from a previous build attempt
rm -f ${TMPDMG}
rm -f ${OUTFOLDER}.dmg

hdiutil create -srcfolder ${OUTFOLDER} -ov ${TMPDMG} -volname "Dave Gnukem"

hdiutil convert ${TMPDMG} -format UDBZ -o ${FINALDMG}


##########
# Clean up temp files a bit
rm -f ${TMPDMG}

