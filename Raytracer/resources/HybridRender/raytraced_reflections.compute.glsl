#define LightPos vec3(0, 72, 0)

layout(rgba16f) uniform writeonly image2D OutputBuffer;

layout(binding = 0) uniform sampler2D InputColorRoughness;
layout(binding = 1) uniform sampler2D InputLinearDepth;
layout(binding = 2) uniform sampler2D InputNormalMetallic;

uniform vec2 iResolution;
uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];
uniform ivec2 Offset;
uniform int Scale;
uniform vec3 LightDir;
uniform int iFrame;
uniform mat4 CubemapRotation;
uniform sampler2D SkyBox2D;
uniform float SkyBoxIntensity;

layout(binding = 4) uniform sampler2D Shadowmap;
uniform mat4 ShadowmapMatrix;

float GRoughness;

void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload)
{
	//if (RayType == RAY_SHADOW)
	//{
	//	Payload.Accum = vec4(1.0);
	//}
	//else
	{
		vec3 Dir = (vec4(Ray.Direction, 0.0) * CubemapRotation).xyz;

		float theta = atan(Dir.z, Dir.x) + PI;
		vec4 col = textureLod(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)), GRoughness);
		Payload.Accum = col * SkyBoxIntensity;
	}
}

bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload)
{
	if (RayType == RAY_SHADOW)
	{
		Payload.Accum = vec4(0.0);
		return true;
	}

	return false;
}

RecursiveRay_s ClosestHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, int Depth, inout Payload_s Payload)
{
#if 1
	vec3 Light = max(0.f, dot(-LightDir, Result.Normal.xyz)) * Material.Color.xyz * 0.33; 
#else
	vec3 Light = CookTorranceDir(LightDir, Result.Pos, CameraPos, vec4(Result.Normal.xyz, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x)) * 5;
#endif

#if 0
	vec4 ShadowMapPos = ShadowmapMatrix * vec4(Result.Pos.xyz, 1.0);
	ShadowMapPos.xy = ShadowMapPos.xy * vec2(0.5) + vec2(0.5);
	Light *= texture2D(Shadowmap, ShadowMapPos.xy).x <= ShadowMapPos.z - 0.001f ? 0.0f : 1.0;
#else
	Light *= Ray_Shadow(Result.Pos + Result.Normal, -LightDir).Accum.xyz;
#endif

	Payload.Accum.xyz += (Light + Material.Color.xyz * Material.EmissiveUnused.x);

	//if (Depth <= 1 && Material.RoughnessMetallic.y > 0.95)
	//{
	//	return NewRay(Result.Pos + normalize(Result.Normal) * 1, reflectN(Ray.Direction, Result.Normal));
	//}

	return StopRay();
}

//gl_WorkGroupID
void main()
{
	const ivec2 FragCoord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	const vec2 FragUV = (vec2(FragCoord.xy) / iResolution.xy);
	const vec3 CameraEnd = mix(mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, FragUV.x),
						 mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, FragUV.x),
						 FragUV.y).xyz;
	
	const vec3 Eye = normalize(CameraEnd - CameraPos);

	vec4 ColorRoughness = texelFetch(InputColorRoughness, FragCoord * Scale + Offset, 0);
	vec4 NormalMetallic = texelFetch(InputNormalMetallic, FragCoord * Scale + Offset, 0);
	vec4 Depth = texelFetch(InputLinearDepth, FragCoord * Scale + Offset, 0);

	vec3 Output = vec3(0.0);
	if (any(ColorRoughness.w > 0.0))
	{
		vec3 wPos = mix(CameraPos, CameraEnd, Depth.x).xyz;

		vec3 WN = normalize(NormalMetallic.xyz * 2 - 1);
		vec3 WV = normalize(wPos - CameraPos);

		vec3 WR = mix(WN, normalize(reflectN(WV, WN)), NormalMetallic.w);

		GRoughness = ColorRoughness.w;
		Payload_s Payload = Ray_Main(wPos + WN * 15, WR + rand3(iFrame) * clamp(ColorRoughness.w, 0.05, 0.95));
		Output = clamp(Payload.Accum.xyz * ColorRoughness.xyz, vec3(0.0), vec3(8.0));
	}

	imageStore(OutputBuffer, FragCoord, vec4(Output, 1.0));
}