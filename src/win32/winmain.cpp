
//dj2022-11 add this "#ifdef WIN32" so we can in theory more 'safely' possibly do things like scan the src folder for all .cpp files for the Makefile, so if a Linux or Unix-style system tries to compile this file it should just quietly do nothing instead of give an error
#ifdef WIN32

#include "../config.h"
#include "../djgraph.h"
#include "../sys_error.h"
#include <windows.h>


extern int main( int argc, char ** argv );

int WINAPI WinMain(HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        PSTR szCmdLine,
                        int iCmdShow)
{
	// FIXME: The intel compiler thinks that you cannot do this .. ???
	// Run the standard "main" function
	return main(__argc, __argv);
}

#endif//WIN32
