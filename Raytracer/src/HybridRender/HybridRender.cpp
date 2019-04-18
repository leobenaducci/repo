#include "stdafx.h"
#include "AntTweakBar/include/AntTweakBar.h"

#if WIN64
#pragma comment(lib, "glew-1.10.0/lib/Release/x64/glew32s.lib")
#pragma comment(lib, "freeglut_bin/lib/x64/freeglut.lib")
#else
#pragma comment(lib, "glew-1.10.0/lib/Release/Win32/glew32s.lib")
#pragma comment(lib, "freeglut_bin/lib/freeglut.lib")
#endif

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

std::chrono::high_resolution_clock::time_point AppStartTime;

#define PER_SAMPLE_SHADING 0

#define SCENE_FROM_OBJ 1

#if SCENE_FROM_OBJ

RaytraceMesh SceneMesh;
std::vector<std::unique_ptr<Texture>> SceneTextures;

#else

SceneObject* pCar = nullptr;
SceneObject* pTethra = nullptr;
SceneObject* pTorus = nullptr;

RaytraceMesh CarMesh;
Texture CarTexture;

#endif

RaytraceScene CurrentScene;

Shader RaytraceShader, RaytracedReflectionsShader, RaytracedLightShader, RaytracedShadowsShader, RaytracedAmbientOcclusionShader, RaytracedIndirectLightShader;
Shader PathTraceShader;

Shader DeferredShader, LightShader, ForwardToCubeShader, ForwardShader, DepthOnlyShader;
Shader ToneMapperShader, ResolveShader, SimpleTexShader, BilateralShader, IndirectBlendShader;
Shader GaussBlurShader, MotionBlurShader, EnvMapShader;
Shader ScreenSpaceReflectionsShader;
Shader TSSAAShader, DepthOfFieldShader, DenoiseShader;

FrameBufferTexture GBuffer_RT0, GBuffer_RT1, GBuffer_RT2, GBuffer_RT3, GBuffer_DST;
FrameBuffer GBuffer, GBufferOnlyLinearDepth;

FrameBufferTexture LightBufferTexture;
FrameBuffer LightBuffer;

FrameBufferTexture PostProcessBufferColor, PostProcessBufferLinearDepth, PostProcessBufferVelocity, PostProcessBufferNormal, PostProcessBufferDepth;
FrameBuffer PostProcessBuffer, PostProcessBufferOnlyColor, PostProcessBufferOnlyDepth;

FrameBufferTexture PostProcessBufferColorOld, PostProcessBufferLinearDepthOld;
FrameBuffer PostProcessBufferOld, PostProcessBufferOldDepth;

FrameBufferTexture AuxBufferColor, AuxBufferDepth;
FrameBuffer AuxBuffer;

FrameBufferTexture BackBufferColor;
FrameBuffer BackBuffer;

FrameBufferTexture ShadowmapBufferColor, ShadowmapBufferTexture;
FrameBuffer ShadowmapBuffer;

//gauss blur never uses [0]
FrameBufferTexture BlurBufferColor[3], BlurBufferDepth;
FrameBuffer BlurBuffers[3];

FrameBufferTexture DepthOfField_COC_Color, DepthOfField_COC_Depth;
FrameBuffer DepthOfField_COC;

FrameBufferTexture DepthOfField_COC_BlurColor, DepthOfField_COC_BlurDepth;
FrameBuffer DepthOfField_COC_Blur;

GLuint BackBufferSize[2]{ 1280, 720 };
GLuint RenderTargetSize[2]{ 1280, 720 };
float ScreenPct = 1.f, NewScreenPct = 1.f;

Texture SkyBox2D[4];
int iSkyBox = 0;
float SkyBoxIntensity = 5.f;

glm::vec3 LightDir = glm::normalize(glm::vec3(1.0f, -1.f, -1.f));

glm::mat4 CubemapRotation;
glm::quat CubemapRotQuat;

GLenum PolygonMode = GL_FILL;

int FrameNum = 0;

int NUM_AA_SAMPLES = 1;
const char* NUM_AA_SAMPLES_STRING()
{
	static char str[128];
	sprintf_s(str, "NUM_AA_SAMPLES %i", NUM_AA_SAMPLES);

	return str;
}

Camera MainCamera, ShadowmapCamera;

bool bPathTracing = false;
bool bHalfRes = true;
bool bSeparatedSpecular = true;
bool bRaytracing = true;
bool bShowReflections = false;
bool bShowSSAO = false;
bool bShowBloom = false;
bool bShowDOF = false;
bool bShowMotionBlur = false;
bool bShowToneMap = true;
bool bShowAccumAA = true;
int SPP = 1;

float RoughnessBias = 0.f, MetallicBias = 0.f;

int TSSAA = 3;
const int TSSAA_Samples[] = { 1, 2, 4, 5, 9, 16 };

int NumBounces = 0;

float AAFrame = 0;

TwBar* TweakBar = nullptr;

static float frand()
{
	return ((float)rand()) / ((float)RAND_MAX);
}

void SetSceneParemeters(Shader* pShader, uint32_t Flags)
{
	CurrentScene.SetParameters(pShader);

	pShader->SetParameter("FrameNum", FrameNum++);

	if (bShowAccumAA)
	{
		const float Samples16[] =
		{
			-.375f, -.375f,
			 -.5f, -.375f,
			  .5f, -.375f,
			 .375f, -.375f,
			-.375f,  -.5f,
			 -.5f,  -.5f,
			  .5f,  -.5f,
			  .375f,  -.5f,
			-.375f,   .5f,
			 -.5f,   .5f,
			  .5f,   .5f,
			  .375f,   .5f,
			-.375f,  .375f,
			 -.5f,  .375f,
			  .5f,  .375f,
			  .375f,  .375f,
		};

		const float Samples9[] =
		{
			-.5f, -.5f,
			 0.f, -.5f,
			 .5f, -.5f,
			-.5f,  0.f,
			 0.f,  0.f,
			 .5f,  0.f,
			-.5f,  .5f,
			 0.f,  .5f,
			 .5f,  .5f,
		};

		const float Samples5[] =
		{
			0.f, 0.f,
			-.5f, -.5f,
			.5f, -.5f,
			.5f,  .5f,
			-.5f,  .5f,
		};

		const float Samples4[] =
		{
			-.5f, -.5f,
			 .5f, -.5f,
			 .5f,  .5f,
			-.5f,  .5f,
		};

		const float Samples2[] =
		{
			-.5f, -.5f,
			 .5f,  .5f,
		};

		const float Samples1[] =
		{
			0.f, 0.f,
		};

		const float* Samples[] = { Samples1, Samples2, Samples4, Samples5, Samples9, Samples16 };

		pShader->SetParameter("RandX", Samples[TSSAA][(FrameNum%TSSAA_Samples[TSSAA]) * 2 + 0] / (float)RenderTargetSize[0]);
		pShader->SetParameter("RandY", Samples[TSSAA][(FrameNum%TSSAA_Samples[TSSAA]) * 2 + 1] / (float)RenderTargetSize[1]);
	}
	else
	{
		pShader->SetParameter("RandX", 0.f);
		pShader->SetParameter("RandY", 0.f);
	}
}

void GaussBlur(FrameBufferTexture* Src, int Level, FrameBuffer* Dest, int NumPasses, int NumSamples, float Sigma, float Factor, bool bBlendOutput, float ColorThreshold)
{
	GLuint w, h;

	BlurBuffers[1].GetViewportSize(&w, &h);

	glDisable(GL_BLEND);

	GaussBlurShader.Use(0);

	GaussBlurShader.SetParameter("Sigma", Sigma);
	GaussBlurShader.SetParameter("NumSamples", NumSamples);
	GaussBlurShader.SetParameter("TexLevel", Level);
	GaussBlurShader.SetParameter("TexSize", glm::vec2(w, h));
	GaussBlurShader.SetParameter("Factor", 1.f);
	GaussBlurShader.SetParameter("ColorThreshold", ColorThreshold);

	for (int i = 0; i < NumPasses; i++)
	{
		Src->Bind(0);
		BlurBuffers[1].Bind();

		GaussBlurShader.SetParameter("KernelDir", glm::vec2(0.0, 1.0));
		Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

		GaussBlurShader.SetParameter("TexLevel", 0);
		GaussBlurShader.SetParameter("ColorThreshold", 0.f);

		BlurBufferColor[1].Bind(0);

		if (i < NumPasses - 1)
		{
			BlurBuffers[2].Bind();
		}
		else
		{
			if (bBlendOutput)
				glEnable(GL_BLEND);

			GaussBlurShader.SetParameter("Factor", Factor);
			Dest->Bind();
		}

		GaussBlurShader.SetParameter("KernelDir", glm::vec2(1.0, 0.0));
		Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

		Src = &BlurBufferColor[2];
	}
}

