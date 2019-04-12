

layout(location = 0) in vec2 iTexCoords;

#if NUM_AA_SAMPLES > 1
	layout(binding=0) uniform sampler2DMS InputLightBuffer;
	layout(binding=1) uniform sampler2DMS InputNormal;
	layout(binding=2) uniform sampler2DMS InputColorRoughness;
	layout(binding=3) uniform sampler2DMS InputDepth;
	layout(binding=4) uniform sampler2DMS InputVelocities;
#else
	layout(binding = 0) uniform sampler2D InputLightBuffer;
	layout(binding = 1) uniform sampler2D InputNormal;
	layout(binding = 2) uniform sampler2D InputColorRoughness;
	layout(binding = 3) uniform sampler2D InputDepth;
	layout(binding = 4) uniform sampler2D InputVelocities;
#endif

void main()
{
	ivec2 TC = ivec2(gl_FragCoord.xy);

	vec4 Accum[4]= {vec4(0.0), vec4(0.0), vec4(0.0), vec4(0.0)};

	for (int i = 0; i < NUM_AA_SAMPLES; i++)
	{
		Accum[0] += texelFetch(InputNormal, TC, i);

		Accum[1].xyz += texelFetch(InputLightBuffer, TC, i).xyz;
		Accum[1].w += texelFetch(InputColorRoughness, TC, i).w;

		Accum[2] += texelFetch(InputDepth, TC, i);
		Accum[3] += texelFetch(InputVelocities, TC, i);
	}

	Accum[0] /= vec4(NUM_AA_SAMPLES);
	Accum[1] /= vec4(NUM_AA_SAMPLES);
	Accum[2] /= vec4(NUM_AA_SAMPLES);
	Accum[3] /= vec4(NUM_AA_SAMPLES);

	gl_FragData[0] = Accum[0];
	gl_FragData[1] = Accum[1];
	gl_FragData[2] = Accum[2];
	gl_FragData[3] = Accum[3];
}
