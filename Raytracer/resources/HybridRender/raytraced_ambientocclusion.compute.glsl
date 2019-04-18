
layout(rgba16f) uniform writeonly image2D OutputBuffer;

layout(binding = 0) uniform sampler2D InputColorRoughness;
layout(binding = 1) uniform sampler2D InputLinearDepth;
layout(binding = 2) uniform sampler2D InputNormalMetallic;

uniform vec2 iResolution;
uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];
uniform ivec2 Offset;
uniform int Scale;
uniform int iFrame;
uniform mat4 CubemapRotation;
uniform sampler2D SkyBox2D;
float GRoughness;

void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload)
{
	vec3 Dir = (vec4(Ray.Direction, 0.0) * CubemapRotation).xyz;
	
	float theta = atan(Dir.z, Dir.x) + PI;
	vec4 col = textureLod(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)), 8);
	Payload.Accum = col;
}

bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload)
{
	Payload.Accum = vec4(0.0);
	return true;
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
	
	vec3 wPos = mix(CameraPos, CameraEnd, Depth.x).xyz;
	
	vec3 WN = normalize(NormalMetallic.xyz * 2 - 1);
	
	vec3 Output = vec3(0.0);
	
	GRoughness = ColorRoughness.w;

	vec3 RayDir = WN + rand3(iFrame) * 0.95;
	Output = CookTorranceDir(-RayDir, Eye, CameraPos, vec4(WN.xyz, NormalMetallic.w), ColorRoughness, 1.f, 0.f);

	Output *= Ray_Shadow(wPos + WN * Depth.x * 15, RayDir).Accum.xyz * ColorRoughness.xyz;

	imageStore(OutputBuffer, FragCoord, vec4(Output, 1.0));
}