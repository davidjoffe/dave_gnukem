#!/bin/bash
# Dave Gnukem
# https://github.com/davidjoffe/dave_gnukem
# dj2016-10 simple quick n dirty script to try automatically build a
# release DMG etc. for Mac OS X
# dj2023-11 updating to use create-dmg for a more polished result and other improvements
# Also tries to recursively resolve .dylib dependencies
set -e
set -u 

# Check for version number argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <version_number>"
    exit 1
fi

APPNAME="Dave Gnukem"
VERSION=$1
#CODESIGNID=""

OUTFOLDER=MacReleases/DaveGnukem_$VERSION
#TMPDMG=MacReleases/tmp_$VERSION.dmg
FINALDMG=MacReleases/DaveGnukem_$VERSION.dmg

APPFOLDER="${APPNAME}.app"

##########
# Actually build the Dave Gnukem binary
make clean
make -j4

# hm why not statically link into one binary? no dylibs .. woudl simplify the packaging slightly ..

# Code-sign davegnukem binary itself
#echo codesign  --timestamp -v -f --sign "TSHWANEDJE HUMAN LANGUAGE TECHNOLOGY CC" davegnukem


##########
# Create temporary output folder to store release files/folders
rm -rf MacReleases
mkdir -p "${OUTFOLDER}/${APPFOLDER}"
mkdir -p "${OUTFOLDER}/${APPFOLDER}/Contents"
mkdir -p "${OUTFOLDER}/${APPFOLDER}/Contents/MacOS"
mkdir -p "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
# Generate / copy Info.plist file
cp -f "Info.plist" "${OUTFOLDER}/${APPFOLDER}/Contents/"
# Generate PkgInfo file
echo -n 'APPL????' > "${OUTFOLDER}/${APPFOLDER}/Contents/PkgInfo"

APP_BINARY="${OUTFOLDER}/${APPFOLDER}/Contents/MacOS/davegnukem"

##########
# COPY REQUIRED APPLICATION FILES/FOLDERS
cp -r data "${OUTFOLDER}/${APPFOLDER}/Contents/"
cp davegnukem "${OUTFOLDER}/${APPFOLDER}/Contents/MacOS"
cp "./data/icon/Dave Gnukem.icns" "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
cp "./data/icon/Dave Gnukem.icns" "${OUTFOLDER}/${APPFOLDER}/Contents/Resources/appicon.icns"
cp COPYING "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
cp README.md "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
cp MIT-LICENSE.txt "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
cp History.txt "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
# Throw in the source as well I guess? May encourage someone to play around with it, and it isn't big
#cp Makefile "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"
#cp -r src "${OUTFOLDER}/${APPFOLDER}/Contents/Resources"


install_name_tool -change /opt/local/lib/libsdl2.dylib @executable_path/libsdl2.dylib "$APP_BINARY"
install_name_tool -change /opt/local/lib/libsdl2_mixer.dylib @executable_path/libsdl2_mixer.dylib "$APP_BINARY"

# Function to copy and fix dylib dependencies
copy_and_fix_dylibs() {
    local dylib=$1
    local dest_dir=$2

    # Copy the dylib to the destination directory
    cp "$dylib" "$dest_dir"

    # Get the base name of the dylib
    local dylib_name=$(basename "$dylib")

    # Get all dependencies of the dylib
    local dependencies=$(otool -L "$dylib" | awk 'NR>1 {print $1}' | grep -v ':' | grep '/opt/local/lib')

    for dep in $dependencies; do
        local dep_name=$(basename "$dep")

        # Change the install name for the dependency within the dylib
        install_name_tool -change "$dep" "@executable_path/../Frameworks/$dep_name" "$dest_dir/$dylib_name"

        # Check if the dependency is already processed
        if [ ! -f "$dest_dir/$dep_name" ]; then
            # Recursively process the dependency
            copy_and_fix_dylibs "$dep" "$dest_dir"
        fi
    done
}

#cp /opt/local/lib/libsdl2.dylib "${OUTFOLDER}/${APPFOLDER}/Contents/MacOS/"
#cp /opt/local/lib/libsdl2_mixer.dylib "${OUTFOLDER}/${APPFOLDER}/Contents/MacOS/"

# Process SDL2 and SDL2_mixer dylibs
# Directory where the dylibs will be copied to
FRAMEWORKS_DIR="${OUTFOLDER}/${APPFOLDER}/Contents/MacOS"
#mkdir -p "$FRAMEWORKS_DIR"
copy_and_fix_dylibs "/opt/local/lib/libsdl2.dylib" "${FRAMEWORKS_DIR}"
copy_and_fix_dylibs "/opt/local/lib/libsdl2_mixer.dylib" "${FRAMEWORKS_DIR}"

# Make sure all files that will go into the DMG are world-readable (otherwise if one user installs the app, and another runs it, some things might not work properly as the other user can't read those files if they're 'user-readable' only)
find "${OUTFOLDER}" -print0 | xargs -0 chmod a+r 
# Not sure if this will be effective in DMG? But idea here is want levels etc. editable if someone copies the folder out to elsewhere .. so make data folder writable
find "${OUTFOLDER}/${APPFOLDER}/Contents/data" -print0 | xargs -0 chmod u+w 




##########
# Now create DMG; this is a two-step process


# Sign the .app itself
#echo codesign  --timestamp -v -f --sign "TSHWANEDJE HUMAN LANGUAGE TECHNOLOGY CC" --deep "${OUTFOLDER}/${APPFOLDER}"

# Update dmg creation to use create-dmg for a more polished result
create-dmg --volname "${APPNAME} $VERSION" "${FINALDMG}" "${OUTFOLDER}"

# Sign the .dmg itself
#echo codesign  --timestamp -v -f --sign "TSHWANEDJE HUMAN LANGUAGE TECHNOLOGY CC" --deep "${FINALDMG}"


# Remove these first if there's some lying around from a previous build attempt
#rm -f ${TMPDMG}
#rm -f ${OUTFOLDER}.dmg

#hdiutil create -srcfolder ${OUTFOLDER} -ov ${TMPDMG} -volname "Dave Gnukem"
#hdiutil convert ${TMPDMG} -format UDBZ -o ${FINALDMG}


##########
# Clean up temp files a bit
#rm -f ${TMPDMG}