void RenderScene()
{
	glm::vec3 lDir = glm::normalize(glm::vec3(-LightDir.x, LightDir.y, LightDir.z));

	CurrentScene.SetLightDir(lDir);
	ShadowmapCamera.bPerspectiveProjection = false;
	ShadowmapCamera.SetOverrideViewportSize(&glm::vec2(512));
	ShadowmapCamera.Pos = MainCamera.Pos - lDir * 100.f;
	ShadowmapCamera.SetLookAtTarget(&(ShadowmapCamera.Pos + glm::normalize(glm::vec3(LightDir.x, -LightDir.y, LightDir.z))));

	glDepthMask(true);
	glColorMask(true, true, true, true);
	glClearColor(0.f, 0.f, 0.f, 0.f);

	FrameBuffer::BindBackBuffer(BackBufferSize[0], BackBufferSize[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw the scene
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//shadowmap
	ShadowmapBuffer.Bind();
	CurrentScene.SetCamera(&ShadowmapCamera);
	CurrentScene.UpdateTransforms();

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	CurrentScene.Render(&DepthOnlyShader, 0, [](Shader* pShader, int32 Flags) { SetSceneParemeters(pShader, Flags); pShader->SetParameter("ViewProjectionMatrix", ShadowmapCamera.ViewProjectionMatrix); });

	//return;

	CurrentScene.SetCamera(&MainCamera);
	CurrentScene.UpdateTransforms();

	///////////////////////////
	//Render GBuffer
	glEnable(GL_MULTISAMPLE);

#if PER_SAMPLE_SHADING
	if (NUM_AA_SAMPLES > 1)
	{
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(NUM_AA_SAMPLES);
	}
#endif
	GBuffer.Bind();
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GBufferOnlyLinearDepth.Bind();
	glClearColor(1.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	GBuffer.Bind();

	glPolygonMode(GL_FRONT_AND_BACK, PolygonMode);

	glDepthMask(true);
	glColorMask(true, true, true, true);
	CurrentScene.Render(&DeferredShader, 0, SetSceneParemeters);

	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	///////////////////////////
	//Render Lights
	GBuffer_RT0.Bind(0);
	GBuffer_RT1.Bind(1);
	GBuffer_RT2.Bind(2);
	GBuffer_RT3.Bind(6);

	if (bPathTracing)
	{
		SkyBox2D[iSkyBox].Bind(3);
		PostProcessBufferLinearDepthOld.Bind(4);
		PostProcessBufferVelocity.Bind(5);

		int PATH_TRACE_SCALE = bHalfRes ? 2 : 1;

		AuxBufferColor.BindImage(0, true, PATH_TRACE_SCALE-1);

		PathTraceShader.Use((NumBounces - 1) | (bSeparatedSpecular ? 8 : 0));
		CurrentScene.SetRaytraceParameters(PathTraceShader);
		PathTraceShader.SetParameter("OutputBuffer", 0);
		PathTraceShader.SetParameter("iFrame", FrameNum);
		PathTraceShader.SetParameter("SkyBox2D", 3);
		PathTraceShader.SetParameter("CubemapRotation", CubemapRotation);
		PathTraceShader.SetParameter("SkyBoxIntensity", SkyBoxIntensity);		

		PathTraceShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / PATH_TRACE_SCALE, RenderTargetSize[1] / PATH_TRACE_SCALE));
		PathTraceShader.SetParameter("Scale", PATH_TRACE_SCALE);
		PathTraceShader.SetParameter("Offset", PATH_TRACE_SCALE == 1 ? ivec2(0) : ivec2(FrameNum & 1, (FrameNum / 2) & 1));
		PathTraceShader.SetParameter("SamplesPerPixel", SPP);

		PathTraceShader.SetParameter("RoughnessBias", RoughnessBias);
		PathTraceShader.SetParameter("MetallicBias", MetallicBias);

		PathTraceShader.Dispatch(RenderTargetSize[0] / PATH_TRACE_SCALE, RenderTargetSize[1] / PATH_TRACE_SCALE, 1, (NumBounces - 1) | (bSeparatedSpecular ? 8 : 0));
		glTextureBarrierNV();
		
		LightBuffer.Bind();

		static bool useDenoiser = true;
		if (useDenoiser)
		{
			GBuffer_RT0.GenerateMipmaps();
			GBuffer_RT1.GenerateMipmaps();

			DenoiseShader.Use(0);
			DenoiseShader.SetParameter("InputTex", 0);
			DenoiseShader.SetParameter("InputAlbedo", 1);
			DenoiseShader.SetParameter("InputNormal", 2);
			DenoiseShader.SetParameter("InputTexLevel", PATH_TRACE_SCALE - 1);
			DenoiseShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / PATH_TRACE_SCALE, RenderTargetSize[1] / PATH_TRACE_SCALE));
			AuxBufferColor.Bind(0);
			GBuffer_RT1.Bind(1);
			GBuffer_RT0.Bind(2);
			glDisable(GL_BLEND);
			Scene::DrawFullscreenQuad(nullptr, 0, nullptr);
		}
		else
		{
			glDisable(GL_BLEND);
			SimpleTexShader.Use(0);
			SimpleTexShader.SetParameter("InputTexLevel", 0);
			SimpleTexShader.SetParameter("InputTex", 0);

			GBuffer_RT1.Bind(0);
			Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);

			SimpleTexShader.Use(0);
			SimpleTexShader.SetParameter("InputTex", 0);
			SimpleTexShader.SetParameter("InputTexLevel", PATH_TRACE_SCALE - 1);
			AuxBufferColor.Bind(0);
			Scene::DrawFullscreenQuad(nullptr, 0, nullptr);
			SimpleTexShader.SetParameter("InputTexLevel", 0);
		}

		glDisable(GL_BLEND);
	}
	else
	{
		int RAY_SHADOWS_SCALE = bHalfRes ? 2 : 1;

		if (bRaytracing)
		{
			AuxBufferColor.BindImage(0, true, RAY_SHADOWS_SCALE - 1);

			RaytracedShadowsShader.Use(0);
			CurrentScene.SetRaytraceParameters(RaytracedShadowsShader);
			RaytracedShadowsShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / RAY_SHADOWS_SCALE, RenderTargetSize[1] / RAY_SHADOWS_SCALE));
			RaytracedShadowsShader.SetParameter("OutputBuffer", 0);
			RaytracedShadowsShader.SetParameter("Offset", RAY_SHADOWS_SCALE == 1 ? ivec2(0) : ivec2(FrameNum & 1, (FrameNum / 2) & 1));
			RaytracedShadowsShader.SetParameter("FrameNum", FrameNum);
			RaytracedShadowsShader.SetParameter("RayTracedShadowsScale", RAY_SHADOWS_SCALE);			

			RaytracedShadowsShader.Dispatch(RenderTargetSize[0] / RAY_SHADOWS_SCALE, RenderTargetSize[1] / RAY_SHADOWS_SCALE, 1, 0);

			glTextureBarrierNV();
		}

		//lighting
		{
			LightBuffer.Bind();

			ShadowmapBufferColor.Bind(4);
			AuxBufferColor.Bind(5);

			Scene::DrawFullscreenQuad(&LightShader, bRaytracing, [RAY_SHADOWS_SCALE](Shader* pShader, uint32_t Flags)
			{
				SetSceneParemeters(pShader, Flags);

				pShader->SetParameter("ShadowmapMatrix", ShadowmapCamera.ViewProjectionMatrix);

				pShader->SetParameter("Shadowmap", 4);
				pShader->SetParameter("RaytracedShadows", 5);
				pShader->SetParameter("RayTracedShadowsLevel", RAY_SHADOWS_SCALE - 1);
				//pShader->SetParameter("RaytracedShadowsLevel", bHalfRes ? 1.f : 0.f);
			});
		}
	}

	///////////////////////////
	//Resolve
