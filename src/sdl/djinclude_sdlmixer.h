// Helper for including SDL mixer stuff
// This is turning into something a bit gross and really C++-ish ...
// This may I'm sure need tweaking further
#ifndef _DJINCLUDE_SDLMIXER_H_
#define _DJINCLUDE_SDLMIXER_H_

//#include "config.h"//For NOSOUND

//#ifndef NOSOUND

#ifdef __OS2__
	#include <SDL/SDL_mixer.h>
#else

	#if defined(__has_include)

		#ifdef WIN32

			// [dj2023-11] vcpkg starts to have this new sdl2-mixer-ext instead of the usual sdl2-mixer (for libvorbis especially I think we may need it) but I don't see it in Ubuntu WSL etc. so for now just on Windows start trying to sort of auto-figure out whether to use sdl2-mixer-ext or sdl2-mixer
			#if __has_include(<SDL_mixer_ext.h>)
				// [dj2023] add this define so we know we are using the ext lib for things like #pragma link setting on MSVC platforms
				#define djUSING_SDL_MIXER_EXT
				#include <SDL_mixer_ext.h>
			#elif __has_include(<SDL/SDL_mixer_ext.h>)
				#define djUSING_SDL_MIXER_EXT
				#include <SDL/SDL_mixer_ext.h>
			#elif __has_include(<SDL/SDL_mixer.h>)
				#include <SDL/SDL_mixer.h>
			#else
				#include <SDL_mixer.h>
			#endif

		#else

			#if __has_include(<SDL/SDL_mixer.h>)
				#include <SDL/SDL_mixer.h>
			#else
				#include <SDL_mixer.h>
			#endif

		#endif

	#else
		// No __has_include, just go the old way (except __OS2__ path which we already set up) .. this may all have to change and be tweaked again I'm sure ..
		#include <SDL_mixer.h>
	#endif

#endif//#ifdef __OS2__
//#endif//#ifndef NOSOUND

#endif//_DJINCLUDE_SDLMIXER_H_
