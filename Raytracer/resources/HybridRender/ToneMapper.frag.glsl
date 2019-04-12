

layout(location = 0) in vec2 iTexCoords;

#if NUM_AA_SAMPLES > 1
	layout(binding=0) uniform sampler2DMS InputTexture;
#else
	layout(binding=0) uniform sampler2D InputTexture;
#endif

layout(binding = 1) uniform sampler2D BloomTexture;
layout(binding = 2) uniform sampler2D OldPostProcessBuffer;
layout(binding = 3) uniform sampler2D VelocityBuffer;


float Uncharted2Tonemap_WhiteScale = 11.2;

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;

	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 ReinhardTonemap(vec3 x)
{
	return x / ( vec3(1.0) + x);
}

void main()
{
	ivec2 TC = ivec2(gl_FragCoord.xy);

	vec3 Accum = vec3(0.0);

#if NUM_AA_SAMPLES > 1
	
	for (int i = 0; i < NUM_AA_SAMPLES; i++)
	{
		Accum += texelFetch(InputTexture, TC, i).xyz;
	}

	Accum /= vec3(NUM_AA_SAMPLES);

#else

	Accum += texture2D(InputTexture, iTexCoords, 0).xyz;
	Accum += texture2D(BloomTexture, iTexCoords, 0).xyz;

#endif

#ifndef NO_TONEMAP
	Accum = Uncharted2Tonemap(Accum * 2.0) / Uncharted2Tonemap(Uncharted2Tonemap_WhiteScale.xxx);
	//Accum = ReinhardTonemap(Accum);
#endif

	//gamma correction
	Accum = pow(Accum, vec3(1.0 / 2.20));

	gl_FragColor = vec4(Accum, 1.0);
}
