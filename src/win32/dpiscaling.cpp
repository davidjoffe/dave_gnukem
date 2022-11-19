//dj2022-11 Refactoring just moving this klunky looking Win32-specific DPI scaling fix from 2018 from main.cpp to its own file for neatness

//dj2022-11 note we may just get rid of djWINXP_SUPPORT flag probably if this dynamic loading workaround generally works without causing problems then we don't need static linking (static linking is what causes XP issue because that function isn't present)
#ifdef WIN32

//#ifndef djWINXP_SUPPORT

// [dj2018-03] For DPI scaling overly-large-Window issue https://github.com/davidjoffe/dave_gnukem/issues/98
//#include <ShellScalingApi.h>//<'Newer' 'more correct' way rather than SetProcessDPIAware() but only Win8.1 or higher supported so commenting that out for
//#pragma comment (lib,"Shcore.lib")//<'Newer' 'more correct' way rather than SetProcessDPIAware()
#include <Windows.h>//SetProcessDPIAware();

//#endif // #ifndef djWINXP_SUPPORT

void djSetProcessDPIAwareHelper()
{
	// [dj2018-03] This is to fix https://github.com/davidjoffe/dave_gnukem/issues/98 .. note we effectively ignore DPI scaling,
	// which is probably OK in our situation as by default we automatically our scaling our Window to the largest size that fits in the display that is a multiple of 320x200.
	// Fix: Application window is being scaled with DPI scaling setting, and doesn't fit on screen by default, it's too large

#ifdef djDYNAMICALLY_BIND_SETPROCESSDPIAWARE
	// dj2018-04-10
	// We want to call SetProcessDPIAware to fix DPI scaling bug on Win10 etc.
	// But, that function was only introduced later in Windows +/- Vista-ish. So,
	// a normal statically-linked call breaks XP support. That didn't bother me
	// too much, but it seems to also break the (tested-on-latest-nightly) ReactOS.
	// I would have thought/imagined this should be simple for the ReactOS folks
	// to just add an empty stub SetProcessDPIAware() (it can do nothing since XP
	// doesn't have DPI scaling stuff anyway, so no such scaling bug) and possibly
	// fix thousands of other applications that call SetProcessDPIAware(), but
	// I don't know if they have good reasons or if it's just not on their radar etc.
	// In the meantime, this workaround uses GetProcAddress() and a function pointer
	// to 'dynamically bind to' and call the function; since it's not statically
	// linked, it then doesn't show the run-time error message of missing function
	// when you start the application - it should run and just quietly skip over
	// it as NULL here.
	// I am a little nervous about this sort of fix - have a feeling like it may
	// introduce instability here on some systems etc. somehow.
	// So I'm increasing risk of crashiness to support ReactOS (and XP).
	// There's another alternative - use the djWINXP_SUPPORT config option as it
	// had been to build a separate XP/ReactOS-supporting executable and have
	// say maybe a second shortcut (e.g. 'Dave Gnukem (XP)' in the Start menu.
	// That has pros and cons of its own. It's a bit ugly, and also, some users
	// may not see it - they'll just run the main shortcut and get a fault. Likewise,
	// some Win10 users will run the XP shortcut, and get an overly-large screen
	// if DPI scaling enabled.
	// (Another alternative is a separate download entirely for XP/ReactOS but
	// that's a pain to maintain, I don't want to do that.) Sigh. I've
	// written a whole book now here on this one stupid issue.

	Log("GetProcAddress SetProcessDPIAware\n");//Want lots of logging here as I feel like this might all be relatively higher risk of crashing
	typedef int (WINAPI* PFN)();//function signature for SetProcessDPIAware
	PFN MySetProcessDPIAware = NULL;
	MySetProcessDPIAware = (PFN)GetProcAddress(
		GetModuleHandle(TEXT("user32.dll")),
		"SetProcessDPIAware");
	if (NULL != MySetProcessDPIAware)
	{
		// Call SetProcessDPIAware() through our dynamic pointer to it
		Log("Call MySetProcessDPIAware\n");
		MySetProcessDPIAware();
		Log("Call MySetProcessDPIAware done\n");
	}
	else
	{
		Log("No GetProcAddress - possibly XP or ReactOS?\n");
	}
#else//else statically bind (this breaks XP, and, currently, ReactOS - causes error message about missing function when you run. dj2018-04)
	#ifndef djWINXP_SUPPORT
	//::SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);// Note we don't a manifest; this seems to work at the moment. "The DPI awareness for an application should be set through the application manifest so that it is determined before any actions are taken which depend on the DPI of the system. Alternatively, you can set the DPI awareness using SetProcessDpiAwareness, but if you do so, you need to make sure to set it before taking any actions dependent on the system DPI. Once you set the DPI awareness for a process, it cannot be changed."
	// NB: Unfortunately it looks like SetProcessDPIAware(); is only Vista or later,
	// the more correct newer way SetProcessDpiAwareness() is Win8.1 or later (doesn't build on VS2010)
	// so it looks like we miiight have to leave XP behind for correct behaviour on a system with DPI scaling set? :/
	// There will [esp going forward] probably be more users with overly-large DPI-scaled Windows, than XP users, so if it's one or the other, we might have
	// to leave XP behind rather. Would be interesting though if ReactOS, or WINE etc. support are negatively affected. I'm guessing not.
		::SetProcessDPIAware();//<- Note this apparently 'may cause race conditions' under some conditions - if we encounter that, then we may have to put this as a manifest setting instead.
	#endif//#ifndef djWINXP_SUPPORT
#endif//#ifdef djDYNAMICALLY_BIND_SETPROCESSDPIAWARE
}

#endif