#if PER_SAMPLE_SHADING
	glDisable(GL_SAMPLE_SHADING);
#endif
	glDisable(GL_MULTISAMPLE);

	PostProcessBuffer.Bind();
	LightBufferTexture.Bind(0);
	GBuffer_RT0.Bind(1);
	GBuffer_RT1.Bind(2);
	GBuffer_RT2.Bind(3);
	GBuffer_RT3.Bind(4);
	Scene::DrawFullscreenQuad(&ResolveShader, 1, SetSceneParemeters);

	// Gen mipmaps
	PostProcessBufferColor.GenerateMipmaps();
	PostProcessBufferNormal.GenerateMipmaps();

	if (!bPathTracing)
	{
		//Reflections
		if (bShowReflections)
		{
			GBuffer_RT1.Bind(0);
			GBuffer_RT2.Bind(1);
			GBuffer_RT0.Bind(2);

			if (bRaytracing)
			{
				SkyBox2D[iSkyBox].Bind(3);

				//Raytraced
				AuxBufferColor.BindImage(0, true, 1);

				RaytracedReflectionsShader.Use(0);
				CurrentScene.SetRaytraceParameters(RaytracedReflectionsShader);
				RaytracedReflectionsShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / 2, RenderTargetSize[1] / 2));
				RaytracedReflectionsShader.SetParameter("OutputBuffer", 0);
				RaytracedReflectionsShader.SetParameter("Offset", ivec2(FrameNum & 1, (FrameNum / 2) & 1));
				RaytracedReflectionsShader.SetParameter("Scale", 2);
				RaytracedReflectionsShader.SetParameter("iFrame", FrameNum);
				RaytracedReflectionsShader.SetParameter("SkyBox2D", 3);
				RaytracedReflectionsShader.SetParameter("CubemapRotation", CubemapRotation);
				RaytracedReflectionsShader.SetParameter("SkyBoxIntensity", SkyBoxIntensity);

				RaytracedReflectionsShader.Dispatch(RenderTargetSize[0] / 2, RenderTargetSize[1] / 2, 1, 0);

				glTextureBarrierNV();

				PostProcessBufferOnlyColor.Bind();
				AuxBufferColor.Bind(0);

				SimpleTexShader.Use(0);
				SimpleTexShader.SetParameter("InputTex", 0);
				SimpleTexShader.SetParameter("InputTexLevel", 1);

				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);

				Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

				glDisable(GL_BLEND);

				SimpleTexShader.SetParameter("InputTexLevel", 0);
			}
			else
			{
				LightBufferTexture.GenerateMipmaps();
				LightBufferTexture.Bind(0);
				SkyBox2D[iSkyBox].Bind(4);
				AuxBuffer.Bind();

				ScreenSpaceReflectionsShader.Use(0);
				SetSceneParemeters(&ScreenSpaceReflectionsShader, 0);
				ScreenSpaceReflectionsShader.SetParameter("SkyBox2D", 4);
				ScreenSpaceReflectionsShader.SetParameter("CubemapRotation", CubemapRotation);

				//SSR
				Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

				//Final copy
				PostProcessBufferOnlyColor.Bind();
				AuxBuffer.BindRead();
				Scene::DrawFullscreenQuad(nullptr, 0, nullptr);
			}
		}

		//Ambient occlusion
		if (bShowSSAO)
		{
			GBuffer_RT1.Bind(0);
			GBuffer_RT2.Bind(1);
			GBuffer_RT0.Bind(2);

			if (bRaytracing)
			{
				SkyBox2D[iSkyBox].Bind(3);

				if (true)
				{
					AuxBufferColor.BindImage(0, true, 0);

					RaytracedAmbientOcclusionShader.Use(0);
					CurrentScene.SetRaytraceParameters(RaytracedAmbientOcclusionShader);
					RaytracedAmbientOcclusionShader.SetParameter("iResolution", vec2(RenderTargetSize[0], RenderTargetSize[1]));
					RaytracedAmbientOcclusionShader.SetParameter("OutputBuffer", 0);
					RaytracedAmbientOcclusionShader.SetParameter("Offset", ivec2(0));
					RaytracedAmbientOcclusionShader.SetParameter("Scale", 1);
					RaytracedAmbientOcclusionShader.SetParameter("iFrame", FrameNum);
					RaytracedAmbientOcclusionShader.SetParameter("SkyBox2D", 3);
					RaytracedAmbientOcclusionShader.SetParameter("CubemapRotation", CubemapRotation);

					RaytracedAmbientOcclusionShader.Dispatch(RenderTargetSize[0], RenderTargetSize[1], 1, 0);

					glTextureBarrierNV();

					PostProcessBufferOnlyColor.Bind();
					AuxBufferColor.Bind(0);

					SimpleTexShader.Use(0);
					SimpleTexShader.SetParameter("InputTex", 0);
					SimpleTexShader.SetParameter("InputTexLevel", 0);
					SimpleTexShader.SetParameter("iResolution", vec2(RenderTargetSize[0], RenderTargetSize[1]));

					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);

					Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

					glDisable(GL_BLEND);

					BilateralShader.SetParameter("InputTexLevel", 0);

				}
				else
				{
					AuxBufferColor.BindImage(0, true, 1);

					RaytracedAmbientOcclusionShader.Use(0);
					CurrentScene.SetRaytraceParameters(RaytracedAmbientOcclusionShader);
					RaytracedAmbientOcclusionShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / 2, RenderTargetSize[1] / 2));
					RaytracedAmbientOcclusionShader.SetParameter("OutputBuffer", 0);
					RaytracedAmbientOcclusionShader.SetParameter("Offset", ivec2(FrameNum & 1, (FrameNum / 2) & 1));
					RaytracedAmbientOcclusionShader.SetParameter("Scale", 2);
					RaytracedAmbientOcclusionShader.SetParameter("iFrame", FrameNum);
					RaytracedAmbientOcclusionShader.SetParameter("SkyBox2D", 3);
					RaytracedAmbientOcclusionShader.SetParameter("CubemapRotation", CubemapRotation);

					RaytracedAmbientOcclusionShader.Dispatch(RenderTargetSize[0] / 2, RenderTargetSize[1] / 2, 1, 0);

					glTextureBarrierNV();

					PostProcessBufferOnlyColor.Bind();
					AuxBufferColor.Bind(0);

					BilateralShader.Use(0);
					BilateralShader.SetParameter("InputTex", 0);
					BilateralShader.SetParameter("InputTexLevel", 1);
					BilateralShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / 2, RenderTargetSize[1] / 2));

					glEnable(GL_BLEND);
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);

					Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

					glDisable(GL_BLEND);

					BilateralShader.SetParameter("InputTexLevel", 0);
				}
			}
		}

		//Indirect lighting
		if (NumBounces > 0)
		{
			NumBounces = std::min(3, std::max(0, NumBounces));
#define GI_SCALE 8

			GBuffer_RT2.GenerateMipmaps();

			GBuffer_RT0.Bind(0);
			GBuffer_RT1.Bind(1);
			GBuffer_RT2.Bind(2);
			ShadowmapBufferColor.Bind(4);

			if (bRaytracing)
			{
				AuxBufferColor.BindImage(0, true, 3);
				SkyBox2D[iSkyBox].Bind(6);

				RaytracedIndirectLightShader.Use(NumBounces - 1);
				CurrentScene.SetRaytraceParameters(RaytracedIndirectLightShader);
				RaytracedIndirectLightShader.SetParameter("iResolution", vec2(RenderTargetSize[0] / GI_SCALE, RenderTargetSize[1] / GI_SCALE));
				RaytracedIndirectLightShader.SetParameter("OutputBuffer", 0);
				RaytracedIndirectLightShader.SetParameter("Offset", ivec2(FrameNum & 1, (FrameNum / 2) & 1));
				RaytracedIndirectLightShader.SetParameter("Scale", GI_SCALE);
				RaytracedIndirectLightShader.SetParameter("iFrame", FrameNum);
				RaytracedIndirectLightShader.SetParameter("ShadowmapMatrix", ShadowmapCamera.ViewProjectionMatrix);
				RaytracedIndirectLightShader.SetParameter("SkyBox2D", 6);
				RaytracedIndirectLightShader.SetParameter("CubemapRotation", CubemapRotation);
				RaytracedIndirectLightShader.SetParameter("SkyBoxIntensity", SkyBoxIntensity);
				//RaytracedIndirectLightShader.SetParameter("Shadowmap", 4);

				RaytracedIndirectLightShader.Dispatch(RenderTargetSize[0] / GI_SCALE, RenderTargetSize[1] / GI_SCALE, 1, NumBounces - 1);

				glTextureBarrierNV();

				PostProcessBufferOnlyColor.Bind();
				AuxBufferColor.Bind(0);
				GBuffer_RT1.Bind(1);
				GBuffer_RT0.Bind(2);

				auto& ShaderToUse = IndirectBlendShader;

				ShaderToUse.Use(0);
				ShaderToUse.SetParameter("InputTex", 0);
				ShaderToUse.SetParameter("ModulateTex", 1);
				ShaderToUse.SetParameter("InputNormal", 2);
				ShaderToUse.SetParameter("InputTexLevel", 3);
				ShaderToUse.SetParameter("iResolution", vec2(RenderTargetSize[0] / GI_SCALE, RenderTargetSize[1] / GI_SCALE));

				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);

				Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

				glDisable(GL_BLEND);
			}
		}
	}

	/////////////////////////////
	//Motion Blur
	if (bShowMotionBlur)
	{
		PostProcessBufferColor.GenerateMipmaps();

		AuxBuffer.Bind();
		PostProcessBufferColor.Bind(0);
		PostProcessBufferVelocity.Bind(1);

		Scene::DrawFullscreenQuad(&MotionBlurShader, 0, SetSceneParemeters);

		//Copy to PostProcessBufferColor
		PostProcessBufferOnlyColor.Bind();
		AuxBuffer.BindRead();
		FrameBuffer::Blit(RenderTargetSize[0], RenderTargetSize[1]);
	}

	////////////////////////////
	//DOF
	if (bShowDOF)
	{
		DepthOfField_COC.Bind();
		PostProcessBufferLinearDepth.Bind(0);

		DepthOfFieldShader.Use(1);
		DepthOfFieldShader.SetParameter("DOF_Params", glm::vec4(0.08f, 0.1f, 0.3f, 0.9f));

		Scene::DrawFullscreenQuad(&DepthOfFieldShader, 1, SetSceneParemeters);

		GaussBlur(&PostProcessBufferColor, 2, &BlurBuffers[0], 3, 12, 5.0f, 1.0f, false, 0.0f);

		DepthOfField_COC_Color.GenerateMipmaps();
		GaussBlur(&DepthOfField_COC_Color, 0, &DepthOfField_COC_Blur, 3, 12, 6.0f, 1.0f, false, 0.0f);

		AuxBuffer.Bind();
		DepthOfField_COC_Color.Bind(0);
		DepthOfField_COC_BlurColor.Bind(1);
		PostProcessBufferColor.Bind(2);
		BlurBufferColor[0].Bind(3);

		DepthOfFieldShader.Use(0);
		Scene::DrawFullscreenQuad(&DepthOfFieldShader, 0, SetSceneParemeters);

		PostProcessBufferOnlyColor.Bind();
		AuxBuffer.BindRead();
		PostProcessBuffer.Blit(RenderTargetSize[0], RenderTargetSize[1]);
	}

	if (bShowAccumAA)
	{
		AuxBuffer.Bind();
		PostProcessBufferColor.Bind(0);
		PostProcessBufferColorOld.Bind(1);
		PostProcessBufferVelocity.Bind(2);

		PostProcessBufferLinearDepth.Bind(3);
		PostProcessBufferLinearDepthOld.Bind(4);

		static glm::vec3 PrevCamPos, PrevCamRot;
		static bool bLastHalfRes;
		static glm::quat LastCubemapRotQuat;

		if (glm::length(PrevCamPos - MainCamera.Pos) > 0.1f || glm::length(PrevCamRot - MainCamera.Rot) > 0.1f)
			AAFrame = 0;
		if (bLastHalfRes != bHalfRes)
			AAFrame = 0;
		if (CubemapRotQuat != LastCubemapRotQuat)
			AAFrame = 0;

		PrevCamPos = MainCamera.Pos;
		PrevCamRot = MainCamera.Rot;
		bLastHalfRes = bHalfRes;
		LastCubemapRotQuat = CubemapRotQuat;

		TSSAAShader.Use(0);
		TSSAAShader.SetParameter("AAFrame", ++AAFrame);

		Scene::DrawFullscreenQuad(&TSSAAShader, 0, SetSceneParemeters);

		PostProcessBufferOnlyDepth.BindRead();
		PostProcessBufferOldDepth.Bind();
		FrameBuffer::Blit(RenderTargetSize[0], RenderTargetSize[1]);

		AuxBuffer.BindRead();

		PostProcessBufferOld.Bind();
		FrameBuffer::Blit(RenderTargetSize[0], RenderTargetSize[1]);

		PostProcessBufferOnlyColor.Bind();
		FrameBuffer::Blit(RenderTargetSize[0], RenderTargetSize[1]);
	}
	else
	{
		PostProcessBufferOnlyColor.BindRead();
		PostProcessBufferOld.Bind();
		FrameBuffer::Blit(RenderTargetSize[0], RenderTargetSize[1]);
	}

	/////////////////////////////
	//sky box
	PostProcessBufferOnlyColor.Bind();
	SkyBox2D[iSkyBox].Bind(0);
	GBuffer_RT1.Bind(1);
	EnvMapShader.Use(0);
	EnvMapShader.SetParameter("SkyBox2D", 0);
	CubemapRotation = glm::mat4_cast(CubemapRotQuat);
	EnvMapShader.SetParameter("CubemapRotation", CubemapRotation);
	EnvMapShader.SetParameter("CameraPos", MainCamera.Pos);
	EnvMapShader.SetParameter("CameraToWorld[0]", CurrentScene.CameraToWorld[0], 4);
	EnvMapShader.SetParameter("SkyBoxIntensity", SkyBoxIntensity);

	Scene::DrawFullscreenQuad(nullptr, 0, nullptr);

	/////////////////////////////
	//Bloom
	if (bShowBloom)
	{
		PostProcessBufferColor.GenerateMipmaps();

		GaussBlur(&PostProcessBufferColor, 2, &BlurBuffers[0], 3, 12, 5.0f, 0.5f, false, 0.5f);
	}
	else
	{
		BlurBufferColor[0].Clear(glm::vec4(0.f, 0.f, 0.f, 0.f));
	}

	//Tonemap & gamma correction
	BackBuffer.Bind();
	PostProcessBufferColor.Bind(0);
	BlurBufferColor[0].Bind(1);
	Scene::DrawFullscreenQuad(&ToneMapperShader, (bShowToneMap ? 0 : 1), SetSceneParemeters);

	FrameBuffer::BindBackBuffer(BackBufferSize[0], BackBufferSize[1]);
	BackBufferColor.Bind(0);
	BackBufferColor.GenerateMipmaps();
	Scene::DrawFullscreenQuad(&SimpleTexShader, 0, SetSceneParemeters);

	/*
	SimpleTexShader.Use(1);
	ShadowmapBufferColor.GenerateMipmaps();
	ShadowmapBufferColor.Bind(0);

	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.f, 0.f);
		glVertex3f(0.25f, 0.25f, 0.f);

		glTexCoord2f(1.f, 0.f);
		glVertex3f(0.5f, 0.25f, 0.f);

		glTexCoord2f(1.f, 1.f);
		glVertex3f(0.5f, 0.5f, 0.f);

		glTexCoord2f(0.f, 1.f);
		glVertex3f(0.25f, 0.5f, 0.f);
	}
	glEnd();
	*/
	return;

	//raytrace
	BackBufferColor.BindImage(0, true, 0);

	RaytraceShader.Use(0);
	CurrentScene.SetRaytraceParameters(RaytraceShader);
	RaytraceShader.SetParameter("BackBuffer", 0);
	RaytraceShader.SetParameter("iResolution", vec2(BackBufferSize[0], BackBufferSize[1]));

	RaytraceShader.Dispatch(BackBufferSize[0]/2, BackBufferSize[1], 1, 0, nullptr);

	BackBuffer.BindRead(0);
	FrameBuffer::Blit(BackBufferSize[0]/2, BackBufferSize[1]);
}

