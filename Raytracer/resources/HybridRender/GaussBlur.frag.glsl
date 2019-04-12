
layout(location = 0) in vec2 iTexCoords;

layout(binding=0) uniform sampler2D InputTexture;

uniform int TexLevel;
uniform vec2 TexSize;

uniform vec2 KernelDir;
uniform float Sigma;
uniform int NumSamples;

uniform float Factor;
uniform float ColorThreshold;
 
void main()
{
	vec2 KernelSize = KernelDir / vec2(textureSize(InputTexture, int(TexLevel)));
	
	vec4 acc = texture(InputTexture, iTexCoords, TexLevel);

	for (int i = 0; i <= NumSamples; i++) 
	{  
		float coeff = exp(-0.5 * float(i) * float(i) / (Sigma * Sigma));
		acc += max(vec4(0.0), texture(InputTexture, clamp(iTexCoords - vec2(i) * KernelSize, 0.0, 1.0), TexLevel) - ColorThreshold) * coeff;
		acc += max(vec4(0.0), texture(InputTexture, clamp(iTexCoords + vec2(i) * KernelSize, 0.0, 1.0), TexLevel) - ColorThreshold) * coeff;
	} 
	
	acc /= sqrt(2 * 3.14159 * (Sigma * Sigma));

	gl_FragColor = acc;
}
