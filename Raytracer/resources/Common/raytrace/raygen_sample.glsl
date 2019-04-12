layout(location = 0, rgba16f) uniform writeonly image2D BackBuffer;
layout(location = 1, rgba16f) uniform writeonly image2D VelocityBuffer;

uniform samplerCube SkyBox;
uniform sampler2D SkyBox2D;
uniform vec2 iResolution;
uniform vec3 CameraPos;
uniform mat4 ViewMatrix;
uniform vec4 CameraToWorld[4];
uniform float iTime;
uniform float iFrame;
uniform float iCameraSpeed;
uniform mat4 CubemapRotation;

uniform mat4 ViewProjectionMatrix;
uniform mat4 PrevViewProjectionMatrix;

vec3 LightDir = -normalize(vec3(1.5, 1, 2));

void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload)
{
	if (RayType == RAY_SHADOW)
	{
		return;
	}
	
	vec3 Dir = (vec4(Ray.Direction, 0.0) * CubemapRotation).xyz;
	//
	//Payload.Accum.xyz *= 512 * pow(textureCube(SkyBox, Dir).xyz, vec3(4.0));
	//
	float theta = atan(Dir.z, Dir.x) + PI;
	vec4 col = textureLod(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)), 7 * Payload.Roughness);
	Payload.Accum.xyz *= (16 * col.xyz);// *pow(2.0, (col.a*255.0) - 128.0);

	//Payload.Accum.xyz += 4 * mix(vec3(0.5, 0.5, 0.7) * 0.5, vec3(0.5, 0.5, 0.7) * 4, Ray.Direction.y*0.5);
}

bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload)
{
	if (RayType == RAY_SHADOW)
	{
		Payload.Accum.xyz = vec3(0.0);
		//Payload.Accum.xyz = mix(vec3(0.5, 0.5, 0.7) * 0.5, vec3(0.5, 0.5, 0.7) * 4, Ray.Direction.y*0.5);
		return true;
	}

	return false;
}

