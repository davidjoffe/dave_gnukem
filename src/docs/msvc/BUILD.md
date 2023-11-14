# Building with Visual Studio

## Install dependencies

You may use vcpkg as a relatively simple suggested way of installing the dependencies for building Dave Gnukem with Visual Studio.

If you don't have vcpkg set up, first follow the basic vcpkg instructions (i.e. in short: clone the vcpkg folder into a local folder, open a Visual Studio Developer Command Prompt, go to that folder, run vcpkg's bootstrap initialization, then install the dependencies as per below for x86 (win32) and/or 64-bit if desired (that's optional), and do the usual once-off 'vcpkg integrate install' for linking your vcpkg folder to Visual Studio - 'vcpkg integrate install' might need a command prompt with administrator rights):

NB do all of the abovementioned and below in a *Developer Command Prompt** for Visual Studio, not a normal cmd prompt:

```cmd
set VCPKG_DEFAULT_TRIPLET=x86-windows
vcpkg install sdl2
vcpkg install --recurse sdl2-mixer[libvorbis]
```

The below are also recommended, though not yet used in official builds, may be added soon for new features toward localization etc.:

```cmd
vcpkg install sdl2-image
vcpkg install --recurse sdl2-ttf
vcpkg install --recurse sdl2-ttf[harfbuzz]
```

(If you want to set up a 64-bit build run "set VCPKG_DEFAULT_TRIPLET=x64-windows" and repeat above "vcpkg install" steps, though it isn't necessary)

(The above listed Harfbuzz TTF extensions may be necessary only for specifically Arabic text support in future planned localization - not yet necessary - check config.h for the Unicode-related settings as to whether you have and want to use harfbuzz for Arabic text, if disabled in config.h it's just not used.)

## Get a copy of the code

Either download and unzip from https://github.com/davidjoffe/dave_gnukem (and https://github.com/davidjoffe/gnukem_data) or do:

```sh
$ git clone https://github.com/davidjoffe/dave_gnukem
$ cd dave_gnukem
$ git clone https://github.com/davidjoffe/gnukem_data
```

## Opening in Visual Studio

Run Visual Studio (not Visual Studio Code), select to open a project or solution, then browse to the DaveGnukem/src/vcDave folder and select the vcDave.sln file.

If it prompts/warns to upgrade library versions, it's normally best to allow it to do so (though for official release builds one may want to target slightly older runtime versions for compatibility, but for general testing/development use whatever works best/simplest on your Visual Studio version and setup).

If there are development libraries or components you may still need, you may need to install them with the Visual Studio Installer first.

## Setting up include and lib folder settings

(Once-off setup) Edit the djVS* property sheet files under src/vcDave and enter your vcpkg folders for include and lib files in these property sheet files, then in Visual Studio project use Property Manager to right-click and add these property sheets (for Debug and Release builds, respectively - the file ending with a "D" is for Debug include/lib folder settings, the other for release).

For now only 32-bit settings are included but in future 64-bit should also be added to these projects.

## Runtime settings

(Once-off setup) In project "Configuration Properties" (right-click in Solution Explorer) change the "Debugging" "Working Directory" by APPENDING "/../.." so that it finds the data files etc. when it runs - it should look like: $(ProjectDir)/../.. (otherwise Dave Gnukem will just immediately close and exit instantly when you run)

## Build

Either use the Build menu in Visual Studio, or possibly the below shortcuts to do:

Build: F7 to build without running (NB make sure to take note whether you have selected Release or Debug)

Ctrl+F5 to build and run without debugger

F5 to build and run in debugger

(The above shortcut keys may differ on some setups)


## Running

Either use the Build menu in Visual Studio, or possibly the below shortcuts to do:

Ctrl+F5 to build and run without debugger

F5 to build and run in debugger

(The above shortcut keys may differ on some setups)
