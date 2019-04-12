
layout(location = 0) in vec2 iTexCoords;

layout(binding=0) uniform sampler2DMS GBuffer_RT0;
layout(binding=1) uniform sampler2DMS GBuffer_RT1;
layout(binding=2) uniform sampler2DMS GBuffer_RT2;
layout(binding=3) uniform sampler2D BlurScene;

uniform mat4 ViewProjectionMatrix;

uniform vec3 CameraPos;
uniform vec2 ViewportSize;
uniform vec4 CameraToWorld[4];

vec3 RayCast(vec3 WorldPos, vec3 WorldReflect, float Depth)
{
	vec4 ClipStart = ViewProjectionMatrix * vec4(WorldPos.xyz, 1.0);
	vec4 ClipEnd = ViewProjectionMatrix * vec4(WorldPos.xyz + WorldReflect * Depth * 1000.f, 1.0);

	vec3 ScreenStart = ClipStart.xyz / ClipStart.w;
	vec3 ScreenEnd = ClipEnd.xyz / ClipEnd.w;

	vec3 StepScreen = ( ScreenEnd - ScreenStart ) / length( ScreenEnd.xy - ScreenStart.xy );

	vec3 RayStart = vec3(ScreenStart.xy * 0.5 + 0.5, ScreenStart.z);
	vec3 RayStep = vec3(StepScreen.xy * 0.5 + 0.5, StepScreen.z);

	for(int i = 0; i < 256; i++)
	{
		vec3 RayPos = RayStart + RayStep * i * 16;

		Depth = 1.0-texelFetch(GBuffer_RT2, ivec2(RayPos.xy * vec2(1280,720)), 0).x;
		Depth *= 1000.f;

		return ScreenStart.zzz / 10;

		if(Depth < RayPos.z)
		{
			//return texture(BlurScene, RayPos.xy).xyz;
			return texelFetch(GBuffer_RT1, ivec2(RayPos.xy * vec2(1280,720)), 0).xyz;
		}
	}

	return vec3(1,0,0);
}

void main()
{
	ivec2 TC;
	vec3 fTC;
	
	vec3 Color = vec3(0.0);

	fTC.xy = gl_FragCoord.xy / vec2(ViewportSize);

	TC = ivec2(gl_FragCoord.xy);
	
	vec3 A = mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, fTC.x);
	vec3 B = mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, fTC.x);
	vec3 C = mix(A, B, fTC.y);

	vec4 NormalMetallic = texelFetch(GBuffer_RT0, TC, 0);
	float Depth = 1.0-texelFetch(GBuffer_RT2, TC, 0).x;
	vec3 wPos = mix(CameraPos, C, Depth.x);

	vec3 WorldNormal = normalize(NormalMetallic.xyz*2-1);
	vec3 WorldCameraDir = normalize(wPos.xyz-CameraPos.xyz);
	vec3 WorldReflection = reflect(WorldCameraDir.xyz, WorldNormal.xyz);

	gl_FragColor.w = 1.0;
	gl_FragColor.xyz = RayCast(wPos, WorldReflection, Depth);
}

