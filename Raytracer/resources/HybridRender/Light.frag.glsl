
#define LightPos vec3(0, 72, 0)

layout(location = 0) in vec2 iTexCoords;

#if NUM_AA_SAMPLES > 1
	layout(binding=0) uniform sampler2DMS GBuffer_RT0;
	layout(binding=1) uniform sampler2DMS GBuffer_RT1;
	layout(binding=2) uniform sampler2DMS GBuffer_RT2;
#else
	layout(binding = 0) uniform sampler2D GBuffer_RT0;
	layout(binding = 1) uniform sampler2D GBuffer_RT1;
	layout(binding = 2) uniform sampler2D GBuffer_RT2;
	layout(binding = 6) uniform sampler2D GBuffer_RT3;
#endif

#ifdef RAYTRACED_SHADOWS
	layout(binding = 5) uniform sampler2D RaytracedShadows;
#else
	layout(binding = 4) uniform sampler2D Shadowmap;
	uniform mat4 ShadowmapMatrix;
#endif

uniform vec3 CameraPos;
uniform vec2 ViewportSize;
uniform vec4 CameraToWorld[4];


void main()
{
	ivec2 TC;
	vec2 fTC;

	TC = ivec2(gl_FragCoord.xy);
	fTC = gl_FragCoord.xy / vec2(ViewportSize);

	vec3 A = mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, fTC.x);
	vec3 B = mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, fTC.x);
	vec3 C = mix(A, B, fTC.y);

	vec3 Accum = vec3(0.0);

	vec3 wPos;
	vec4 ShadowMapPos;

#ifdef PER_SAMPLE_SHADING
	const int SampleID = gl_SampleID;
	const float NumSamples = 1.0;
#else
	const float NumSamples = float(NUM_AA_SAMPLES);
	for (int SampleID = 0; SampleID < NUM_AA_SAMPLES; SampleID++)
#endif
	{
		vec4 NormalMetallic = texelFetch(GBuffer_RT0, TC, SampleID);
		NormalMetallic.xyz = normalize(NormalMetallic.xyz * 2 - 1);
		vec4 ColorRoughness = texelFetch(GBuffer_RT1, TC, SampleID);
		vec4 Depth = texelFetch(GBuffer_RT2, TC, SampleID);
		wPos = mix(CameraPos, C, Depth.x);

#if OUTPUT_NORMAL
		Accum += NormalMetallic.xyz;
#elif OUTPUT_DEPTH
		Accum += Depth.xxx;
#else

		Accum += CookTorranceDir(wPos, CameraPos, NormalMetallic, ColorRoughness);
		//Accum += CookTorrancePoint(wPos, CameraPos, LightPos, NormalMetallic, ColorRoughness);

#ifdef RAYTRACED_SHADOWS
		Accum.xyz *= texture2D(RaytracedShadows, fTC.xy, 1).xyz;
#else
		ShadowMapPos = ShadowmapMatrix * vec4(wPos.xyz, 1.0);
		ShadowMapPos.xy = ShadowMapPos.xy * vec2(0.5) + vec2(0.5);
		
		Accum.xyz *= vec3(texture2D(Shadowmap, ShadowMapPos.xy).x <= ShadowMapPos.z-0.001f ? 0.0f : 1.0);
#endif //RAYTRACED_SHADOWS

		Accum.xyz += ColorRoughness.xyz * texelFetch(GBuffer_RT3, TC, SampleID).z;

#endif
	}

	gl_FragColor = vec4(Accum, 1.0);
}

