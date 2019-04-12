

layout(location = 0) in vec2 iTexCoords;

layout(binding=0) uniform sampler2D SceneColor;
layout(binding=1) uniform sampler2D SceneVelocity;

uniform mat4 ViewProjectionMatrix;
uniform mat4 PrevViewProjectionMatrix;
uniform vec3 CameraPos;
uniform vec4 CameraToWorld[4];

void main()
{
	vec2 Vel = texture(SceneVelocity, iTexCoords.xy, 0).xy;

	vec4 CameraClipPos = vec4(0.0);
	vec4 CameraPrevClipPos = vec4(0.0);
	
	vec2 ClipPos = iTexCoords.xy;
	vec2 ClipPosVel = Vel.xy + (CameraPrevClipPos.xy - CameraClipPos.xy);
	vec2 PrevClipPos = ClipPos.xy + ClipPosVel;

	int NumSteps = 16;
	vec2 StepSize = (ClipPos.xy - PrevClipPos.xy) / float(NumSteps);
	
	float Level = max(abs(ClipPosVel.x), abs(ClipPosVel.y)) * 16;

	vec3 ColorAccum = vec3(0.0);
	float Norm = 0.0;
	for(int i = 0; i < NumSteps; i++)
	{
		float Factor = 1.0 / (i/4 + 1.0);
		ColorAccum += texture(SceneColor, ClipPos.xy + StepSize * i, Level).xyz * Factor;

		Norm += Factor;
	}

	gl_FragColor = vec4(ColorAccum / Norm, 1.0);
}
