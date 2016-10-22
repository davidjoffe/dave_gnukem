
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
