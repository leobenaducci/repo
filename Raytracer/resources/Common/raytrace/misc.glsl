#define PI (3.14159265358979323846)

float rcp(float a) { return 1.0 / a; }
vec2 rcp(vec2 a) { return vec2(1.0) / a; }
vec3 rcp(vec3 a) { return vec3(1.0) / a; }

void swap(inout int a, inout int b) { int c = a; a = b; b = c; }
void swap(inout float a, inout float b) { float c = a; a = b; b = c; }
void swap(inout vec2 a, inout vec2 b) { vec2 c = a; a = b; b = c; }
void swap(inout vec3 a, inout vec3 b) { vec3 c = a; a = b; b = c; }

float rand(float co)
{
	return fract(52.9829189f * (fract(floor(co * 5) * 0.06711056f + floor(co * 3) * 0.00583715f))) * 2 - 1;
}

vec2 rand(vec2 co)
{
	return vec2(rand(co.x-co.y*2), rand(co.y+co.x*0.5));
}

vec3 rand(vec3 co)
{
	return vec3(rand(co.x*3-co.z+co.y*2), rand(co.y+co.z*2+co.x*15), rand(co.z+co.y*1.3+co.x*0.7));
}

vec3 rand3(float Frame)
{
	return rand(vec3((gl_GlobalInvocationID.xyz + gl_LocalInvocationID.xyz * 2) ^ ivec3(Frame)));
}

vec3 randSphere(float fFrame)
{
	vec2 Invok = (gl_GlobalInvocationID.xy + gl_LocalInvocationID.xy * gl_GlobalInvocationID.z) * vec2(200000, 300000);
	vec2 Frame = vec2(1.5*fFrame, 3.1415*fFrame);
	vec2 Polar = (Invok + Frame);
	vec3 result;

	result.x = sin(Polar.x) * cos(Polar.y);
	result.y = sin(Polar.x) * sin(Polar.y);
	result.z = cos(Polar.x);

	return result;
}

vec3 reflectN(vec3 rV, vec3 N)
{
	return reflect(normalize(rV), normalize(N));
}

float schlick_refract(float cosine, float ref_idx)
{
    float r0 = (1.0-ref_idx) / (1.0+ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0-r0)*pow(1.0-cosine, 5);
}

bool refract(vec3 v, vec3 n, float ior, out vec3 refracted)
{
    float dt = dot(v, n);
    float discriminant = 1.0 - ior*ior *(1.0-dt*dt);
    if(discriminant <= 0.0)
        return false;
    refracted = ior * (v - n * dt) - n*sqrt(discriminant);
    return true;
}

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;

	return ((x*(A*x + C * B) + D * E) / (x*(A*x + B) + D * F)) - E / F;
}

void CopyTempResult(inout Context_s Context, RayResult_s Temp, Object_s Obj)
{
	Context.Result = Temp;
#if RAYTRACE_HAS_RESULT_MATERIAL
	Context.Material = Obj.Material;
#endif
}

#if RAYTRACE_HAS_RESULT_MATERIAL
void SampleTextures(inout Context_s Context)
{
	if (Context.bValidClosest || Context.bValidAny)
	{
		Context.Material.Color *= texture2D(Context.Material.Albedo, Context.Result.TexCoord);
		Context.Material.RoughnessMetallic *= texture2D(Context.Material.Gloss, Context.Result.TexCoord).xy;
	}
}
#endif