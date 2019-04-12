
#define RAY_CLOSEST_HIT 0
#define RAY_ANY_HIT 1

#define RAY_MAIN RAY_CLOSEST_HIT
#define RAY_SHADOW RAY_ANY_HIT

#define RAY_BACKFACE_CULL 1
#define RAY_FRONTFACE_CULL 2

#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_CLOSEST_HIT 1
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_NORMAL 2
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_POSITION 4
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_MATERIAL 8
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_FACE_CULLING 16
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_ANY_HIT 32

#ifndef RAYTRACE_OPTIMIZATIONS
#define RAYTRACE_OPTIMIZATIONS 0
#endif

#define RAYTRACE_HAS_CLOSEST_HIT ((RAYTRACE_OPTIMIZATIONS & RAYTRACE_OPTIMIZATIONS_FLAG_NO_CLOSEST_HIT) == 0)
#define RAYTRACE_HAS_RESULT_NORMAL ((RAYTRACE_OPTIMIZATIONS & RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_NORMAL) == 0)
#define RAYTRACE_HAS_RESULT_POSITION ((RAYTRACE_OPTIMIZATIONS & RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_POSITION) == 0)
#define RAYTRACE_HAS_RESULT_MATERIAL ((RAYTRACE_OPTIMIZATIONS & RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_MATERIAL) == 0)
#define RAYTRACE_HAS_FACE_CULLING ((RAYTRACE_OPTIMIZATIONS & RAYTRACE_OPTIMIZATIONS_FLAG_NO_FACE_CULLING) == 0)
#define RAYTRACE_HAS_ANY_HIT ((RAYTRACE_OPTIMIZATIONS & RAYTRACE_OPTIMIZATIONS_FLAG_NO_ANY_HIT) == 0)

//////////////
struct RecursiveRay_s
{
	vec3 Origin;
	vec3 Direction;
};

struct BvhNode_s
{
	vec4 AABBMin;
	vec4 AABBMax;
};

struct Material_s
{
	lowp vec4 Color;
	lowp vec2 RoughnessMetallic;
	lowp vec2 EmissiveUnused;
	layout(bindless_sampler) sampler2D Albedo;
	layout(bindless_sampler) sampler2D Gloss;
};

struct Object_s
{
	vec3 AABBMin;
	vec3 AABBMax;
	mat4 Transform;
	mat4 InverseTransform;
	Material_s Material;
	ivec2 MeshShape;
};

struct Vertex_s
{
	vec4 Normal;
	vec4 TexCoord;
};

struct MeshShape_s
{
	BvhNode_s* BvhTree;
	vec4* Triangles;
	Vertex_s* Vertices;
};

struct Mesh_s
{
	MeshShape_s* Shapes;
};

struct Scene_s
{
    int NumObjects;
	Object_s* Objects;
	BvhNode_s* Tree;
};

struct RayResult_s
{
#if RAYTRACE_HAS_RESULT_POSITION
	vec3 Pos;
#endif
#if RAYTRACE_HAS_RESULT_NORMAL
	mediump vec3 Normal;
#endif
#if RAYTRACE_HAS_RESULT_MATERIAL
	vec2 TexCoord;
#endif
    float Distance;
};

struct Context_s
{
	RecursiveRay_s Ray;
	Payload_s Payload;
	RayResult_s Result;
#if RAYTRACE_HAS_RESULT_MATERIAL
	Material_s Material;
#endif
#if RAYTRACE_HAS_FACE_CULLING
	lowp float FaceCullingValue;
#endif
	bool bValidClosest, bValidAny;
};

RecursiveRay_s StopRay()
{
	RecursiveRay_s r;
	r.Origin = vec3(0.0);
	r.Direction = vec3(0.0);
	return r;
}

RecursiveRay_s NewRay(vec3 RayOrigin, vec3 RayDir)
{
	RecursiveRay_s r;
	r.Origin = RayOrigin;
	r.Direction = RayDir;
	return r;
}

RayResult_s InitRayResult(float MaxLength = 100000)
{
	RayResult_s r;
    r.Distance = MaxLength;
    return r;
}

uniform Scene_s Scene;
uniform Mesh_s* Meshes;

//implementable functions
bool AnyHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, inout Payload_s Payload);
RecursiveRay_s ClosestHit(RecursiveRay_s Ray, RayResult_s Result, Material_s Material, int RayType, int Depth, inout Payload_s Payload);
void Miss(RecursiveRay_s Ray, int RayType, inout Payload_s Payload);