void MainLoop()
{
	const double AppTime = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::high_resolution_clock::now() - AppStartTime).count();
	static double LastTime = AppTime;
	static float FrameDelta = 1.f;

	{
		glUseProgram(0);

		// Switch to window coordinates to render
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, BackBufferSize[0], BackBufferSize[1], 0, 0, 1000);

		glRasterPos2i(BackBufferSize[0]-100, 40);
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

		char FalseTrue[2][16] = { "FALSE","TRUE" };
		char str[512];

		sprintf_s(str, "FPS: %i\n", (int)(1.f / FrameDelta));
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*)str);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	if (bPathTracing)
	{
		NumBounces = std::max(1, NumBounces);
		bShowReflections = true;
		bShowSSAO = true;
	}
	else if (!bRaytracing)
	{
		NumBounces = 0;
		//bShowReflections = false;
		bShowSSAO = false;
	}

	TwDraw();
	glutSwapBuffers();

	FrameDelta = (float)((AppTime - LastTime) * 0.001f);
	LastTime = AppTime;

	Sleep(0);

	if(FrameNum > 0)
		RenderScene();
	FrameNum++;

	CurrentScene.UpdateRaytraceScene();

	DWORD ProcID = GetProcessId(GetModuleHandle(NULL));
	bool bCaptureInput = GetWindowThreadProcessId(GetActiveWindow(), &ProcID) == GetCurrentThreadId();

	MainCamera.Update(FrameDelta, bCaptureInput);
	ShadowmapCamera.Update(FrameDelta, false);

	bool bScreenPctDirty = false;

	extern void CompileShaders();
	extern void ReshapeFunc(int Width, int Height);

	if (bCaptureInput)
	{
		//{
		//	POINT MousePos;
		//	GetCursorPos(&MousePos);
		//
		//	static int PrevMouseX = MousePos.x, PrevMouseY = MousePos.y;
		//
		//	if (GetAsyncKeyState(VK_LBUTTON) && bCaptureInput)
		//	{
		//		CubemapRotationX = (MousePos.x - PrevMouseX) * FrameDelta * 1.f;
		//		CubemapRotationY = (MousePos.y - PrevMouseY) * FrameDelta * 1.f;
		//
		//		static const glm::mat4 IdentityMatrix4(1.f, 0.f, 0.f, 0.f,
		//			0.f, 1.f, 0.f, 0.f,
		//			0.f, 0.f, 1.f, 0.f,
		//			0.f, 0.f, 0.f, 1.f);
		//
		//		CubemapRotation = glm::rotate(CubemapRotation, glm::radians(CubemapRotationX), glm::vec3(glm::vec4(0.f, 1.f, 0.f, 0.f) * MainCamera.ViewMatrix));
		//		CubemapRotation = glm::rotate(CubemapRotation, glm::radians(CubemapRotationY), glm::vec3(glm::vec4(1.f, 0.f, 0.f, 0.f) * MainCamera.ViewMatrix));
		//
		//		SetCursorPos(PrevMouseX, PrevMouseY);
		//		GetCursorPos(&MousePos);
		//	}
		//
		//	PrevMouseX = MousePos.x;
		//	PrevMouseY = MousePos.y;
		//}


		if (GetAsyncKeyState('R'))
		{
			CompileShaders();
			AAFrame = 0;
		}
	}

	if (ScreenPct != NewScreenPct && bScreenPctDirty == false)
	{
		ScreenPct = NewScreenPct;
		ReshapeFunc(BackBufferSize[0], BackBufferSize[1]);
	}
}


