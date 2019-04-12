struct Payload_s
{
    vec4 Accum;
    vec3 Strength;
	float Roughness;
	vec4 WorldPos;
};

Payload_s InitPayload()
{
    Payload_s Result;
    Result.Accum = vec4(1.0);
    Result.Strength = vec3(1.0);
	Result.Roughness = 0.0;
	Result.WorldPos = vec4(0.0);

    return Result;
}
