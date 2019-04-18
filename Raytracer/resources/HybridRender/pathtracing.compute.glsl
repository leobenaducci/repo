
layout(rgba16f) uniform writeonly image2D OutputBuffer;

layout(binding = 0) uniform sampler2D InputNormalMetallic;
layout(binding = 1) uniform sampler2D InputColorRoughness;
layout(binding = 2) uniform sampler2D InputLinearDepth;
layout(binding = 2) uniform sampler2D SkyBox2D;
layout(binding = 4) uniform sampler2D InputPrevLinearDepthBuffer;
layout(binding = 5) uniform sampler2D VelocityBuffer;

uniform vec2 iResolution;
uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];
uniform vec3 PrevCameraPos;
uniform vec4 PrevCameraToWorld[4];
uniform ivec2 Offset;
uniform int Scale;
uniform int iFrame;
uniform vec3 LightDir;
uniform mat4 CubemapRotation;

uniform float RoughnessBias = 0.f;
uniform float MetallicBias = 0.f;

uniform int SamplesPerPixel = 2;

uniform float SkyBoxIntensity;

lowp float GDiffFactor = 1.f;
lowp float GSpecFactor = 1.f;
lowp float GMetallic;
lowp float GRoughness;
lowp int iSample = 0;
vec3 GWorldPos;
mediump vec3 GNormal;

vec3 GetWorldPosition(vec2 TexCoords)
{
	vec3 A = mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, TexCoords.x);
	vec3 B = mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, TexCoords.x);
	vec3 C = mix(A, B, TexCoords.y);

	vec4 Depth = texture2D(InputLinearDepth, TexCoords, 0);
	return mix(CameraPos, C, Depth.x);
}

vec3 GetPrevWorldPosition(vec2 TexCoords)
{
	vec3 A = mix(PrevCameraToWorld[0].xyz, PrevCameraToWorld[1].xyz, TexCoords.x);
	vec3 B = mix(PrevCameraToWorld[2].xyz, PrevCameraToWorld[3].xyz, TexCoords.x);
	vec3 C = mix(A, B, TexCoords.y);

	vec4 Depth = texture2D(InputPrevLinearDepthBuffer, TexCoords, 0);
	return mix(PrevCameraPos, C, Depth.x);
}

void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload)
{
	mediump vec3 Dir = (vec4(Ray.Direction, 0.0) * CubemapRotation).xyz;

	mediump float theta = atan(Dir.z, Dir.x) + PI;
	mediump vec4 col = textureLod(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)), 7 * GRoughness);
	
	vec3 IncomingLight = max(vec3(0.0), CookTorranceDir(-Ray.Direction, GWorldPos, Ray.Origin, vec4(GNormal, GMetallic), vec4(col.xyz, GRoughness), GDiffFactor, GSpecFactor));
	Payload.Accum.xyz = IncomingLight * SkyBoxIntensity;
}

bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload)
{
	return false;
}

float GetRoughness(float Roughness)
{
	return clamp(Roughness + RoughnessBias, 0.1f, 0.9f);
}

float GetMetallic(float Metallic)
{
	return clamp(Metallic + MetallicBias, 0.f, 1.f);
}

RecursiveRay_s ClosestHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, int Depth, inout Payload_s Payload)
{
	if (RayType == RAY_MAIN)
	{
		if (Material.EmissiveUnused.x > 0)
		{
			Payload.Accum.xyz *= Material.Color.xyz * Material.EmissiveUnused.x;
		}
		else if (Depth < NUM_BOUNCES)
		{
			vec3 IncomingLight = max(vec3(0.0), CookTorranceDir(-Ray.Direction, GWorldPos, Ray.Origin, vec4(GNormal, GMetallic), vec4(Material.Color.xyz, GRoughness), GDiffFactor, GSpecFactor));
			Payload.Accum.xyz *= IncomingLight;

			GDiffFactor = 1.f;
			GSpecFactor = 1.f;

			vec3 RandDir = rand3(iFrame * 8 + iSample);
			vec3 Dir = RandDir * sign(dot(RandDir, GNormal));
			Dir = mix(Dir, reflect(Ray.Direction, GNormal) + RandDir * GRoughness, GMetallic);

			GRoughness = GetRoughness(Material.RoughnessMetallic.x);
			GMetallic = GetMetallic(Material.RoughnessMetallic.y);
			GNormal = Result.Normal;
			GWorldPos = Result.Pos;
			return NewRay(Result.Pos + Result.Normal * GRoughness, Dir);
		}
		else
		{
			Payload.Accum.xyz *= vec3(0);
		}
	}

	return StopRay();
}

