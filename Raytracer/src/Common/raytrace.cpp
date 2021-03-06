// RayTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "raytrace.h"

using namespace glm;

static int ScrambleAxis[3] = { 0,0,0 };

GpuBvhTree_s GenerateGpuBvhTree(const std::vector<BvhNode_s>& Tree)
{
	GpuBvhTree_s GpuTree;
	GpuTree.Size = Tree.size() + 1;
	GpuTree.Nodes = new GpuBvhNode_s[GpuTree.Size];
	
	for (size_t i = 0; i < Tree.size(); i++)
	{
		GpuTree.Nodes[i].AABBMin = vec4(Tree[i].AABBMin, *(float*)&Tree[i].ChildsNext.x);
		GpuTree.Nodes[i].AABBMax = vec4(Tree[i].AABBMax, *(float*)&Tree[i].ChildsNext.w);
	}

	//see intersection.glsl
	GpuTree.Nodes[GpuTree.Size-1] = GpuBvhNode_s{ vec4(0.0), vec4(0.0) };

	return std::move(GpuTree);
}

void GenerateStacklessData(std::vector<BvhNode_s>& Tree, int NodeIdx, int ParentIdx)
{
	BvhNode_s& Node = Tree[NodeIdx];
	if (Node.ChildsNext[0] > 0)
	{
		BvhNode_s& ChildA = Tree[Node.ChildsNext[0]];
		ChildA.ChildsNext[3] = Node.ChildsNext[1];
		GenerateStacklessData(Tree, Node.ChildsNext[0], NodeIdx);
	}

	if (Node.ChildsNext[1] > 0)
	{
		BvhNode_s& ChildB = Tree[Node.ChildsNext[1]];
		ChildB.ChildsNext[3] = Node.ChildsNext[3];
		GenerateStacklessData(Tree, Node.ChildsNext[1], NodeIdx);
	}
}

void CreateBvhChilds(std::vector<BvhNode_s>& Tree, BvhNode_s& Parent, const GpuObject_s* Objects, const std::vector<int>& ContainedObjects)
{
	//calculate parent bounding box based on contained objects
	vec3 PosMin = Parent.AABBMin = vec3(1.e+16f, 1.e+16f, 1.e+16f);
	vec3 PosMax = Parent.AABBMax = vec3(-1.e+16f, -1.e+16f, -1.e+16f);

	for (auto iObj : ContainedObjects)
	{
		Parent.AABBMin = glm::min(Parent.AABBMin, Objects[iObj].AABBMin);
		Parent.AABBMax = glm::max(Parent.AABBMax, Objects[iObj].AABBMax);

		PosMin = glm::min(PosMin, Objects[iObj].Pos());
		PosMax = glm::max(PosMax, Objects[iObj].Pos());
	}

	vec3 Abs = abs(Parent.AABBMin - Parent.AABBMax);
	for (int i = 0; i < 3; i++)
	{
		if (Abs[i] < 0.1f)
		{
			Parent.AABBMin[i] -= 0.05f;
			Parent.AABBMax[i] += 0.05f;
		}
	}


	//is leaf?
	if (ContainedObjects.size() == 1)
	{
		Parent.ChildsNext[0] = -(ContainedObjects[0] + 1);
		Parent.ChildsNext[1] = 0;
		return;
	}

	//get the biggest axis to divide the node
	const vec3 Bounds = PosMax - PosMin;
	const int BiggestAxis = (Bounds.x > Bounds.y && Bounds.x > Bounds.z) ? 0 : ((Bounds.y > Bounds.x && Bounds.y > Bounds.z) ? 1 : 2);
	float MidAxis = (PosMin[BiggestAxis] + PosMax[BiggestAxis]) / 2.f;

	ScrambleAxis[BiggestAxis] = !ScrambleAxis[BiggestAxis];

	//find the contained objects for each child
	std::vector<int> ChildObjects[2];
	for (auto iObj : ContainedObjects)
	{
		int childArray = Objects[iObj].Pos()[BiggestAxis] < MidAxis ? ScrambleAxis[BiggestAxis] : !ScrambleAxis[BiggestAxis];
		ChildObjects[childArray].push_back(iObj);
	}

	if (ChildObjects[0].size() == 0 && ChildObjects[1].size() >= 2)
	{
		ChildObjects[0].push_back(ChildObjects[1].back());
		ChildObjects[1].pop_back();
	}
	else if(ChildObjects[1].size() == 0 && ChildObjects[0].size() >= 2)
	{
		ChildObjects[1].push_back(ChildObjects[0].back());
		ChildObjects[0].pop_back();
	}

	//create the childs (nodes or leaves)
	for (int iChild = 0; iChild < 2; iChild++)
	{
		if (ChildObjects[iChild].size() >= 1)
		{
			Parent.ChildsNext[iChild] = (int)Tree.size();
			Tree.push_back(BvhNode_s());
			CreateBvhChilds(Tree, Tree.back(), Objects, ChildObjects[iChild]);
		}
		else
		{
			Parent.ChildsNext[iChild] = 0;
		}
	}
}

