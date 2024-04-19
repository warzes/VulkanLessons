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
#include "09_TextureCubeMap.h"
#include "10_TextureCubemapArray.h"
#include "11_Texture3D.h"
#include "12_InputAttachments.h"
#include "13_Subpasses.h"
#include "14_Offscreen.h"
#include "15_ParticleSystem.h"
#include "16_Stencilbuffer.h"
#include "17_VertexAttributes.h"
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
	//TextureArraysApp app;
	//TextureCubeMapApp app;
	//TextureCubemapArrayApp app;
	//Texture3DApp app;
	InputAttachmentsApp app;
	//SubpassesApp app;
	//OffscreenApp app;
	//ParticleSystemApp app;
	//StencilbufferApp app;
	//VertexAttributesApp app;

	app.Run();

}
//-----------------------------------------------------------------------------