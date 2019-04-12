struct Payload_s
{
	vec4 Accum;
#ifndef PATH_TRACING
	vec3 Strength;
	float Roughness;
	vec4 WorldPos;
#endif
};

Payload_s InitPayload()
{
	Payload_s Result;
#ifdef PATH_TRACING
	Result.Accum = vec4(1.0);
#else
	Result.Accum = vec4(0.0);
	Result.Strength = vec3(1.0);
	Result.Roughness = 0.0;
	Result.WorldPos = vec4(0.0);
#endif

	return Result;
}