void Display()
{
}


void ReshapeFunc(int Width, int Height)
{
	RECT r;
	GetClientRect(GetActiveWindow(), &r);
	TwWindowSize(r.right - r.left, r.bottom - r.top);

	BackBufferSize[0] = (Width + 7) & ~7;
	BackBufferSize[1] = (Height + 7) & ~7;

	Width = (int)(Width * ScreenPct);
	Height = (int)(Height * ScreenPct);

	RenderTargetSize[0] = (Width + 7) & ~7;
	RenderTargetSize[1] = (Height + 7) & ~7;

	//GBuffer
	{
		//Normal.xyz, metallic
		GBuffer_RT0.SetSize(Width, Height, NUM_AA_SAMPLES);
		GBuffer_RT0.SetFormat(GL_RGBA, GL_RGB10_A2);

		//Albedo.xyz, roughness
		GBuffer_RT1.SetSize(Width, Height, NUM_AA_SAMPLES);
		GBuffer_RT1.SetFormat(GL_RGBA, GL_RGBA8);

		//Linear depth
		GBuffer_RT2.SetSize(Width, Height, NUM_AA_SAMPLES);
		GBuffer_RT2.SetFormat(GL_R, GL_R32F);

		//Velocity, Emissive
		GBuffer_RT3.SetSize(Width, Height, NUM_AA_SAMPLES);
		GBuffer_RT3.SetFormat(GL_RG, GL_RGBA16F);

		GBuffer_DST.SetSize(Width, Height, NUM_AA_SAMPLES);
		GBuffer_DST.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32);

		GBuffer.SetBuffer(FrameBuffer::Type::COLOR0, &GBuffer_RT0);
		GBuffer.SetBuffer(FrameBuffer::Type::COLOR1, &GBuffer_RT1);
		GBuffer.SetBuffer(FrameBuffer::Type::COLOR2, &GBuffer_RT2);
		GBuffer.SetBuffer(FrameBuffer::Type::COLOR3, &GBuffer_RT3);
		GBuffer.SetBuffer(FrameBuffer::Type::DEPTH, &GBuffer_DST);

		GBufferOnlyLinearDepth.SetBuffer(FrameBuffer::Type::COLOR0, &GBuffer_RT2);
	}

	//Light Accumulation Buffer
	{
		LightBufferTexture.SetSize(Width, Height, NUM_AA_SAMPLES);
		LightBufferTexture.SetFormat(GL_RGBA, GL_RGBA16F);

		LightBuffer.SetBuffer(FrameBuffer::Type::COLOR0, &LightBufferTexture);
		LightBuffer.SetBuffer(FrameBuffer::Type::DEPTH, &GBuffer_DST);
	}

	//Shadowmap Buffer
	{
		ShadowmapBufferColor.SetSize(2048, 2048, 1);
		ShadowmapBufferColor.SetFormat(GL_R, GL_R32F);
	
		ShadowmapBufferTexture.SetSize(2048, 2048, 1);
		ShadowmapBufferTexture.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24);
	
		ShadowmapBuffer.SetBuffer(FrameBuffer::Type::COLOR0, &ShadowmapBufferColor);
		ShadowmapBuffer.SetBuffer(FrameBuffer::Type::DEPTH, &ShadowmapBufferTexture);
	}


	//Post Process Buffer
	{
		PostProcessBufferNormal.SetSize(Width, Height, 1);
		PostProcessBufferNormal.SetFormat(GL_RGBA, GL_RGB10_A2);

		PostProcessBufferColor.SetSize(Width, Height, 1);
		PostProcessBufferColor.SetFormat(GL_RGBA, GL_RGBA16F);

		PostProcessBufferLinearDepth.SetSize(Width, Height, 1);
		PostProcessBufferLinearDepth.SetFormat(GL_R, GL_R32F);

		PostProcessBufferVelocity.SetSize(Width, Height, 1);
		PostProcessBufferVelocity.SetFormat(GL_RG, GL_RGBA16F);

		PostProcessBufferDepth.SetSize(Width, Height, 1);
		PostProcessBufferDepth.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH24_STENCIL8);

		PostProcessBuffer.SetBuffer(FrameBuffer::Type::COLOR0, &PostProcessBufferNormal);
		PostProcessBuffer.SetBuffer(FrameBuffer::Type::COLOR1, &PostProcessBufferColor);
		PostProcessBuffer.SetBuffer(FrameBuffer::Type::COLOR2, &PostProcessBufferLinearDepth);
		PostProcessBuffer.SetBuffer(FrameBuffer::Type::COLOR3, &PostProcessBufferVelocity);
		PostProcessBuffer.SetBuffer(FrameBuffer::Type::DEPTH, &PostProcessBufferDepth);

		PostProcessBufferOnlyColor.SetBuffer(FrameBuffer::Type::COLOR0, &PostProcessBufferColor);
		PostProcessBufferOnlyColor.SetBuffer(FrameBuffer::Type::DEPTH, &PostProcessBufferDepth);

		PostProcessBufferOnlyDepth.SetBuffer(FrameBuffer::Type::COLOR0, &PostProcessBufferLinearDepth);
	}

	{
		PostProcessBufferColorOld.SetSize(Width, Height, 1);
		PostProcessBufferColorOld.SetFormat(GL_RGBA, GL_RGBA16F);

		PostProcessBufferOld.SetBuffer(FrameBuffer::Type::COLOR0, &PostProcessBufferColorOld);

		PostProcessBufferLinearDepthOld.SetSize(Width, Height, 1);
		PostProcessBufferLinearDepthOld.SetFormat(GL_R, GL_R32F);

		PostProcessBufferOldDepth.SetBuffer(FrameBuffer::Type::COLOR0, &PostProcessBufferLinearDepthOld);
	}

	//Aux buffer Buffer
	{
		AuxBufferColor.SetSize(Width, Height, 1);
		AuxBufferColor.SetFormat(GL_RGBA, GL_RGBA16F);

		AuxBufferDepth.SetSize(Width, Height, 1);
		AuxBufferDepth.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH24_STENCIL8);

		AuxBuffer.SetBuffer(FrameBuffer::Type::COLOR0, &AuxBufferColor);
		AuxBuffer.SetBuffer(FrameBuffer::Type::DEPTH, &AuxBufferDepth);
	}

	//Blur Buffer
	{
		BlurBufferDepth.SetSize(Width / 4, Height / 4, 1);
		BlurBufferDepth.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH24_STENCIL8);

		for (int i = 0; i < sizeof(BlurBufferColor) / sizeof(BlurBufferColor[0]); i++)
		{
			BlurBufferColor[i].SetSize(Width / 4, Height / 4, 1);
			BlurBufferColor[i].SetFormat(GL_RGBA, GL_RGBA16F);

			BlurBuffers[i].SetBuffer(FrameBuffer::Type::COLOR0, &BlurBufferColor[i]);
			BlurBuffers[i].SetBuffer(FrameBuffer::Type::DEPTH, &BlurBufferDepth);
		}
	}

	//COC
	{
		DepthOfField_COC_Depth.SetSize(Width, Height, 1);
		DepthOfField_COC_Depth.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH24_STENCIL8);

		DepthOfField_COC_Color.SetSize(Width, Height, 1);
		DepthOfField_COC_Color.SetFormat(GL_RGBA, GL_RG16F);

		DepthOfField_COC.SetBuffer(FrameBuffer::Type::COLOR0, &DepthOfField_COC_Color);
		DepthOfField_COC.SetBuffer(FrameBuffer::Type::DEPTH, &DepthOfField_COC_Depth);

		DepthOfField_COC_BlurDepth.SetSize(Width / 4, Height / 4, 1);
		DepthOfField_COC_BlurDepth.SetFormat(GL_DEPTH_COMPONENT, GL_DEPTH24_STENCIL8);

		DepthOfField_COC_BlurColor.SetSize(Width / 4, Height / 4, 1);
		DepthOfField_COC_BlurColor.SetFormat(GL_RGBA, GL_RG16F);

		DepthOfField_COC_Blur.SetBuffer(FrameBuffer::Type::COLOR0, &DepthOfField_COC_BlurColor);
		DepthOfField_COC_Blur.SetBuffer(FrameBuffer::Type::DEPTH, &DepthOfField_COC_BlurDepth);
	}

	//Back Buffer
	{
		BackBufferColor.SetSize(Width, Height, 1);
		BackBufferColor.SetFormat(GL_RGBA, GL_RGBA8);

		BackBuffer.SetBuffer(FrameBuffer::Type::COLOR0, &BackBufferColor);
	}
}

