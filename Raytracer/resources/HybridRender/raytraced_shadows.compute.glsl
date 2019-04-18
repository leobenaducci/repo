#define LightPos vec3(0, 72, 0)

layout(rgba16f) uniform writeonly image2D OutputBuffer;

layout(binding = 0) uniform sampler2D InputNormalMetallic;
layout(binding = 1) uniform sampler2D InputColorRoughness;
layout(binding = 2) uniform sampler2D InputLinearDepth;

uniform vec2 iResolution;
uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];
uniform ivec2 Offset;
uniform int Scale;
uniform int FrameNum;
uniform vec3 LightDir;
uniform int RayTracedShadowsScale;

void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload)
{
	Payload.Accum.xyz = vec3(1.0);
}

bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload)
{
	Payload.Accum.xyz = vec3(0.0);
	return true;
}

RecursiveRay_s ClosestHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, int Depth, inout Payload_s Payload)
{
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

	float Alpha = texelFetch(InputColorRoughness, FragCoord*RayTracedShadowsScale, 0).w;
	vec4 Depth = texelFetch(InputLinearDepth, FragCoord*RayTracedShadowsScale, 0);
	vec4 NormalMetallic = texelFetch(InputNormalMetallic, FragCoord*RayTracedShadowsScale, 0);
	NormalMetallic.xyz = normalize(NormalMetallic.xyz * 2 - 1);
	vec3 wPos = mix(CameraPos, CameraEnd, Depth.x).xyz;
	vec3 Normal = NormalMetallic.xyz;

	vec3 Dir = -LightDir + rand3(FrameNum) * 0.1;
	float Shadow = Ray_Shadow(wPos + Dir * Depth.x * 15, Dir).Accum.x;

	imageStore(OutputBuffer, FragCoord, vec4(vec3(Shadow), 1.0));
}