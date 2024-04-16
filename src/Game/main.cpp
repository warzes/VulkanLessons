#include "stdafx.h"
#include "Engine/WindowsInclude.h"
#include "GameApp.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
//#	pragma warning(disable : 5045)
#	pragma warning(push, 0)
#endif
#if defined(_MSC_VER)
#	pragma warning(pop)
#endif
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "3rdparty.lib" )
#	pragma comment( lib, "Engine.lib" )
#endif
//-----------------------------------------------------------------------------
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	GameApp app;
	app.Run();
}