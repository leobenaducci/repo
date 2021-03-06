// RayTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "mesh.h"

#pragma once

using namespace glm;

#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_CLOSEST_HIT 1
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_NORMAL 2
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_POSITION 4
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_MATERIAL 8
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_FACE_CULLING 16
#define RAYTRACE_OPTIMIZATIONS_FLAG_NO_ANY_HIT 32

constexpr size_t GLSL_ALIGN(size_t s)
{
	if (s <= 2) return 2;
	else if (s <= 4) return 4;
	else if (s <= 8) return 8;
	else if (s <= 16) return 16;
}

#define GLSL_TYPE(x) __declspec(align(GLSL_ALIGN(sizeof(x)))) x
#define GLSL_POINTER(x) __declspec(align(8)) x

struct BvhNode_s
{
	GLSL_TYPE(vec3) AABBMin;
	GLSL_TYPE(vec3) AABBMax;
	GLSL_TYPE(ivec4) ChildsNext;
	GLSL_TYPE(vec4) Pad;
};

struct GpuBvhNode_s
{
	GLSL_TYPE(vec4) AABBMin;
	GLSL_TYPE(vec4) AABBMax;
};

struct GpuBvhTree_s
{
	size_t Size;
	GpuBvhNode_s* Nodes;

	GpuBvhTree_s() : Size(0), Nodes(nullptr) {}
	GpuBvhTree_s(const GpuBvhTree_s& Other) : Size(Other.Size) { Nodes = new GpuBvhNode_s[Size]; memcpy(Nodes, Other.Nodes, sizeof(GpuBvhNode_s) * Size); }
	GpuBvhTree_s(GpuBvhTree_s&& Other) { Size = Other.Size; Nodes = Other.Nodes; Other.Size = 0; Other.Nodes = nullptr; }
	~GpuBvhTree_s() { delete[] Nodes; }
};

struct GpuObject_s
{
	GLSL_TYPE(vec3) AABBMin;
	GLSL_TYPE(vec3) AABBMax;
	GLSL_TYPE(vec4) Transform[4];
	GLSL_TYPE(vec4) InverseTransform[4];	
	//Material_s {
	GLSL_TYPE(vec4) Color;
	GLSL_TYPE(vec2) RoughnessMetallic;
	GLSL_TYPE(vec2) EmissiveUnused;
	GLSL_TYPE(GLuint64EXT) Albedo;
	GLSL_TYPE(GLuint64EXT) Gloss;
	//}
	GLSL_TYPE(ivec2) MeshShape;

	GpuObject_s() : MeshShape(0, 0)
	{
		SetTransform(vec3(0.f), vec3(0.f), vec3(1.f));
	}

	void SetTransform(vec3 Pos, vec3 Rot, vec3 Scale = vec3(1.f))
	{
		static glm::mat4 IdentityMatrix4(1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f);

		mat4 WorldTransform;
		WorldTransform = glm::translate(IdentityMatrix4, Pos);
		WorldTransform = glm::rotate(WorldTransform, glm::radians(Rot.z), glm::vec3(0.f, 0.f, 1.f));
		WorldTransform = glm::rotate(WorldTransform, glm::radians(Rot.y), glm::vec3(1.f, 0.f, 0.f));
		WorldTransform = glm::rotate(WorldTransform, glm::radians(Rot.x), glm::vec3(0.f, 1.f, 0.f));
		WorldTransform = glm::scale(WorldTransform, Scale);
		memcpy(&Transform[0], &WorldTransform[0], sizeof(vec4) * 4);

		mat4 InvWorldTransform = transpose(inverse(WorldTransform));
		memcpy(&InverseTransform[0], &InvWorldTransform[0], sizeof(vec4) * 4);
	}

	vec3 Pos() const
	{
		return vec3(Transform[3].x, Transform[3].y, Transform[3].z);
	}

	mat4& GetTransformRef()
	{
		return *(mat4*)&Transform[0];
	}
};

struct GpuMeshShape_s
{
	GLSL_TYPE(GLuint64EXT) BvhTree;
	GLSL_TYPE(GLuint64EXT) Vertices;
};

struct GpuMesh_s
{
	GLSL_TYPE(GLuint64EXT) Shapes;
	bool operator==(const GpuMesh_s& B) const { return Shapes == B.Shapes; }
};

struct RaytraceScene_s
{
	GLSL_TYPE(int) NumObjects;
	GLSL_TYPE(GLuint64EXT) Objects;
	GLSL_TYPE(GLuint64EXT) BvhRoot;
};

class RaytraceMesh : public Mesh
{
public:
	RaytraceMesh() : Mesh() {}
	RaytraceMesh(const RaytraceMesh&) = delete;
	RaytraceMesh(const RaytraceMesh&&) = delete;
	virtual ~RaytraceMesh() { Meshes[Name] = nullptr; }

	virtual void OnLoaded();

	GpuMesh_s GpuMesh;

	static std::map<std::string, RaytraceMesh*> Meshes;
	static RaytraceMesh* FindMesh(const std::string& Name) { return Meshes[Name]; }
};

GpuBvhTree_s GenerateGpuBvhTree(const std::vector<BvhNode_s>& Tree);

void GenerateStacklessData(std::vector<BvhNode_s>& Tree, int NodeIdx, int ParentIdx);

void CreateBvhChilds(std::vector<BvhNode_s>& Tree, BvhNode_s& Parent, const GpuObject_s* Objects, const std::vector<int>& ContainedObjects);
std::vector<BvhNode_s> CreateBvhTree(const int NumObjects, const GpuObject_s* Objects);

void CreateMeshShapeBvhChilds(std::vector<BvhNode_s>& Tree, BvhNode_s& Parent, const Mesh* pMesh, int ShapeIdx, const std::vector<int>& ContainedPolygons);
std::vector<BvhNode_s> CreateMeshShapeBvhTree(const Mesh* pMesh, int ShapeIdx);
GpuMesh_s CreateGpuMesh(Mesh* pMesh);

void TransformAABB(vec3& Mins, vec3& Maxs, const mat4& Transform);

void CompileRaytraceShader(const std::string& Payload, const std::string& RayGenAndHit, Shader& OutShader, int32 OptimizationFlags = 0, int ComputeLocalDepthPower = 1);

