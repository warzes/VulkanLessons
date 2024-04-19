#include "stdafx.h"
#include "GameApp.h"
#include "01_TriangleApp.h"
#include "02_PipelinesApp.h"
#include "03_DescriptorSets.h"
#include "04_DynamicUniformBuffer.h"
#include "05_PushConstants.h"
#include "06_SpecializationConstants.h"
#include "07_Texture.h"
#include "08_TextureArrays.h"
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
	//DescriptorSets app;
	//DynamicUniformBuffer app;
	//PushConstants app;
	//SpecializationConstants app;
	//TextureApp app;
	TextureArraysApp app;

	app.Run();

}
//-----------------------------------------------------------------------------