RecursiveRay_s ClosestHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, int Depth, inout Payload_s Payload)
{
	float ior = 0.7;
	if (RayType == RAY_MAIN)
	{
		if (Depth == 1)
		{
			Payload.WorldPos = vec4(Result.Pos, 1.0);
		}
#if 0
		vec3 Dir = mix(Result.Normal, reflect(Ray.Direction, Result.Normal), Material.RoughnessMetallic.y) + rand3(iFrame) * Material.RoughnessMetallic.x * 0.77;
		vec3 IncomingLight = max(vec3(0.0), CookTorranceDir(-Dir, Result.Pos, CameraPos, vec4(Result.Normal, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x)));
		Payload.Accum.xyz *= IncomingLight * PI;

		if (Depth <= 2)
		{
			Payload.Roughness = Material.RoughnessMetallic.x;
			return NewRay(Result.Pos + Result.Normal * Material.RoughnessMetallic.x, Dir);
		}
		else
		{
			Payload.Accum.xyz *= 0.0;
		}

#elif 0
		Payload.Accum.xyz = vec3(0.0);
		Payload.Roughness = Material.RoughnessMetallic.x;
		for (int i = 0; i < 1; i++)
		{
			//vec3 Dir = -LightDir;
			vec3 Dir = mix(Result.Normal, reflect(Ray.Direction, Result.Normal), Material.RoughnessMetallic.y) + rand3(iFrame) * Material.RoughnessMetallic.x * 0.77;
			//vec3 Dir = mix(Result.Normal, reflect(Ray.Direction, Result.Normal), Material.RoughnessMetallic.y) + rand3(iFrame * 1 + i) * Material.RoughnessMetallic.x;r
			vec3 IncomingLight = max(vec3(0.0), CookTorranceDir(-Dir, Result.Pos, CameraPos, vec4(Result.Normal, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x))) * 5;
			//vec3 IncomingLight = max(vec3(0.0), dot(Result.Normal, Dir)) * Material.Color.xyz;
			Payload.Accum.xyz = min(Ray_Shadow(Result.Pos + Result.Normal * Material.RoughnessMetallic.x, Dir).Accum.xyz * IncomingLight, vec3(25.0));
		}
		
		//Payload.Accum.xyz /= 1;

		Payload.Accum.xyz = clamp(Payload.Accum.xyz, vec3(0.0), vec3(10.0));

#elif 0
		//vec3 Dir = mix(Result.Normal, reflect(Ray.Direction, Result.Normal), Material.RoughnessMetallic.y) + rand(Result.Pos.xyz * 10 /*+ vec3(iFrame + 1)*/) * Material.RoughnessMetallic.x;
		//vec3 IncomingLight = max(vec3(0.0), CookTorranceDir(-Dir, Result.Pos, CameraPos, vec4(-Result.Normal, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x)));
		vec3 Dir = -LightDir + rand(Result.Pos.xyz*10)*0.07;
		vec3 IncomingLight = max(vec3(0.0), dot(Dir, Result.Normal) * Material.Color.xyz);
		
		Payload.Accum.xyz = IncomingLight;
		Payload.Accum.xyz *= Ray_Shadow(Result.Pos + Result.Normal * Material.RoughnessMetallic.x, Dir).Accum.xyz;
		Payload.Accum.xyz = clamp(Payload.Accum.xyz, vec3(0.0), vec3(100.0));

		//Payload.Roughness = Material.RoughnessMetallic.x;
		//if (Depth < 3)
		//{
		//	return NewRay(Result.Pos + Result.Normal * Material.RoughnessMetallic.x, Dir);
		//}
		//else
		{
			//Payload.Accum.xyz *= Ray_Shadow(Result.Pos + Result.Normal * Material.RoughnessMetallic.x, Dir).Accum.xyz;
		}
#elif 1
		
		vec3 shadowFactor = Ray_Shadow(Result.Pos + Result.Normal, -LightDir, 0).Accum.xyz;
		Payload.Accum.xyz = shadowFactor * CookTorranceDir(LightDir, Result.Pos, CameraPos, vec4(Result.Normal, Material.RoughnessMetallic.y), vec4(Material.Color.xyz, Material.RoughnessMetallic.x)) * 5.0;

#elif 0
		vec3 NextRayDir = normalize(Result.Normal + rand(Result.Pos.xyz*iTime) * Result.Material.RoughnessMetallic.x);
		if (Depth == 1)
		{
			Payload.Accum.xyz = Material.Color.xyz * (Material.RoughnessMetallic.z+1);
			return NewRay(Result.Pos + Result.Normal, NextRayDir);
		}
		else
		{
			vec3 shadowFactor = Ray_Shadow(Result.Pos + Result.Normal, NextRayDir, 0).Accum.xyz;
			Payload.Accum.xyz *= (1.f + shadowFactor);
		}
#else
		if(Depth == 1)
		{
			vec3 NextRayDir = normalize(Result.Normal + rand(Result.Pos*10) * 0.5f * Material.RoughnessMetallic.x);
			vec3 shadowFactor = Ray_Shadow(Result.Pos + Result.Normal, NextRayDir, 0).Accum.xyz;
			
			Payload.Accum.xyz = 5.f * shadowFactor * Material.Color.xyz + Material.Color.xyz * Material.RoughnessMetallic.z;
			
			NextRayDir = normalize(reflect(Ray.Direction, Result.Normal) + rand(Result.Pos * 10) * Material.RoughnessMetallic.x);
			Payload.Strength = 0.3f;// Material.RoughnessMetallic.y;
			return NewRay(Result.Pos + Result.Normal * 0.1f, NextRayDir);
		}
		else
		{
			vec3 NextRayDir = normalize(Result.Normal + vec3(200, 600, 200) + rand(Result.Pos * 10) * 0.5f * Material.RoughnessMetallic.x);
			vec3 shadowFactor = Ray_Shadow(Result.Pos + Result.Normal, NextRayDir, 0).Accum.xyz;

			Payload.Accum.xyz += Payload.Strength * 5.f * shadowFactor * Material.Color.xyz;
		}

#endif
	}

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

	vec4 ClipPos = ViewProjectionMatrix * vec4(Payload.WorldPos.xyz, 1.0);
	vec4 PrevClipPos = PrevViewProjectionMatrix * vec4(Payload.WorldPos.xyz, 1.0);

	vec2 Velocity = (ClipPos.xy / ClipPos.w) - (PrevClipPos.xy / PrevClipPos.w);

	imageStore(BackBuffer, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(Output, 1.0));

	imageStore(VelocityBuffer, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), vec4(mix(vec2(100.0), Velocity.xy, Payload.WorldPos.w), 0.0, 0.0));
}