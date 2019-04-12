vec3 CookTorrance_BRDF(vec3 N, vec3 L, vec3 V, vec3 BaseColor, float Metallic, float Roughness, float DiffFactor, float SpecFactor);

vec3 CookTorrancePoint(vec3 Pos, vec3 CamPos, vec3 lPos, vec4 NormalMetallic, vec4 ColorRoughness, float DiffFactor = 1.f, float SpecFactor = 1.f)
{
	vec3 EyeDir = normalize((CamPos - Pos) / 1000.0);
	vec3 lDir = normalize(lPos - Pos);

	return CookTorrance_BRDF(normalize(NormalMetallic.xyz), lDir, EyeDir, ColorRoughness.xyz, NormalMetallic.w, ColorRoughness.w, DiffFactor, SpecFactor);
}

vec3 CookTorranceDir(vec3 LightDir, vec3 Pos, vec3 CamPos, vec4 NormalMetallic, vec4 ColorRoughness, float DiffFactor = 1.f, float SpecFactor = 1.f)
{
	vec3 EyeDir = normalize(CamPos - Pos);

	return CookTorrance_BRDF(normalize(NormalMetallic.xyz), -LightDir, EyeDir, ColorRoughness.xyz, NormalMetallic.w, ColorRoughness.w, DiffFactor, SpecFactor);
}

vec3 F_Schlick(vec3 SpecColor, float u)
{
	float m = clamp(1.0 - u, 0.0, 1.0);
	float m2 = m * m;
	float Fc = m2 * m2*m;

	return SpecColor + (1.0 - Fc) * SpecColor;
}

float D_BlinnPhong(float NoH, float R)
{
	float SpecPow, D;

	//	R = R*R;

	SpecPow = exp2(10.0 * (1.0 - R) + 1.0); // 2.0 / R - 2;

	D = max(0.0, NoH);
	D = pow(D, SpecPow) * (SpecPow + 2.0) / (PI * SpecPow);

	return D;
}

float D_GGX(float NoH, float R)
{
	R = R * R;
	float NH = max(0.0, NoH);
	float D = (NH*NH*(R*R - 1.0) + 1.0);

	return (R*R) / (PI*D*D);
}

vec3 CookTorrance_BRDF(vec3 N, vec3 L, vec3 V, vec3 BaseColor, float Metallic, float Roughness, float DiffFactor, float SpecFactor)
{
	vec3 SpecColor = mix(vec3(0.004), BaseColor, Metallic);
	vec3 H = normalize(V + L);

	float D, G;
	vec3 F = vec3(1.0);
	float Diff;

	//D = D_BlinnPhong(dot(N, H), Roughness);
	D = D_GGX(dot(N, H), Roughness);

	F = F_Schlick(SpecColor, dot(N, H));

	G = 1.0;

	Diff = (max(0.0, dot(N, L)) / PI);

	return SpecColor * F * max(D * G, 0.0) * SpecFactor + BaseColor * Diff * (1.0f - Metallic) * DiffFactor;
}