std::vector<BvhNode_s> CreateBvhTree(const int NumObjects, const GpuObject_s* Objects)
{
	std::vector<int> ObjectIndices;
	ObjectIndices.reserve(NumObjects);
	for (int iObj = 0; iObj < NumObjects; iObj++)
		ObjectIndices.push_back(iObj);

	std::vector<BvhNode_s> Tree;
	Tree.reserve(NumObjects * 2);
	Tree.push_back(BvhNode_s());

	memset(ScrambleAxis, 0, sizeof(ScrambleAxis));

	CreateBvhChilds(Tree, Tree.back(), Objects, ObjectIndices);

	GenerateStacklessData(Tree, 0, 0);

	return Tree;
}

void CreateMeshShapeBvhChilds(std::vector<BvhNode_s>& Tree, BvhNode_s& Parent, const Mesh* pMesh, int ShapeIdx, const std::vector<int>& ContainedPolygons)
{
	//calculate parent bounding box based on contained objects
	vec3 PosMin = Parent.AABBMin = vec3(1.e+16f, 1.e+16f, 1.e+16f);
	vec3 PosMax = Parent.AABBMax = vec3(-1.e+16f, -1.e+16f, -1.e+16f);

	const auto* pShape = &pMesh->GetShapes()[ShapeIdx];
	auto GetPolyCenter = [=](int iPoly) { return (pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 0]].Position + pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 1]].Position + pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 2]].Position) / 3.f; };
	for (auto iPoly : ContainedPolygons)
	{
		Mesh::Vertex_t verts[] = { pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 0]], pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 1]], pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 2]] };

		for (auto vert : verts)
		{
			Parent.AABBMin = glm::min(Parent.AABBMin, vert.Position);
			Parent.AABBMax = glm::max(Parent.AABBMax, vert.Position);
		}

		PosMin = glm::min(PosMin, GetPolyCenter(iPoly));
		PosMax = glm::max(PosMax, GetPolyCenter(iPoly));
	}
	
	vec3 Abs = abs(Parent.AABBMin - Parent.AABBMax);
	for (int i = 0; i < 3; i++)
	{
		if (Abs[i] < 0.1f)
		{
			Parent.AABBMin[i] -= 0.05f;
			Parent.AABBMax[i] += 0.05f;
		}
	}

	//is leaf?
	if (ContainedPolygons.size() <= 1)
	{
		Parent.ChildsNext.x = Parent.ChildsNext.y = Parent.ChildsNext.z = 0;
		for (GLsizei i = 0; i < (GLsizei)ContainedPolygons.size(); i++)
		{
			Parent.ChildsNext[i] = -(ContainedPolygons[i] + 1);
		}
		return;
	}

	//get the biggest axis to divide the node
