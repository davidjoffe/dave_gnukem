# Dave Gnukem
Dave Gnukem is a retro-style 2D scrolling platform shooter similar to, and inspired by, Duke Nukem 1 (~1991). The source code is cross-platform and open source. It runs on Windows, Linux, Mac OS X and more. (The original Duke Nukem 1 had 16-color EGA 320x200 graphics; the aim here is 'similar but different' gameplay and 'look and feel'.)

Please donate to the project, or else I'll make more games like this: https://paypal.me/davidjoffe

**Release Downloads:** https://sourceforge.net/projects/gnukem/

**Direct Download Link:** https://sourceforge.net/projects/gnukem/files/latest/download?source=files

**Source code repository:** https://github.com/davidjoffe/dave_gnukem

**Repository for datasrc:** https://github.com/davidjoffe/gnukem_datasrc

**Repository for 'data' subfolder:** https://github.com/davidjoffe/gnukem_data

**Project page**: http://djoffe.com/gnukem/

**Mini Demo Video**: https://youtu.be/Hi7WYnOA_fo


![Dave Gnukem Screenshot](http://scorpioncity.com/dave_gnukem/gallery/Dave%20Gnukem%200.90%20%2813%20Jan%202018%29.png "Dave Gnukem Screenshot")

### Who is to blame for this?

Created by: David Joffe http://djoffe.com/ / https://www.liveedu.tv/david_joffe/ / https://minds.com/David_Joffe

The project was originally created and maintained by David Joffe (~1994 to 2004, and Oct 2016 to present). It was maintained by EMH (Evil Mr Henry http://www.emhsoft.com/) from 2004 to 2008. Additional contributions by: [T.O.G](http://www.nuke24.net/ "T.O.G."), [Vytautas Shaltenis](https://rtfb.lt/), Kent Mein, Steve Merrifield, Felix Richter, Kevin Joffe. See also 'Additional Credits' below.

As of 8 Oct 2016, this project is under 'properly' active development again, and the goal/intention is to get to a 'version 1' by no later than Oct 2017^H^H^H^H^H^H^H^HApr 2018.

### About Duke Nukem 1

Duke Nukem 1 was a famous original 16-color 320x200 'classic' game released by Apogee Software in 1991 that launched the Duke Nukem series: https://goo.gl/yP4PbS  The original Duke Nukem 1 was created by Todd Replogle (co-creator of the Duke Nukem series), John Carmack (of id Software), Scott Miller (founder of 3D Realms), Allen H. Blum III, George Broussard, and Jim Norwood.

### Development LiveStreaming Channels

You can watch development on this project being live-streamed at:

* https://www.liveedu.tv/david_joffe/
 * https://www.liveedu.tv/david_joffe/l98yv-dave-gnukem-side-scrolling-shooter/ (full development video archive here)
 * https://www.livecoding.tv/david_joffe/videos/wae18-dave-gnukem-c-game-side-scrolling-shooter and 
 * https://www.livecoding.tv/david_joffe/videos/K9XBz-dave-gnukem-c-game-side-scrolling-shooter-3 etc.
* https://www.twitch.tv/david_joffe/
* https://www.youtube.com/user/davidjoffe2
* https://www.pscp.tv/DavidJoffe

### (Approximate) Roadmap to Version 1

* Jul/Aug 2017: v0.8: Implement many more monster types, and a few other special 'things' (e.g. rockets etc.)
* Aug/Sep 2017: v0.9: Implement equivalent of Dr Proton and finalize plotline etc.; Finalize one basic set of playable levels. Finalize and round off spritesets to be 'not entirely terrible'.
* Apr 2018: v1.0: Final rounding-off and final testing, 'call it version 1'.

### Notes

Note the aim of Dave Gnukem is not to be a 'clone', nor to try match DN1 gameplay 'precisely' - the goal is to be 'similar but different', more like a parody though not really funny. Feasibly, the source could be used to attempt that, but it's not the present purpose or intention of this project. Likewise, it's not the intention to try load actual original level or sprite data into this 'engine', though again it'd probably be feasible to adapt it for such. - David Joffe, 2017-07

## Gameplay Instructions

Try find the exit, while dodging or shooting monsters.

### Default Keys:

* Left/Right: Move left/right
* Ctrl: Jump
* Alt: Shoot
* Up Arrow: Action key (for e.g. opening doors, using teleporters or lifts, activating the exit, etc.)
* Escape: In-game menu

* PgUp/PgDn: Increase/decrease volume
* Insert: Toggle sounds on/off
* Shift+F6/F7: Dec/Inc speed (framerate)
* Shift+F8/F9: Toggle map and sprite auto-shadows on/off, respectively
* F10 Save screenshot

To open doors, find the correct color key, and press the action key on the 'lock' nearby the door(s).

Power boots allow you to jump higher. The special molecule pickup gives you full health.

### Storyline/Plot:

The year is $CURRENTYEAR+8. An evil genius, Dr Proetton, has been hired by the CIA to infect the world's computers with a virus called SystemD, crippling them. Only you can stop him. You must find the floppy disk with the Devuan Antivirus on it, and install it on the master computer, which is hidden in Vault7.

*Any resemblance to actual persons or entities is purely coincidental*

## Features

* Now with less crashes
* Game taking forever to complete, like Duke Nukem Forever
* Shootable bananas
* Integrated level editor
* Subliminal messages
* Puns
* Evil SuperTux
* SystemD integration
* Big rockets
* Software doesn't spy on you
* Cutting edge lighting effects you'd expect from 1990
* Family friendly
* Teleporters
* Fans that blow you ;)
* Something that sort of resembles a cannon on wheels
* Nostalgic references to technology nobody uses anymore
* Red balloons
* Arcane command-line parameters nobody will use
* Overweight hero to represent rise in obesity rates since original 1991 release
* A surprisingly bearable 18 frames per second
* Detects and corrects wrongthink
* Bugs older than your children
* Floors
* Walls
* Doors
* Much much more!

# Ports

* Open Pandora: http://repo.openpandora.org/?page=detail&app=davegnukem-magicsam - release thread: https://pyra-handheld.com/boards/threads/dave-gnukem.79533/ (by https://github.com/sviscapi)
* MorphOS [Mar 2018, by Bruno Peloille]: http://www.morphos-storage.net/?page=Games%2FShoot+2D&file=Davegnukem_0.91.lha

# History / Changes

Recent:

* [30 Mar 2018] Add 'About' page to main menu
* [30 Mar 2018] Add new F10 save screenshot function (saves on Windows under \Users\USERNAME\AppData\Roaming\DaveGnukem\screenshots and on Linux under $HOME/.gnukem/screenshots)
* [26 Mar 2018] Bugfixes and level improvements (also fixed a level bug where one of the levels was impossible to complete)
* [22 Mar 2018] Boot sprite; Improve Dr Proetton (Dr Proton) sprite and behavior
* [20 Mar 2018] Fix overly-large window if Windows DPI scaling settings are enabled (unfortunately the fix breaks Windows XP support - but you can enable djWINXP_SUPPORT in config.h if you want to generate a build that supports Windows XP)

2018/03/11 - Version 0.91

* [11 Mar 2018] Create a Windows installer
* [22 Jan 2018] Add 'easter egg' [arin_j stream viewer suggestion]
* [22 Jan 2018] Level editor highlights instances of selected sprite (yellow), and sprite selection mouse-over sprite (cyan) (F3 toggles this ON/OFF if/when it gets annoying)
* [13 Jan 2018] New slightly experimental map auto-shadows and sprite auto-drop-shadows
* [13 Jan 2018] New shortcut keys Shift+F8/F9: Toggle map and sprite auto-shadows on/off, respectively
* Overhauled shooting/bullets code
* Level Editor: New "Ctrl+F7" command shows overview of all levels, and some basic stats on 'important' gameplay items
* Ctrl+Shift+W Large gameplay viewport mode (currently classified as a 'cheat')
* All cheat keys (and debug/testing keys like H to inflict self-damage) must now first be enabled by pressing Ctrl+Shift+G to enable in-game debugging stuff
* New cheat Backspace+PgUp increases firepower by one (added for testing)
* Numerous bugfixes and gameplay tweaks

2017/08/12 - Version 0.81 [Linux/Mac source release, Windows source and binary]:

* Add water
* Add very basic Dr Proetton ('final boss') and basic foundations of game ending sequence when you beat Dr Proetton
* Add jump-monster
* Add rockets
* Level Editor: New "Ctrl+Alt+N" command to generate a new blank level (NB: Still needs to be added manually to default.gam!)
* Fix a couple of bugs relating to jump height and powerboots

2017/08/05 - Version 0.72 [Linux/Mac source release, Windows source and binary]:

* Add new 'sort of looks like a cannon on wheels' monster type
* Implemented new inventory pickup item 'antivirus disk' and corresponding 'master computer' (when we insert AV disk into master computer we save the world) (should only be one of each of these, and in last level of a 'mission')
* Add new high-voltage "barrier" that must be shot multiple times to destroy before hero can pass through (touching it results in immediate death)
* Add new monster that constitutes the approximate equivalent of DN1 rabbits (for now, or maybe permanently, this is just a sort of 'evil Tux' - not sure yet if placeholder sprite, or part of plot)
* Add new floor type that auto-crumbles after hero walks or jumps twice on it (2nd spriteset, position 5x4 - place only 1 block per segment)
* Re-do instructions screens to display storyline/plot
* Basic more proper/correct implementation of the CAcme falling blocks
* Fixed: Bullets go through doors

2017/06/22 - Version 0.71 [Linux/Mac source, binary Windows only]:

* Add new 'flying robot' monster type
* Implemented access card and access card 'door' and door activator
* Added special molecule pickup that gives you full health
* Level Editor: Add unsaved-changes indication
* Level Editor: Add level statistics page (Ctrl+F6)
* Improve character control: Make vertical jumping/falling movement a bit smoother (more similar to the original DN1)
* Improve viewport vertical auto-scrolling
* New keyboard shortcuts: Shift+F6/F7: Dec/Inc speed (framerate)
* Fix soda can not animating

2017/06/22 - Version 0.70 [Linux/Mac source, binary Windows only]

* Add new basic 'Instructions' option to in-game menu
* Fix: Potential game 'freeze' in teleporter if exiting level editor (or holding in action key)
* Added "-scale N" command line option (1 = 320x200, 2=640x400, etc.) (if passed, this overrides the default behavior of attempting to "intelligently" select the scale factor based on e.g. desktop resolution etc.)
* Fix: "Key polling behavior is subtly incorrect" (this should slightly improve the character control, especially if trying to move left/right one block only)
* Minor sprite and level improvements
* Add hero jump/landing sounds (credit for the sounds: Juhani Junkala), key pick-up sound, and more
* Increase gameplay viewport width from 12 to 13 blocks

2016/10/29 - Version 0.66 [Linux/Mac source, binary Windows only]

* Add shortcut key to toggle sounds on/off (Insert)
* Fix: On Windows, game window often starts slightly off bottom of screen
* Fix: Restore Game from main menu doesn't work
* Fix: Save/Restore game doesn't save "mission" (i.e. selected "game")
* New proper exit sound (is "PowerUp13.mp3" by Eric Matyas http://soundimage.org/)
* Fixed: A monster could kill you in the moments after you entered the exit
* Better hero shooting sound

2016/10/22 - Version 0.63 / 0.64 / 0.65

* Add background music. So far, have only added music by Eric Matyas http://soundimage.org/
* Add game volume controls (use PgUp and PgDn keys to increase/decrease volume from menus or in-game)
* Level Editor: New feature: Hold in Ctrl+Alt and click with the mouse to automatically start level with hero 'dropped in' to the clicked position as starting position (to help with level editing / testing)

2016/10/16 - Version 0.62

* Get Dave Gnukem compiling and running on Mac OS X: https://www.livecoding.tv/david_joffe/videos/jbBvb-dave-gnukem-c-game-side-scrolling-shooter-9
* Miscellaneous minor aesthetic improvements

2016/10/10 - Version 0.61

* Fix: "Dying in the pungee sticks will often cause a crash" (thank you to porter who pointed that out)
* Fix a segfault in Linux (possibly VNC only?) when navigating in-game menus and other sub-menus
* Add semi-experimental 'big viewport mode' (Backspace + B in game to toggle) to make level creation/testing easier for level editors

2016/10/09 - Version 0.6 (0.60)

* Take the 2008-08-06 v0.56a EMH release from SourceForge, create github project https://github.com/davidjoffe/dave_gnukem
* Fixed 'tiny game window'
* Fix some Linux compile issues, make a few other fixes/improvements (streamed on LiveCoding), call it v0.6 ('un-abandoning' project - DJ)

# Developer Info / Build Info

Dependencies: LibSDL1.2, LibSDL-Mixer 1.2

## Windows Build Instructions

* Visual Studio 2010 project is included.
* **NB** You must set your Visual Studio 'Working Directory' to ../../ in order to run this (under Project / Properties / Configure Properties / Debugging) or it won't find the data files and just immediately exit on run
* If you get an error about not finding SDL DLL(s) when running, set 'Project / Properties / Configure Properties / Environment' to: PATH=c:\your\path\to\DLLs;%PATH%

## Linux Build Instructions

Type 'make'

Run with ./davegnukem if it built correctly

Installing dependencies on Debian etc.:

apt-get install libsdl1.2-dev

apt-get install libsdl-mixer1.2-dev


## Mac OS X Build Instructions

Same as Linux - type 'make'. Run with ./davegnukem if it built correctly.

Dependencies: You may have to first install LibSDL1.2 and LibSDL-Mixer1.2 (these can be installed by downloading the source code and doing 'make' and 'make install' (as root) for each of these first).

# Level Editor Instructions

(New 1 Jul 2017) Level Editor how-to video: https://youtu.be/xiznDqg2BHg

Note there is currently no 'undo', so be careful. If happy with a set of work, save your changes.

NB: All in-game cheat/debugging keys (eg level up) must first be turned on by pressing Ctrl+Shift+G

* F5 from within the game invokes the level editor
* F1: Save current changes (NB: there is currently no 'unsaved changes' warning, so remember to save your changes, if desired)
* Escape: Exit level editor and start playing current level
* Hold in Ctrl+Alt and click with the mouse to drop in hero and start playing/testing level at the clicked position
* X: Toggle display/editing of 'foreground layer' (there are two 'layers', the background layer primarily for solid stuff, e.g. walls, and the foreground layer generally for e.g. objects or monsters, or other semi-transparent objects)
* Z: Toggle display/editing of 'background layer'
* M,N: Previous/Next spriteset. There is a left-click and right-click "sprite palette"; clicking the left or right mouse button in the map preview area places an object from the left-click or right-click palette, respectively. Click in the spriteset with the left or mouse button to select that sprite object to either the left or right-click "sprite palette".
* To select the hero start position in the map, select and place either the 2nd-last or 3rd-last object in the first spriteset (it looks like a figure with a left or right arrow, which indicates the start direction). There should only be one of these per level.
* To place the exit position in the map, select and place the last object in the first spritset. There should only be one of these per level.
* 1-9: Macros: Float the mouse cursor in the map preview area and press one of these shortcuts to place from a few pre-defined complex objects, e.g. crates.
* F: Do a 'horizontal fill' of the current selected sprite (on the current layer)
* To choose the desired level, use the Up+L 'cheat' from within the game
* To choose the desired 'game' i.e. 'mission' (i.e. set of levels), use the main game menu 'Select mission'
* Ctrl+F6: Show level sprite instance statistics
* Ctrl+F7: Show overview of all levels, with some basic stats on 'important' gameplay items

Game Cheats Useful for Testing:

* Up+L - Next level
* Backspace+P - Get Powerboots
* Backspace+G - 'God Mode'
* Backspace+PgDn - Get all keys, access card, antivirus disk, and full firepower
* H - Damage Health

Note that for active animated objects that consist in the spriteset of multiple sprites (e.g. each conveyor belt piece has 4 sprites, for its animation), you *must* place the *first* of the four in the map for the object to work correctly.

Keep in mind that once the hero has the powerboots, he'll be able to jump higher. So there is a sort of logical progression if you place powerboots in a level, i.e. they should be placed once per 'mission' (i.e. set of levels), and levels prior to that should assume the default lower jump height, levels after that point might possibly assume the higher jump height.

To add a new 'mission' (i.e. set of levels), edit the missions.txt file and add a new line containing the name of your .gam file, e.g. "mygame/mygame.gam" (without quotes). (You should try keep all files associated with a particular game/mission in its own subfolder.) To add/remove levels for that game/mission, edit the .gam file (see the included .gam files to see the format).


# TODOs (To Sort):

* Create 'Roadmap' to "version 1"? E.g. starter list below:
* Videos?
* Check through code for possible buffer over-runs and 
* v0.7 [stil issue with ctrl jump keypress story]
* v0.7 linux segfault
* 'Texture Manager':
	* Clean up of 'textures'
	* Actual hardware accel if available [todo: proper blending]
	* Restoring hardware surfaces on toggle fullscreen with DirectX
* Explosion sound doesn't always play (fixed 2016-10-22 but fix could use more testing, cf. channel -1 story)
* DEV
	* Update TargetName to DaveGnukem in VS2010
	* Set up working folders to work 'out the box', so to speak
	* Add a dev readme.txt to help devs get set up

* [med] Level Editor is hogging CPU [check sprite editor, does it also?]
* Finalize sprites and other graphics to at least reasonable ready state
* Finalize a basic playable set of levels
* Check for all possible hardcoded keys that may be interfering with redefined keys (e.g. Ctrl, H, backspace+G etc.)
* [After redefine keys] Ctrl *both* shoot and jump
* H to damage health should not be on by default [likewise other things]
* Better colors for menu
* Some more / better sounds
* CREDITS: Add Vytautas Shaltenis, a.k.a. rtfb, EMH, etc. [who else?]
* Some better graphics
* Fix up and update credits etc.
* Signed executables in binary release?
* [check] Dying seems to reset score to 0 - is that right? Cf. DN1 behavior
* Is the -640 flag now basically obsolete? Remove? Or change to do something else useful? E.g. to make window maximum size e.g. same as monitor could be useful for level editing.

## Sprite TODOs:

* Some of the background brick colors should be a bit darker
* Some smoothing on some of hard pixely 'lines' (e.g. on shadow edges)
* Finish conveyor belt
* Un-EGA-ish sprites that still look a bit too EGA-ish (some of the especially ones look a bit too 16-color-ish perhaps, though the original DN1 was 16 colors)
* Red background pipes look a bit .. I dunno what .. chromatic? Maybe desaturate slightly .. or something

## Level TODOs:

* ? Make a list of 'level TODOs' that are must-have for a 'version 1'?


# Notes

The name is a pun on 'Duke Nukem' and 'Gnu' as a sort of 'hat-tip'/reference to the open source license/model.

This wasn't a retro game when I started on it (~1994ish), but it's taken so long it's now 'de facto' retro genre. So we are copying also the Duke Nukem Forever model of taking forever to be released.

# Additional Credits

* 2018-03-22 Add boots sprite made by freepik.com, 7from flaticon.com, license Creative Commons BY 3.0
* 2017-08-04 Add font data/fonts/simple_6x8.tga by http://www.zingot.com/ from https://opengameart.org/content/bitmap-font-pack License https://creativecommons.org/licenses/by/3.0/ (small changes made to color, and convert from PNG to TGA)
* 2016-10-30 data/sounds/soft_explode.wav From same Juhani Junkala collection as per below
* 2016-10-30 data/sounds/key_pickup.wav http://opengameart.org/content/key-pickup author Vinrax, license 'CC BY 3.0' https://creativecommons.org/licenses/by/3.0/
* 2016-10-30 data/sounds/jump.wav and data/sounds/jump_landing.wav From same Juhani Junkala collection as per below
* 2016-10-23 data/sounds/shoot\_cg1_modified.wav Slightly modified version of cg1.wav from http://opengameart.org/content/chaingun-pistol-rifle-shotgun-shots by Michel Baradari http://michel-baradari.de/
	"Sounds (c) by Michel Baradari apollo-music.de
	Licensed under CC BY 3.0 http://creativecommons.org/licenses/by/3.0/
	Hosted on opengameart.org"
* 2016-10-23 data/sounds/sfx\_weapon_singleshot7.wav by Juhani Junkala (CC0 creative commons license) ("The Essential Retro Video Game Sound Effects Collection [512 sounds]")
* 2016-10 Thanks to daveywavey @ livecoding https://www.livecoding.tv/daveywavey/ for help with setting up github repo