void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	printf(message);
	printf("\n");
}

void CompileShaders()
{
	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "Raytrace", RaytraceShader);
	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "raytraced_reflections", RaytracedReflectionsShader);
	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "raytraced_light", RaytracedLightShader);
	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "raytraced_shadows", RaytracedShadowsShader, 31);
	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "raytraced_ambientocclusion", RaytracedAmbientOcclusionShader, 31);

	RaytracedIndirectLightShader.ClearFlags();
	RaytracedIndirectLightShader.AddFlag(0, "NUM_BOUNCES 1");
	RaytracedIndirectLightShader.AddFlag(1, "NUM_BOUNCES 2");
	RaytracedIndirectLightShader.AddFlag(2, "NUM_BOUNCES 3");
	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "raytraced_indirectlight", RaytracedIndirectLightShader);

	PathTraceShader.ClearFlags();
	PathTraceShader.AddFlag(0, "PATH_TRACING");
	PathTraceShader.AddFlag(0, "NUM_BOUNCES 1");
	PathTraceShader.AddFlag(1, "PATH_TRACING");
	PathTraceShader.AddFlag(1, "NUM_BOUNCES 2");
	PathTraceShader.AddFlag(2, "PATH_TRACING");
	PathTraceShader.AddFlag(2, "NUM_BOUNCES 3");
	PathTraceShader.AddFlag(0 | 8, "SEPARATED_SPECULAR");
	PathTraceShader.AddFlag(0 | 8, "PATH_TRACING");
	PathTraceShader.AddFlag(0 | 8, "NUM_BOUNCES 1");
	PathTraceShader.AddFlag(1 | 8, "SEPARATED_SPECULAR");
	PathTraceShader.AddFlag(1 | 8, "PATH_TRACING");
	PathTraceShader.AddFlag(1 | 8, "NUM_BOUNCES 2");
	PathTraceShader.AddFlag(2 | 8, "SEPARATED_SPECULAR");
	PathTraceShader.AddFlag(2 | 8, "PATH_TRACING");
	PathTraceShader.AddFlag(2 | 8, "NUM_BOUNCES 3");

	CompileRaytraceShader(GetResourcesPath() + "Payload", GetResourcesPath() + "pathtracing", PathTraceShader, RAYTRACE_OPTIMIZATIONS_FLAG_NO_FACE_CULLING| RAYTRACE_OPTIMIZATIONS_FLAG_NO_ANY_HIT);


	DeferredShader.CompileFiles(GetResourcesPath() + "Deferred");

	EnvMapShader.CompileFiles(GetResourcesPath() + "EnvMap");

	LightShader.ClearFlags();
	LightShader.AddFlag(0, NUM_AA_SAMPLES_STRING());
	LightShader.AddFlag(1, NUM_AA_SAMPLES_STRING());
	LightShader.AddFlag(1, "RAYTRACED_SHADOWS");

	//LightShader.AddFlag(1, NUM_AA_SAMPLES_STRING());
	//LightShader.AddFlag(1, "OUTPUT_NORMAL 1");
	//
	//LightShader.AddFlag(2, NUM_AA_SAMPLES_STRING());
	//LightShader.AddFlag(2, "OUTPUT_DEPTH 1");

#if PER_SAMPLE_SHADING
	LightShader.AddFlag(0, "PER_SAMPLE_SHADING");
	LightShader.AddFlag(1, "PER_SAMPLE_SHADING");
