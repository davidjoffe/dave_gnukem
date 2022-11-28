
//dj2022-11 adding this small file for non-Makefile builds e.g. MSVS / Visual Studio etc. due to new Makefile now passing this in .. might re-do this differently later ..
/*
# 'version string history' here:
# "v1.0 - 3 Apr 2018" [version 1]
# "v1.0.1 - 25 Apr 2020"
# "v1.0.2 - 19 Nov 2022" [<- last version on SDL1 - about to update to SDL2]
# "v1.0.3 - 19 Nov 2022" [New version number for SDL2 version with Matteo Bini SDL2 commit merged in] (1.0.3-dev, working towards official new 1.0.3 stable)
# "v1.0.3 - 29 Nov 2022" First official stable version with Matteo Bini's updates to SDL2, and new improved Debian packaging files by Matteo Bini
*/

#ifndef _djVERSION_H_
#define _djVERSION_H_

// See also Makefile ..

#ifndef V_NUM
#define V_NUM "1.0.3"
#endif
#ifndef V_DATE
#define V_DATE "29 Nov 2022"
#endif
#ifndef VERSION
#define VERSION V_NUM " - " V_DATE
#endif

#endif
