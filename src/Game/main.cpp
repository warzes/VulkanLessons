#include "stdafx.h"
#include "GameApp.h"
#include "01_TriangleApp.h"
#include "02_PipelinesApp.h"
#include "03_DescriptorSets.h"
//-----------------------------------------------------------------------------
#if defined(_MSC_VER)
#	pragma comment( lib, "3rdparty.lib" )
#	pragma comment( lib, "Engine.lib" )
#	pragma comment( lib, "vulkan-1.lib" )
#endif
//-----------------------------------------------------------------------------
int main(
	[[maybe_unused]] int   argc,
	[[maybe_unused]] char* argv[])
{
	//TriangleApp app;
	//PipelinesApp app;
	DescriptorSets app;

	app.Run();

}
//-----------------------------------------------------------------------------