#endif

	LightShader.CompileFiles(GetResourcesPath() + "Light");

	ForwardToCubeShader.CompileFiles(GetResourcesPath() + "ForwardToCube");

	ForwardShader.ClearFlags();
	ForwardShader.AddFlag(1, "SHADOW_MAP 1");
	ForwardShader.CompileFiles(GetResourcesPath() + "Forward");

	DepthOnlyShader.CompileFiles(GetResourcesPath() + "DepthOnly");

	ResolveShader.ClearFlags();
	ResolveShader.AddFlag(0, NUM_AA_SAMPLES_STRING());
	ResolveShader.AddFlag(1, NUM_AA_SAMPLES_STRING());
	ResolveShader.AddFlag(1, "RESOLVE_TEXTURE_2");
	ResolveShader.CompileFiles(GetResourcesPath() + "Resolve");

	//Tone mapper doesn't needs to resolve msaa
	ToneMapperShader.ClearFlags();
	ToneMapperShader.AddFlag(0, "NUM_AA_SAMPLES 1");
	ToneMapperShader.AddFlag(1, "NUM_AA_SAMPLES 1");
	ToneMapperShader.AddFlag(1, "NO_TONEMAP");
	ToneMapperShader.CompileFiles(GetResourcesPath() + "ToneMapper");

	//Tone mapper doesn't needs to resolve msaa
	TSSAAShader.CompileFiles(GetResourcesPath() + "TSSAA");

	DenoiseShader.CompileFiles(GetResourcesPath() + "denoise");

	SimpleTexShader.AddFlag(1, "CUSTOM_TEXCOORDS");
	SimpleTexShader.AddFlag(2, "CUBE_MAP");	
	SimpleTexShader.CompileFiles(GetResourcesPath() + "SimpleTex");

	GaussBlurShader.CompileFiles(GetResourcesPath() + "GaussBlur");

	MotionBlurShader.CompileFiles(GetResourcesPath() + "MotionBlur");

	ScreenSpaceReflectionsShader.CompileFiles(GetResourcesPath() + "ScreenSpaceReflections");

	BilateralShader.ClearFlags();
	BilateralShader.AddFlag(1, "MODULATE_TEXTURE");
	BilateralShader.AddFlag(2, "MULTITEX");
	BilateralShader.CompileFiles(GetResourcesPath() + "bilateral");

	IndirectBlendShader.CompileFiles(GetResourcesPath() + "indirectblend");	

	DepthOfFieldShader.ClearFlags();
	DepthOfFieldShader.AddFlag(1, "OUTPUT_COC 1");
	DepthOfFieldShader.CompileFiles(GetResourcesPath() + "DepthOfField");
}

