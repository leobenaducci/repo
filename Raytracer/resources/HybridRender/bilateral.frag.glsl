#define SIGMA 20.0
#define BSIGMA 2.5
#define MSIZE 25

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D InputTex;
uniform int InputTexLevel = 0;

#ifdef MULTITEX
uniform sampler2D InputTex2;
uniform int InputTexLevel2 = 0;
#endif

uniform vec2 iResolution;

uniform sampler2D ModulateTex;

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v, v) / (sigma*sigma)) / sigma;
}

vec3 Tonemap(vec3 x)
{
	return x;
}

void main()
{
	vec3 c = Tonemap(texture(InputTex, iTexCoords, InputTexLevel).rgb);

	//declare stuff
	const int kSize = (MSIZE - 1) / 2;
	float kernel[MSIZE];
	vec3 final_colour = vec3(0.0);

	//create the 1-D kernel
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), SIGMA);
	}

	vec3 cc;
	float factor;
	float bZ = 1.0 / normpdf(0.0, BSIGMA);
	//read out the texels
	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			cc = Tonemap(texture2D(InputTex, iTexCoords.xy + (vec2(float(i), float(j))) / iResolution.xy, InputTexLevel).rgb);
			#ifdef MULTITEX
				vec3 cc2 = Tonemap(texture2D(InputTex2, iTexCoords.xy + (vec2(float(i), float(j))) / iResolution.xy, InputTexLevel2).rgb);
			#else
				vec3 cc2 = cc;
			#endif
			factor = normpdf3(cc2 - c, BSIGMA)*bZ*kernel[kSize + j] * kernel[kSize + i];
			Z += factor;
			final_colour += factor * cc;

		}
	}

	vec4 Output = vec4(final_colour / Z, 1.0);

#ifdef MODULATE_TEXTURE
	Output *= texture2D(ModulateTex, iTexCoords.xy);
#endif

	gl_FragColor = Output;
}