//gl_WorkGroupID
void main()
{
	const ivec2 FragCoord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	lowp vec4 ColorRoughness = texelFetch(InputColorRoughness, FragCoord * Scale + Offset, 0);
	mediump float Roughness = ColorRoughness.w;

	memoryBarrier();

	vec3 Output = vec3(1.0);

	if (any(Roughness > 0))
	{
		Output = vec3(0.0);
		mediump float Total = 0.f;

		const vec2 FragUV = (vec2(FragCoord.xy) / iResolution.xy);
		const vec3 CameraEnd = mix(mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, FragUV.x),
			mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, FragUV.x),
			FragUV.y).xyz;

		const vec3 Eye = normalize(CameraEnd - CameraPos);
		vec4 NormalMetallic = texelFetch(InputNormalMetallic, FragCoord * Scale + Offset, 0);
		vec4 Depth = texelFetch(InputLinearDepth, FragCoord * Scale + Offset, 0);

		vec3 wPos = mix(CameraPos, CameraEnd, Depth.x).xyz;

		mediump vec3 WN = normalize(NormalMetallic.xyz * 2 - 1);

		//
		mediump vec4 VelSample = texture(VelocityBuffer, FragUV, 0);
		mediump vec2 Vel2D = VelSample.xy;
		mediump vec2 PrevClipPos = clamp(FragUV.xy - Vel2D.xy * 0.5f, vec2(0.0), vec2(1.0));
		lowp float PosDif = length(GetWorldPosition(FragUV.xy) - GetPrevWorldPosition(PrevClipPos.xy));
		lowp float BlendWeight = clamp(PosDif * 0.25f, 0.0, 1.0);

		//Emissive
		Output += VelSample.z * ColorRoughness.xyz;

		bool bMoreSamples = BlendWeight > 0.25f;
		memoryBarrier();
		
		int NumSamplesPerPixel = SamplesPerPixel;
		if (any(bMoreSamples))
			NumSamplesPerPixel *= 2;
		//

		for (iSample = 0; iSample < NumSamplesPerPixel; iSample++)
		{
			vec3 RandDir = rand3(iFrame * 16 + iSample);
			RandDir = RandDir * sign(dot(RandDir, WN));

			GRoughness = GetRoughness(Roughness);
			GMetallic = GetMetallic(NormalMetallic.w);
			vec3 SpecDir = reflect(Eye, WN) + RandDir * GRoughness;

			GWorldPos = wPos;
			GNormal = WN;

#ifdef SEPARATED_SPECULAR
			GDiffFactor = 0.f;
			GSpecFactor = 1.f;
			Output += Ray_Main(wPos + WN * Depth.x * 15, SpecDir).Accum.xyz;
			Total += 1.f;

			if (any(GMetallic == 0.f))
			{
				GDiffFactor = 1.f - GMetallic;
				GSpecFactor = 0.f;
				Output += Ray_Main(wPos + WN * Depth.x * 15, RandDir).Accum.xyz;
				Total += 1.f;
			}
#else
			GDiffFactor = 1.f;
			GSpecFactor = 1.f;
			Output += Ray_Main(wPos + WN * Depth.x * 15, mix(RandDir, SpecDir, GMetallic)).Accum.xyz;
			Total += 1.f;
#endif
		}

		Output.xyz /= Total;
	}
	
	Output.xyz = clamp(Output.xyz, vec3(0.0), vec3(4.0));
	imageStore(OutputBuffer, FragCoord, vec4(Output.xyz, Roughness > 0 ? 1.0 : 0.0));
}