#pragma optimize("", off)
void CreateScene()
{
	Texture::Load(GetResourcesPath() + "green_point_park_1k.hdr", SkyBox2D[0]);
	Texture::Load(GetResourcesPath() + "moonlit_golf_1k.hdr", SkyBox2D[1]);
	Texture::Load(GetResourcesPath() + "cape_hill_1k.hdr", SkyBox2D[2]);
	Texture::Load(GetResourcesPath() + "noon_grass_1k.hdr", SkyBox2D[3]);

#if SCENE_FROM_OBJ
	TwAddSeparator(TweakBar, "Materials", "");
		
	MainCamera.Pos = 0.f * vec3(-13, 27, -22);

	std::string meshFileName = "picapica2";
	float meshScale = 1.f;
	Mesh::LoadMesh(GetResourcesPath() + meshFileName + "/" + meshFileName + ".gltf", SceneMesh, meshScale);

	SceneObject* Obj;

	for (size_t i = 0; i < SceneMesh.GetNumShapes(); i++)
	{
		Obj = CurrentScene.AddObject("");
		Obj->SetMesh(SceneObject::EMeshType::Mesh, &SceneMesh, i);
		Obj->SetPosition(glm::vec3(0.f));
		Obj->SetScale(glm::vec3(1.f));
		Obj->SetRotation(glm::vec3(0.f));
		Obj->SetMaterialColor(SceneMesh.GetMaterialDiffuse(i));
		Obj->SetMaterialProperties(glm::vec4(SceneMesh.GetMaterialMetalness(i), SceneMesh.GetMaterialRoughness(i), SceneMesh.GetMaterialEmissive(i), 0.f));

		TwAddVarRW(TweakBar, (std::string("Object ") + std::to_string(i) + " Emissive").c_str(), TW_TYPE_FLOAT, &Obj->MaterialProperties.z, "min=0");

		auto LoadTexture = [meshFileName](const std::string& TexName)
		{
			Texture* pTexture = nullptr;
			std::string fileName = GetResourcesPath() + meshFileName + "/" + TexName;
			auto ExistingTexture = std::find_if(SceneTextures.begin(), SceneTextures.end(), [fileName](const std::unique_ptr<Texture>& A) { return A->GetFileName() == fileName; });
			if (ExistingTexture == SceneTextures.end())
			{
				std::unique_ptr<Texture> Tex = std::make_unique<Texture>();
				pTexture = Tex.get();
				Texture::Load(fileName, *pTexture);
				SceneTextures.push_back(std::move(Tex));
			}
			else
			{
				pTexture = ExistingTexture->get();
			}

			return pTexture;
		};

		std::string Albedo = SceneMesh.GetMaterialAlbedo(i);
		if (!Albedo.empty())
		{
			if (auto AlbedoTexture = LoadTexture(Albedo))
				Obj->SetAlbedoTexture(AlbedoTexture);
		}

		std::string Normal = SceneMesh.GetMaterialNormalMap(i);
		if (!Normal.empty())
		{
			size_t it = Normal.find("bump");
			if (it != std::string::npos)
			{
				Normal.replace(it, 4, "normal");

				if (auto NormalTexture = LoadTexture(Normal))
					Obj->SetNormalMap(NormalTexture);
			}
		}
	}

#else
	Mesh::LoadMesh(GetResourcesPath() + "Lamborghini.obj", CarMesh, 1.f, true, true);
	Texture::Load(GetResourcesPath() + "Lamborginhi_diffuse.jpeg", CarTexture);

	SceneObject* Obj;

	//opaque scene
	{
		//floor
		Obj = CurrentScene.AddObject("Floor");
		Obj->SetMesh(SceneObject::EMeshType::Quad);
		Obj->SetPosition(glm::vec3(0.f, -30.f, 90.f));
		Obj->SetScale(glm::vec3(120.f, 120.f, 120.f));
		Obj->SetRotation(glm::vec3(0.f, 0.f, 0.f));
		Obj->SetMaterialColor(glm::vec4(.75f, .75f, .75f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(0.f, 0.8f, 0.f, 0.f));

		//left wall
		Obj = CurrentScene.AddObject("LeftWall");
		Obj->SetMesh(SceneObject::EMeshType::Quad);
		Obj->SetPosition(glm::vec3(-120.f, 30.f, 90.f));
		Obj->SetScale(glm::vec3(60.f, 120.f, 120.f));
		Obj->SetRotation(glm::vec3(0.f, 0.f, -90.f));
		Obj->SetMaterialColor(glm::vec4(1.f, .5f, .5f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(0.0f, 0.8f, 0.f, 0.f));

		//right wall
		Obj = CurrentScene.AddObject("RightWall");
		Obj->SetMesh(SceneObject::EMeshType::Quad);
		Obj->SetPosition(glm::vec3(120.f, 30.f, 90.f));
		Obj->SetScale(glm::vec3(60.f, 120.f, 120.f));
		Obj->SetRotation(glm::vec3(0.f, 0.f, 90.f));
		Obj->SetMaterialColor(glm::vec4(.5f, 1.f, .5f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(0.0f, 0.8f, 0.f, 0.f));

		//back wall
		Obj = CurrentScene.AddObject("BackWall");
		Obj->SetMesh(SceneObject::EMeshType::Quad);
		Obj->SetPosition(glm::vec3(0.f, 30.f, 200.f));
		Obj->SetScale(glm::vec3(120.f, 120.f, 60.f));
		Obj->SetRotation(glm::vec3(0.f, 270.f, 0.f));
		//Obj->SetRotation(glm::vec3(0, 135.f, 90.f));
		Obj->SetMaterialColor(glm::vec4(.5f, .5f, 1.f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(0.0f, 0.8f, 0.f, 0.f));

		//Torus
		Obj = CurrentScene.AddObject("Torus");
		Obj->SetMesh(SceneObject::EMeshType::Torus);
		Obj->SetPosition(glm::vec3(-68.f, 0.f, 100.f));
		Obj->SetScale(glm::vec3(24.f));
		Obj->SetMaterialColor(glm::vec4(.25f, .25f, .75f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(1.0f, 0.3f, 0.f, 0.f));
		pTorus = Obj;

		//Tethra
		Obj = CurrentScene.AddObject("Tethra");
		Obj->SetMesh(SceneObject::EMeshType::Tetrahedron);
		Obj->SetPosition(glm::vec3(52.f, 0.f, 60.f));
		Obj->SetScale(glm::vec3(8.f, 8.f, 8.f));
		Obj->SetMaterialColor(glm::vec4(0.75f, .75f, .75f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(1.0f, 0.3f, 0.f, 0.f));
		pTethra = Obj;

		//Teapot
		Obj = CurrentScene.AddObject("Teapot");
		Obj->SetMesh(SceneObject::EMeshType::Teapot);
		Obj->SetPosition(glm::vec3(0.f, -32.f, 110.f));
		Obj->SetScale(glm::vec3(8.f, 8.f, 8.f));
		Obj->SetMaterialColor(glm::vec4(0.7f, 0.7f, .05f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(1.0f, 0.3f, 0.f, 0.f));

		//Cylinder
		Obj = CurrentScene.AddObject("Cone");
		Obj->SetMesh(SceneObject::EMeshType::Cone);
		Obj->SetPosition(glm::vec3(80.f, -16.f, 110.f));
		Obj->SetScale(glm::vec3(8.f, 16.f, 8.f));
		Obj->SetMaterialColor(glm::vec4(0.7f, 0.7f, .05f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(1.0f, 0.3f, 0.f, 0.f));

		/// Add the center car
		Obj = CurrentScene.AddObject("Car");
		Obj->SetMesh(SceneObject::EMeshType::Mesh, &CarMesh);
		Obj->SetPosition(glm::vec3(0.f, -30.f, 30.f));
		Obj->SetScale(glm::vec3(0.25f));
		Obj->SetRotation(glm::vec3(90.f, 0.f, 0.f));
		Obj->SetMaterialColor(glm::vec4(1.0f, 1.f, 1.f, 1.f));
		Obj->SetMaterialProperties(glm::vec4(0.0f, 0.5f, 0.f, 0.f));
		Obj->SetAlbedoTexture(&CarTexture);
		pCar = Obj;
	}
#endif
}

void CreateTweakBar()
{
	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); // same as MouseMotion
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);

	// send the ''glutGetModifers'' function pointer to AntTweakBar
	TwGLUTModifiersFunc(glutGetModifiers);

	TweakBar = TwNewBar("Hybrid Render");
	TwBar* bar = TweakBar;

	typedef enum { DEFERRED, RAYTRACED, PATHTRACED } ERenderModes;
	TwEnumVal RenderModes[] = { {DEFERRED, "Deferred"}, {RAYTRACED, "RayTraced"}, {PATHTRACED, "PathTraced"} };

	typedef enum { None, ForceZero, ForceOne } ERenderModes;
	TwEnumVal PropertyOverride[] = { {None, "None"}, {ForceZero, "ForceZero"}, {ForceOne, "ForceOne"} };

	auto SetRenderModeCB = [](const void* value, void*)
	{
		if (*(int*)value == 0)
		{
			bPathTracing = false;
			bRaytracing = false;
		}
		else if (*(int*)value == 1)
		{
			bPathTracing = false;
			bRaytracing = true;
		}
		else if (*(int*)value == 2)
		{
			bPathTracing = true;
			bRaytracing = false;
		}

		AAFrame = 0;
	};

	auto GetRenderModeCB = [](void* value, void*)
	{
		if (bPathTracing)
			*(int*)value = 2;
		else if (bRaytracing)
			*(int*)value = 1;
		else
			*(int*)value = 0;
	};

	auto SetPropertyOverrideCB = [](const void* value, void* pVar)
	{
		float& fBias = *(float*)pVar;
		if (*(int*)value == 0)
			fBias = 0.f;
		else if (*(int*)value == 1)
			fBias = -1.f;
		else if (*(int*)value == 2)
			fBias = 1.f;

		AAFrame = 0;
	};

	auto GetPropertyOverrideCB = [](void* value, void* pVar)
	{
		float& fBias = *(float*)pVar;
		if (fBias == 0.f)
			*(int*)value = 0;
		else if (fBias == -1.f)
			*(int*)value = 1;
		else if (fBias == 1.f)
			*(int*)value = 2;
	};

	TwType RenderModeType = TwDefineEnum("Render Mode", RenderModes, 3);
	TwType ProperyOverrideType = TwDefineEnum("ProperyOverride", PropertyOverride, 3);

	TwAddVarCB(bar, "Render Mode", RenderModeType, SetRenderModeCB, GetRenderModeCB, nullptr, "");
	TwAddVarRW(bar, "Separated Specular (PathTrace)", TwType::TW_TYPE_BOOLCPP, &bSeparatedSpecular, "");
	TwAddVarRW(bar, "Half Res", TwType::TW_TYPE_BOOLCPP, &bHalfRes, nullptr);
	TwAddVarRW(bar, "SamplesPerPixel", TwType::TW_TYPE_INT32, &SPP, "min=1 max=8");
	TwAddVarCB(bar, "Roughness Override", ProperyOverrideType, SetPropertyOverrideCB, GetPropertyOverrideCB, &RoughnessBias, "");
	TwAddVarCB(bar, "Metallic Override", ProperyOverrideType, SetPropertyOverrideCB, GetPropertyOverrideCB, &MetallicBias, "");
	TwAddVarRW(bar, "Num Bounces", TwType::TW_TYPE_INT8, &NumBounces, "min=0 max=3");
	TwAddVarRW(bar, "Reflections", TwType::TW_TYPE_BOOLCPP, &bShowReflections, nullptr);
	TwAddVarRW(bar, "Ambient Occlusion", TwType::TW_TYPE_BOOLCPP, &bShowSSAO, nullptr);
	TwAddVarRW(bar, "Bloom", TwType::TW_TYPE_BOOLCPP, &bShowBloom, nullptr);
	TwAddVarRO(bar, "Depth of Field", TwType::TW_TYPE_BOOLCPP, &bShowDOF, nullptr);
	TwAddVarRW(bar, "Motion Blur", TwType::TW_TYPE_BOOLCPP, &bShowMotionBlur, nullptr);
	TwAddVarRW(bar, "Tone Map", TwType::TW_TYPE_BOOLCPP, &bShowToneMap, nullptr);
	TwAddVarRW(bar, "Temporal Anti-Aliasing", TwType::TW_TYPE_BOOLCPP, &bShowAccumAA, nullptr);
	TwAddVarRW(bar, "ScreenPercentage", TwType::TW_TYPE_FLOAT, &NewScreenPct, "min=0.4 max=2.0 step=0.2");
	TwAddVarRW(bar, "Light Direction", TwType::TW_TYPE_DIR3F, &LightDir, nullptr);
	TwAddVarRW(bar, "Sky", TwType::TW_TYPE_INT32, &iSkyBox, "min=0 max=3");
	TwAddVarRW(bar, "Sky Intensity", TwType::TW_TYPE_FLOAT, &SkyBoxIntensity, "min=0 step=0.1");
	TwAddVarRW(bar, "Sky Rotation", TwType::TW_TYPE_QUAT4F, &CubemapRotQuat, nullptr);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(BackBufferSize[0], BackBufferSize[1]);
	glutCreateWindow("Hybrid Raytracer Test");

	glewInit();

	TwInit(TwGraphAPI::TW_OPENGL, nullptr);

	glutIdleFunc(MainLoop);
	glutDisplayFunc(Display);
	glutReshapeFunc(ReshapeFunc);

	glutReshapeWindow(BackBufferSize[0], BackBufferSize[1]);

	CompileShaders();

	CreateTweakBar();

	CreateScene();

	AppStartTime = std::chrono::high_resolution_clock::now();

	glDebugMessageCallbackARB(glDebugOutput, nullptr);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, nullptr, GL_FALSE);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM_ARB, 0, nullptr, GL_TRUE);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_ARB, 0, nullptr, GL_TRUE);

	glutMainLoop();

	TwTerminate();

	return 0;
}