#if 0 //SAH... slower for some reason
	auto GetPolyArea = [=](int iPoly) -> float
	{
		vec3 v0 = pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 0]].Position;
		vec3 v1 = pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 1]].Position;
		vec3 v2 = pMesh->GetVertices()[pShape->VertexIndices[iPoly * 3 + 2]].Position;

		float AB = length(v1 - v0);
		float C = dot(normalize(v1 - v0), normalize(v2 - v0));

		return 0.5*AB * sin(acos(C));
	};

	int BiggestAxis = 0;
	float BiggestAxisDiff = 1000000;
	float TriArea[3][2] = { {0,0}, {0,0}, {0,0} };
	float TriDiff[3] = { 0,0,0 };
	for (int iAxis = 0; iAxis < 3; iAxis++)
	{
		const float MidAxis = (PosMin[iAxis] + PosMax[iAxis]) / 2.f;
		for (auto iPoly : ContainedPolygons)
		{
			int childArray = GetPolyCenter(iPoly)[iAxis] < MidAxis ? 0 : 1;
			TriArea[iAxis][childArray] += GetPolyArea(iPoly);
		}

		TriDiff[iAxis] = fabsf(TriArea[iAxis][0] - TriArea[iAxis][1]);

		if (TriArea[iAxis][0] > 0 && TriArea[iAxis][1] > 0 && TriDiff[iAxis] < BiggestAxisDiff)
		{
			BiggestAxis = iAxis;
			BiggestAxisDiff = TriDiff[iAxis];
		}
	}
#else
	const vec3 Bounds = PosMax - PosMin;
	int BiggestAxis = (Bounds.x > Bounds.y && Bounds.x > Bounds.z) ? 0 : ((Bounds.y > Bounds.x && Bounds.y > Bounds.z) ? 1 : 2);
#endif
	const float MidAxis = (PosMin[BiggestAxis] + PosMax[BiggestAxis]) / 2.f;

	ScrambleAxis[BiggestAxis] = !ScrambleAxis[BiggestAxis];

	//find the contained objects for each child
	std::vector<int> ChildObjects[2];
	for (auto iPoly : ContainedPolygons)
	{
		int childArray = GetPolyCenter(iPoly)[BiggestAxis] < MidAxis ? ScrambleAxis[BiggestAxis] : !ScrambleAxis[BiggestAxis];
		ChildObjects[childArray].push_back(iPoly);
	}

	//hackfix
	if (ChildObjects[0].size() == 0 && ChildObjects[1].size() >= 2)
	{
		ChildObjects[0].push_back(ChildObjects[1].back());
		ChildObjects[1].pop_back();
	}
	else if (ChildObjects[1].size() == 0 && ChildObjects[0].size() >= 2)
	{
		ChildObjects[1].push_back(ChildObjects[0].back());
		ChildObjects[0].pop_back();
	}

	//create the childs (nodes or leaves)
	for (int iChild = 0; iChild < 2; iChild++)
	{
		if (ChildObjects[iChild].size() >= 1)
		{
			Parent.ChildsNext[iChild] = (int)Tree.size();
			Tree.push_back(BvhNode_s());
			CreateMeshShapeBvhChilds(Tree, Tree.back(), pMesh, ShapeIdx, ChildObjects[iChild]);
		}
		else
		{
			Parent.ChildsNext[iChild] = 0;
		}
	}
}

std::vector<BvhNode_s> CreateMeshShapeBvhTree(const Mesh* pMesh, int ShapeIdx)
{
	auto* pShape = &pMesh->GetShapes()[ShapeIdx];
	std::vector<int> ContainedPolygons;
	ContainedPolygons.reserve(pShape->VertexIndices.size()/3);
	for (size_t iObj = 0; iObj < pShape->VertexIndices.size() / 3; iObj++)
		ContainedPolygons.push_back((GLsizei)iObj);

	std::vector<BvhNode_s> Tree;
	Tree.reserve(ContainedPolygons.size() * 2);
	Tree.push_back(BvhNode_s());

	memset(ScrambleAxis, 0, sizeof(ScrambleAxis));

	CreateMeshShapeBvhChilds(Tree, Tree.back(), pMesh, ShapeIdx, ContainedPolygons);
	GenerateStacklessData(Tree, 0, 0);

	return Tree;
}

