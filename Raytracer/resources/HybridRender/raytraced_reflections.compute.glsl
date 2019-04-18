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
	if (RayType == RAY_SHADOW)
	{
		Payload.Accum = vec4(1.0);
	}
	else
	{
		vec3 Dir = (vec4(Ray.Direction, 0.0) * CubemapRotation).xyz;

		float theta = atan(Dir.z, Dir.x) + PI;
		vec4 col = textureLod(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)), GRoughness * 7);
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
	vec3 Light = CookTorranceDir(-LightDir, Result.Pos, CameraPos, vec4(Result.Normal.xyz, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x)) * 5;
	Light *= Ray_Shadow(Result.Pos + Result.Normal, -LightDir).Accum.xyz;

	Payload.Accum.xyz += (Light + Material.Color.xyz * Material.EmissiveUnused.x);

	if (Depth <= 1)
	{
		return NewRay(Result.Pos + normalize(Result.Normal), reflectN(Ray.Direction, Result.Normal));
	}

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

		vec3 WR = reflect(WV, WN);

		GRoughness = clamp(ColorRoughness.w, 0.05, 0.65);

		vec3 RayDir = WR + rand3(iFrame) * GRoughness;
		vec3 Factor = CookTorranceDir(-RayDir, WV, CameraPos, vec4(WN.xyz, NormalMetallic.w), vec4(ColorRoughness.xyz, GRoughness), 0.f, 1.f);

		Payload_s Payload;
		if (any(max(max(Factor.x, Factor.y), Factor.z) > 0))
		{
			Payload = Ray_Main(wPos + WN * Depth.x * 10, RayDir);
			Payload.Accum.xyz *= Factor;
		}

		Output = clamp(Payload.Accum.xyz, vec3(0.0), vec3(8.0));
	}

	imageStore(OutputBuffer, FragCoord, vec4(Output, 1.0));
}