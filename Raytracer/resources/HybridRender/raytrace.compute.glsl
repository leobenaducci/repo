layout(location = 0, rgba16f) uniform writeonly image2D BackBuffer;

uniform vec2 iResolution;
uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];

void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload)
{
}

bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload)
{
	if (RayType == RAY_SHADOW)
	{
		Payload.Accum.xyz = vec3(0.0);
		return true;
	}

	return false;
}

RecursiveRay_s ClosestHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, int Depth, inout Payload_s Payload)
{
	//vec3 shadowFactor = Ray_Shadow(Result.Pos + Result.Normal, -LightDir, 0).Accum.xyz;
	Payload.Accum.xyz = Result.Normal.xyz * 0.5 + 0.5; // Material.Color.xyz;// shadowFactor * CookTorranceDir(LightDir, Result.Pos, CameraPos, vec4(Result.Normal, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x)) * 5.0;

	return StopRay();
}

//gl_WorkGroupID
void main()
{
	const vec2 FragCoord = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	const vec2 FragUV = (FragCoord.xy / iResolution.xy);
	const vec3 Eye = mix(mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, FragUV.x),
						 mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, FragUV.x),
						 FragUV.y).xyz - CameraPos.xyz;

	Payload_s Payload = Ray_Main(CameraPos, normalize(Eye));

	vec3 Output = clamp(Payload.Accum.xyz, vec3(0.0), vec3(256.0));

	imageStore(BackBuffer, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(Output, 1.0));
}