GpuMesh_s CreateGpuMesh(Mesh* pMesh)
{
	GpuMesh_s NewMesh;

	Buffer* buffer;

	const auto& Vertices = pMesh->GetVertices();

#if 0
	{
		std::vector<glm::vec4> AlignedVertices;
		AlignedVertices.reserve(Vertices.size()*2);
		for (auto vtx : Vertices)
		{
			AlignedVertices.emplace_back(vtx.Normal, 0.f);
			AlignedVertices.emplace_back(vtx.TexCoord.x, vtx.TexCoord.y, 0.f, 0.f);
		}

		buffer = pMesh->AddBuffer();
		buffer->UploadData(AlignedVertices.data(), sizeof(glm::vec4) * AlignedVertices.size());
		NewMesh.Vertices = buffer->GetBufferAddress();
	}
#endif

	std::vector<GpuMeshShape_s> Shapes;
	Shapes.resize(pMesh->GetShapes().size());
	for (size_t i = 0; i < Shapes.size(); i++)
	{
		const auto& VertexIndices = pMesh->GetShapes()[i].VertexIndices;
		{
			std::vector<glm::vec4> AlignedShapeVertices;

			for (auto it : VertexIndices)
				AlignedShapeVertices.push_back(vec4(Vertices[it].Position, 0));

			for (size_t i = 0; i < AlignedShapeVertices.size() / 3; i++)
			{
				vec3 A = vec3(AlignedShapeVertices[i * 3 + 0]);
				vec3 B = vec3(AlignedShapeVertices[i * 3 + 1]);
				vec3 C = vec3(AlignedShapeVertices[i * 3 + 2]);

				vec3 N = normalize(cross(B - A, C - A));

				AlignedShapeVertices[i * 3 + 0].w = N.x;
				AlignedShapeVertices[i * 3 + 1].w = N.y;
				AlignedShapeVertices[i * 3 + 2].w = N.z;
			}

			std::vector<glm::vec4> AlignedVertices;
			AlignedVertices.reserve(VertexIndices.size() * 3);
			for (size_t it = 0; it < VertexIndices.size(); it++ )
			{
				const auto& vtx = Vertices[VertexIndices[it]];
				AlignedVertices.emplace_back(AlignedShapeVertices[it]);
				AlignedVertices.emplace_back(vtx.Normal, 0.f);
				AlignedVertices.emplace_back(vtx.TexCoord.x, vtx.TexCoord.y, 0.f, 0.f);
				AlignedVertices.emplace_back();
			}

			buffer = pMesh->AddBuffer();
			buffer->UploadData(AlignedVertices.data(), sizeof(glm::vec4) * AlignedVertices.size());
			Shapes[i].Vertices = buffer->GetBufferAddress();
		}

		std::vector<glm::ivec4> AlignedIndices;
		AlignedIndices.reserve(VertexIndices.size());
		for (size_t idx = 0; idx < VertexIndices.size()/3; idx++)
			AlignedIndices.emplace_back(VertexIndices[idx * 3 + 0], VertexIndices[idx * 3 + 1], VertexIndices[idx * 3 + 2], 0);

#if 0
		buffer = pMesh->AddBuffer();
		buffer->UploadData(AlignedIndices.data(), sizeof(glm::ivec4) * AlignedIndices.size());
		Shapes[i].Polygons = buffer->GetBufferAddress();
#endif

		auto BvhTree = CreateMeshShapeBvhTree(pMesh, (GLsizei)i);
		auto GpuTree = GenerateGpuBvhTree(BvhTree);
		buffer = pMesh->AddBuffer();
		buffer->UploadData(GpuTree.Nodes, sizeof(GpuBvhNode_s) * GpuTree.Size);

		Shapes[i].BvhTree = buffer->GetBufferAddress();
	}

	buffer = pMesh->AddBuffer();
	buffer->UploadData(Shapes.data(), sizeof(GpuMeshShape_s) * Shapes.size());
	NewMesh.Shapes = buffer->GetBufferAddress();

	return NewMesh;
}

