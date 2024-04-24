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
#include "20_gltfloading.h"
#include "21_gltfskinning.h"
#include "22_gltfscenerendering.h"
#include "30_multisampling.h"
#include "31_hdr.h"
#include "32_shadowmapping.h"
#include "33_shadowmappingcascade.h"
#include "34_shadowmappingomni.h"
#include "35_texturemipmapgen.h"
#include "36_screenshot.h"
#include "37_oit.h"
#include "40_multithreading.h"
#include "41_Instancing.h"
#include "42_indirectdraw.h"
#include "43_occlusionquery.h"
#include "44_pipelinestatistics.h"
#include "50_pbrbasic.h"
#include "51_pbribl.h"
#include "52_pbrtexture.h"
#include "53_deferred.h"
#include "54_deferredmultisampling.h"
#include "55_deferredshadows.h"
#include "56_ssao.h"
#include "60_computeshader.h"
#include "61_computeparticles.h"
#include "62_computenbody.h"
#include "63_computecullandlod.h"
#include "64_geometryshader.h"
#include "65_viewportarray.h"
#include "66_displacement.h"
#include "67_terraintessellation.h"
#include "68_tessellation.h"
#include "70_textoverlay.h"
#include "71_distancefieldfonts.h"
#include "72_imgui.h"
#include "80_radialblur.h"
#include "81_bloom.h"
#include "82_parallaxmapping.h"
#include "83_sphericalenvmapping.h"
#include "90_conservativeraster.h"
#include "91_pushdescriptors.h"
#include "92_inlineuniformblocks.h"
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
	//InputAttachmentsApp app;
	//SubpassesApp app;
	//OffscreenApp app;
	//ParticleSystemApp app;
	//StencilbufferApp app;
	//VertexAttributesApp app;
	
	//GLTFLoadingApp app;
	//GLTFSkinningApp app;
	//GLTFSceneApp app;
	
	//MultisamplingApp app;
	//HDRApp app;
	//ShadowMappingApp app;
	//ShadowMappingCascadeApp app;
	//ShadowMappingOmniApp app;
	//TextureMipmapgenApp app;
	//ScreenshotApp app;
	//OITApp app;
		
	//MultithreadingApp app;
	//InstancingApp app;
	//IndirectDrawApp app;
	//OcclusionQueryApp app;
	//PipelineStatisticsApp app;
	
	//PBRBasicApp app;
	//PBRIBLApp app;
	//PBRTextureApp app;
	//DefferedApp app;
	//DeferredMultisamplingApp app;
	//DeferredShadowsApp app;
	//SSAOApp app;
	
	ComputeShaderApp app;
	//ComputeParticlesApp app;
	//ComputeNBodyApp app;
	//ComputeCullAndLodApp app;
	//GeometryShaderApp app;
	//ViewportArrayApp app;
	//DisplacementApp app;
	//TerrainTessellationApp app;
	//TessellationApp app;
	
	//TextOverlayApp app;
	//DistanceFieldFontsApp app;
	//ImguiApp app;
	
	//RadialBlurApp app;
	//BloomApp app;
	//ParallaxMappingApp app;
	//SphericalEnvMappingApp app;
	//ConservativerasterApp app;

	app.Run();

}
//-----------------------------------------------------------------------------