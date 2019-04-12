#define SIGMA 20.0
#define BSIGMA 15.0
#define MSIZE 30

layout(location = 0) in vec2 iTexCoords;

uniform sampler2D InputNormal;
uniform sampler2D InputTex;
uniform sampler2D ModulateTex;

uniform int InputTexLevel = 0;
uniform vec2 iResolution;

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v, v) / (sigma*sigma)) / sigma;
}

float normpdf4(in vec4 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v, v) / (sigma*sigma)) / sigma;
}

vec4 Tonemap(vec4 x)
{
	return x;
}

void main()
{
	vec4 c = Tonemap(texture(InputTex, iTexCoords, InputTexLevel).rgba);

	//declare stuff
	const int kSize = (MSIZE - 1) / 2;
	float kernel[MSIZE];
	vec4 final_colour = vec4(0.0);

	//create the 1-D kernel
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), SIGMA);
	}

	float bZ = 1.0 / normpdf(0.0, BSIGMA);

	float Z = 0.0;
	vec4 cc = vec4(0.0);
	float factor = 0.f;

	//read out the texels
	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			cc = Tonemap(texture2D(InputTex, iTexCoords.xy + (vec2(float(i), float(j))) / iResolution.xy, InputTexLevel).rgba);
			factor = normpdf4(cc - c, BSIGMA)*bZ*kernel[kSize + j] * kernel[kSize + i];
			Z += factor;
			final_colour += factor * cc;

		}
	}

	vec4 Output = vec4(final_colour / Z);

	Z = 0.0;
	cc = vec4(0.0);
	factor = 0.f;
	vec3 final_normal = vec3(0.0);
	//read out the texels
	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			cc = Tonemap(texture2D(InputNormal, iTexCoords.xy + (vec2(float(i), float(j))) / iResolution.xy, 0).rgba);
			factor = normpdf3(cc.xyz - c.xyz, BSIGMA)*bZ*kernel[kSize + j] * kernel[kSize + i];
			Z += factor;
			final_normal += factor * cc.xyz;

		}
	}

	vec3 Normal = vec3(final_normal / Z);

	Output.xyz *= vec3(dot(texture2D(InputNormal, iTexCoords.xy, 0).xyz * 2 - 1, Normal.xyz * 2 - 1));
	Output.xyz *= texture2D(ModulateTex, iTexCoords.xy).xyz;

	//Output.xyz = Normal.xyz;

	gl_FragColor = Output; // vec4(dot(texture2D(InputNormal, iTexCoords.xy, 0).xyz * 2 - 1, texture2D(InputNormal, iTexCoords.xy, 2).xyz * 2 - 1));
}