void TransformAABB(vec3& Mins, vec3& Maxs, const mat4& Transform)
{
	vec3 Points[8] =
	{
		vec3(Mins.x, Mins.y, Mins.z),
		vec3(Mins.x, Mins.y, Maxs.z),
		vec3(Mins.x, Maxs.y, Mins.z),
		vec3(Mins.x, Maxs.y, Maxs.z),
		vec3(Maxs.x, Mins.y, Mins.z),
		vec3(Maxs.x, Mins.y, Maxs.z),
		vec3(Maxs.x, Maxs.y, Mins.z),
		vec3(Maxs.x, Maxs.y, Maxs.z),
	};

	for (auto& It : Points)
	{
		It = vec3(Transform * vec4(It, 1.f));
	}

	Mins = vec3(1.e+16f, 1.e+16f, 1.e+16f);
	Maxs = vec3(-1.e+16f, -1.e+16f, -1.e+16f);

	for (auto& It : Points)
	{
		Mins = glm::min(Mins, It);
		Maxs = glm::max(Maxs, It);
	}
}

void CompileRaytraceShader(const std::string& Payload, const std::string& RayGenAndHit, Shader& OutShader, int32 OptimizationFlags, int ComputeLocalDepthPower)
{
	static const std::string CommonPath("../../resources/Common/raytrace/");

	std::vector<std::string> RaytraceFiles;

	RaytraceFiles.push_back(Payload);
	RaytraceFiles.push_back(CommonPath + "types");
	RaytraceFiles.push_back(CommonPath + "misc");
	RaytraceFiles.push_back(CommonPath + "lighting");
	RaytraceFiles.push_back(CommonPath + "intersection");
	RaytraceFiles.push_back(CommonPath + "raytrace");

	std::string OptimizationFlagsString = "RAYTRACE_OPTIMIZATIONS (0 | ";
	if (OptimizationFlags & RAYTRACE_OPTIMIZATIONS_FLAG_NO_CLOSEST_HIT)
		OptimizationFlagsString += "RAYTRACE_OPTIMIZATIONS_FLAG_NO_CLOSEST_HIT | ";
	if (OptimizationFlags & RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_NORMAL)
		OptimizationFlagsString += "RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_NORMAL | ";
	if (OptimizationFlags & RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_POSITION)
		OptimizationFlagsString += "RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_POSITION | ";
	if (OptimizationFlags & RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_MATERIAL)
		OptimizationFlagsString += "RAYTRACE_OPTIMIZATIONS_FLAG_NO_RESULT_MATERIAL | ";
	if (OptimizationFlags & RAYTRACE_OPTIMIZATIONS_FLAG_NO_FACE_CULLING)
		OptimizationFlagsString += "RAYTRACE_OPTIMIZATIONS_FLAG_NO_FACE_CULLING | ";
	if (OptimizationFlags & RAYTRACE_OPTIMIZATIONS_FLAG_NO_ANY_HIT)
		OptimizationFlagsString += "RAYTRACE_OPTIMIZATIONS_FLAG_NO_ANY_HIT | ";	
	OptimizationFlagsString += "0)";

	OutShader.SetComputeLocalSize(8 / ComputeLocalDepthPower, 8 / ComputeLocalDepthPower, (int)std::pow(2, ComputeLocalDepthPower - 1));
	OutShader.CompileFiles(RayGenAndHit, RaytraceFiles);
}
