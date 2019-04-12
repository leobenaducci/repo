
#ifndef NUM_AA_SAMPLES
	#define NUM_AA_SAMPLES 1
#endif

layout(location = 0) in vec2 iTexCoords;

layout(binding = 0) uniform sampler2D InputColorRoughness;
layout(binding = 1) uniform sampler2D InputLinearDepth;
layout(binding = 2) uniform sampler2D InputNormalMetallic;

uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewProjectionMatrix;

uniform vec3 CameraPos;
uniform vec2 ViewportSize;
uniform vec4 CameraToWorld[4];

uniform mat4 CubemapRotation;
uniform sampler2D SkyBox2D;

uniform int FrameNum;

#define PER_PIXEL_PROJ 1
#define REFL_SAMPLES ((4+NUM_AA_SAMPLES) / NUM_AA_SAMPLES)

#define STEP_LOD 1

int MaxIter = 32 / STEP_LOD;

float RoughnessToLod(float R)
{
	return 2*R + 2;
}

#if PER_PIXEL_PROJ
float JitterScale = 0.01;
float StepSize = 2.0 * STEP_LOD;
#else
float JitterScale = 0.001;
float StepSize = 0.25 * STEP_LOD;
#endif

float FetchDepth(vec2 TC, int Sample)
{
	return textureLod(InputLinearDepth, TC, 0).x;
}

vec3 RayCast(vec3 StartPoint, vec3 Delta, float Roughness, int Sample)
{
	vec3 Color = vec3(0,0,0);
	float Depth, StartDepth;
	int i = 0;

	vec4 P;

	do
	{
#if PER_PIXEL_PROJ
		P = (ViewProjectionMatrix * vec4(StartPoint.xyz, 1.0)).xyzw;
		P.xy = (P.xy/P.w) * 0.5 + 0.5;
		P.z = (ViewMatrix * vec4(StartPoint.xyz, 1.0)).z / 1000.0;
#else
		P.xy = StartPoint.xy * 0.5 + 0.5;
		P.z = StartPoint.z;
#endif

		Depth = FetchDepth(P.xy, Sample);

		if(P.z > Depth + 0.0001)
		{
			Color = textureLod(InputColorRoughness, P.xy, RoughnessToLod(Roughness)).xyz;
			break;
		}

		StartPoint += Delta * StepSize;
		Delta *= 1.01;
	}while(i++ < MaxIter);

	P.xy = P.xy * 2.0 - 1.0;
	return Color.xyz * clamp(1.0-dot(P.xy,P.xy), 0.0, 1.0);
}

float rand(float co)
{
	return fract(52.9829189f * (fract(floor(co * 5) * 0.06711056f + floor(co * 3) * 0.00583715f))) * 2 - 1;
}

vec3 rand(vec3 co)
{
	return vec3(rand(co.x * 3 - co.z + co.y * 2), rand(co.y + co.z * 2 + co.x * 15), rand(co.z + co.y*1.3 + co.x*0.7));
}

void main()
{
	gl_FragColor = vec4(0.0);

	vec2 fTC = iTexCoords;

	vec4 Color = texture(InputColorRoughness, fTC, 0);
	float GRoughness = Color.w;
	gl_FragColor = Color;

	vec4 NewColor = vec4(0.0);
	vec4 NormalMetallic = texture(InputNormalMetallic, fTC, 0);
	
	float Depth = FetchDepth(fTC, 0);

	vec3 A = mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, fTC.x);
	vec3 B = mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, fTC.x);
	vec3 C = mix(A, B, fTC.y);

	vec3 wPos = mix(CameraPos, C, Depth);
	vec3 Normal = normalize(NormalMetallic.xyz*2-1);

	vec3 WN = Normal;
	vec3 WV = normalize(wPos - CameraPos);

	vec3 WR = normalize(reflect(WV, WN) + rand(wPos.xyz * 0.001f * FrameNum) * clamp(Color.w * 0.1, 0.0, 0.95));

#if PER_PIXEL_PROJ
	vec3 StartPoint = wPos;
	vec3 DeltaTC = normalize(WR);
#else

	vec4 ClipPos = (ViewProjectionMatrix * vec4(wPos.xyz, 1.0));
	vec4 ClipRefl = (ViewProjectionMatrix * vec4((wPos + WR).xyz, 1.0));

	ClipPos.xy /= ClipPos.w;
	ClipRefl.xy /= ClipRefl.w;

	ClipPos.z = (ViewMatrix * vec4(wPos.xyz, 1.0)).z / 1000.0;
	ClipRefl.z = (ViewMatrix * vec4((wPos + WR).xyz, 1.0)).z / 1000.0;

	vec3 StartPoint = ClipPos.xyz;
	vec3 DeltaTC = (ClipRefl-ClipPos).xyz;
#endif

	Color.xyz = vec3(0.0);
	for(int i = 0; i < REFL_SAMPLES; i++)
	{
		vec3 jitter = fract(vec3(157.23, 684.57, 0) *  (i+FrameNum)) * JitterScale - JitterScale * 0.50;
		Color.xyz += RayCast(StartPoint, DeltaTC + jitter, Color.w, 0);
	}

	vec3 Dir = (vec4(WR, 0.0) * CubemapRotation).xyz;
	float theta = atan(Dir.z, Dir.x) + PI;
	vec4 col = textureLod(SkyBox2D, vec2(theta / (2 * PI), 1 - (Dir.y*0.5 + 0.5)), GRoughness) * 0.5f;
	NewColor.xyz = mix(col.xyz, Color.xyz, clamp(dot(Color.xyz, vec3(1.0)), 0.0, 1.0));

	NewColor.xyz += Color.xyz * pow(1-clamp(dot(WN, -WV), 0.0, 1.0), 2.0) ;
	gl_FragColor.xyz += NewColor.xyz / (REFL_SAMPLES * NUM_AA_SAMPLES);
}

