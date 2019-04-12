

layout(location = 0) in vec2 iTexCoords;

layout(binding = 0) uniform sampler2D InputTexture;
layout(binding = 1) uniform sampler2D OldPostProcessBuffer;
layout(binding = 2) uniform sampler2D VelocityBuffer;

layout(binding = 3) uniform sampler2D DepthBuffer;
layout(binding = 4) uniform sampler2D PrevDepthBuffer;

uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];

uniform vec3 PrevCameraPos;
uniform vec4 PrevCameraToWorld[4];

uniform float AAFrame;
uniform float RandX, RandY;

vec3 GetWorldPosition(vec2 TexCoords)
{
	vec3 A = mix(CameraToWorld[0].xyz, CameraToWorld[1].xyz, TexCoords.x);
	vec3 B = mix(CameraToWorld[2].xyz, CameraToWorld[3].xyz, TexCoords.x);
	vec3 C = mix(A, B, TexCoords.y);

	vec4 Depth = texture2D(DepthBuffer, TexCoords, 0);
	return mix(CameraPos, C, Depth.x);
}

vec3 GetPrevWorldPosition(vec2 TexCoords)
{
	vec3 A = mix(PrevCameraToWorld[0].xyz, PrevCameraToWorld[1].xyz, TexCoords.x);
	vec3 B = mix(PrevCameraToWorld[2].xyz, PrevCameraToWorld[3].xyz, TexCoords.x);
	vec3 C = mix(A, B, TexCoords.y);

	vec4 Depth = texture2D(PrevDepthBuffer, TexCoords, 0);
	return mix(PrevCameraPos, C, Depth.x);
}

void main()
{
	vec2 Vel2D = texture(VelocityBuffer, iTexCoords, 0).xy;
	float Vel2DSize = dot(Vel2D.xy, Vel2D.xy) + 1.0e-6;

	vec2 PrevClipPos = clamp(iTexCoords.xy - Vel2D.xy * 0.5f, vec2(0.0), vec2(1.0));

	vec4 OldBuffer = clamp(texture(OldPostProcessBuffer, PrevClipPos.xy, 0).xyzw, vec4(0.0), vec4(1000.0));

	float BlendWeight = 0.0;

	//BlendWeight = clamp(1.0 - (Vel2DSize * 50.0), 0.0, 1.0);
	//BlendWeight = clamp(1.0 - sqrt(abs(OldBuffer.a - Vel2DSize)) * 1.0, 0.0, 1.0);
	//BlendWeight = clamp(1.0 - OldBuffer.a * 2.0 * 65535.0, 0.0, 1.0);
	float PosDif = length(GetWorldPosition(iTexCoords.xy) - GetPrevWorldPosition(PrevClipPos.xy));
	BlendWeight = 1.0 - clamp(PosDif * 0.25f, 0.0, 1.0);

	if (PrevClipPos.x <= 0.001 || PrevClipPos.y <= 0.001 || PrevClipPos.x >= 0.999 || PrevClipPos.y >= 0.999)
		BlendWeight = 0;

	vec4 Accum = texture2D(InputTexture, iTexCoords + vec2(RandX, RandY));
	Accum.xyz = mix(Accum.xyz, OldBuffer.xyz, 0.97 * BlendWeight /* Accum.a*/);
	//Accum.xyz = mix(min(Accum.xyz, 4096.f), OldBuffer.xyz, 1.f - (1.f / AAFrame));

	float AccumA = Vel2DSize;

	gl_FragColor =  vec4(BlendWeight);//  vec4(Accum.xyz, AccumA);
	gl_FragColor = vec4(Accum.xyz, AccumA);
}
