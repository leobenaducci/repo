layout(location = 0) in vec2 iTexCoords;

uniform sampler2D InputAlbedo;
uniform sampler2D InputNormal;
uniform sampler2D InputTex;
uniform int InputTexLevel;
uniform vec2 iResolution;

float exponent = 2;

#define SIZE 1
void main() 
{
	vec4 CenterAlbedo = texture2D(InputAlbedo, iTexCoords.xy, InputTexLevel);
	vec4 CenterNormal = texture2D(InputNormal, iTexCoords.xy, InputTexLevel)*2-1;

	vec4 Color = texture2D(InputTex, iTexCoords.xy, InputTexLevel);
	float Total = 1;

	for (int y = -SIZE; y <= SIZE; y++)
	{
		for (int x = -SIZE; x <= SIZE; x++)
		{
			vec2 TC = iTexCoords.xy + vec2(x,y) / iResolution;
			vec4 Albedo = texture2D(InputAlbedo, TC.xy, InputTexLevel);
			vec4 Normal = texture2D(InputNormal, TC.xy, InputTexLevel)*2-1;
			
			float InvWeightAbledo = abs(dot(Albedo - CenterAlbedo, vec4(1,1,1,0))) * 4;
			float InvWeightNormal = (1.0-max(0.0, dot(Normal, CenterNormal))) * 4 ;
			float Weight = 1.0f - clamp(InvWeightNormal, 0.0, 1.0);
			Weight *= 1 - max(0.0, sqrt(x*x+y*y) * 0.1f);

			Color += texture2D(InputTex, TC.xy, InputTexLevel) * Weight;
			Total += Weight;			
		}
	}

	gl_FragColor = (Color / Total) * texture2D(InputAlbedo, iTexCoords.xy, 0);;
}