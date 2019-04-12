#include "stdafx.h"
#include "mesh.h"
#include "raytrace.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#if WIN64
#pragma comment(lib, "assimp/lib/x64/IrrXML64.lib")
#pragma comment(lib, "assimp/lib/x64/zlibstatic64.lib")
#pragma comment(lib, "assimp/lib/x64/assimp-vc140-mt.lib")
#else
#pragma comment(lib, "assimp/lib/IrrXML.lib")
#pragma comment(lib, "assimp/lib/zlibstatic.lib")
#pragma comment(lib, "assimp/lib/assimp-vc140-mt.lib")
#endif

std::map<std::string, RaytraceMesh*> RaytraceMesh::Meshes;

Mesh::Mesh()
	: VertexBuffer(nullptr)
{

}


Mesh::~Mesh()
{
}

size_t Mesh::FindOrAddVertex(glm::vec3 Position, glm::vec3 Normal, glm::vec2 TexCoord, glm::vec4 Tangent)
{
	Vertex_t NewVtx{ Position,Normal,TexCoord,Tangent };

	//auto it = std::find(Vertices.begin(), Vertices.end(), NewVtx);
	//if (it != Vertices.end())
	//{
	//	return (int32_t)(it - Vertices.begin());
	//}

	Vertices.push_back(NewVtx);
	return Vertices.size() - 1;
}

bool Mesh::LoadMesh(const std::string& FileName, Mesh& NewMesh, float Scale, bool bMergeShapes, bool bRemoveLastShape)
{
	Assimp::Importer Importer;
	const aiScene* pScene = Importer.ReadFile(FileName, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices);

	if (pScene == nullptr || !pScene->HasMeshes())
		return false;

	NewMesh.Name = FileName;

	NewMesh.AABB[0] = glm::vec3(1.e+6f, 1.e+6f, 1.e+6f);
	NewMesh.AABB[1] = glm::vec3(-1.e+6f, -1.e+6f, -1.e+6f);

	NewMesh.Shapes.reserve(pScene->mNumMeshes);
	for(uint32 iMesh = 0; iMesh < pScene->mNumMeshes; iMesh++)
	{
		Shape_s NewShape;

		const aiMesh* pMesh = pScene->mMeshes[iMesh];

		NewShape.FirstVertex = NewMesh.GetVertices().size();
		NewShape.VertexIndices.reserve(pMesh->mNumFaces * 3);
		for (size_t iFace = 0; iFace < pMesh->mNumFaces; iFace++)
		{
			aiFace* pFace = &pMesh->mFaces[iFace];
			for (int iTri = 0; iTri < 3; iTri++)
			{
				uint32 iIdx = pFace->mIndices[iTri];

				glm::vec3 vtx = glm::vec3(pMesh->mVertices[iIdx].x, pMesh->mVertices[iIdx].y, pMesh->mVertices[iIdx].z) * Scale;
				glm::vec3 nrm = pMesh->HasNormals() ? glm::vec3(pMesh->mNormals[iIdx].x, pMesh->mNormals[iIdx].y, pMesh->mNormals[iIdx].z) : glm::vec3(0.f);
				glm::vec2 tc = pMesh->HasTextureCoords(0) ? glm::vec2(pMesh->mTextureCoords[0][iIdx].x, pMesh->mTextureCoords[0][iIdx].y) : glm::vec2(0.f);
				glm::vec4 tn = pMesh->HasTangentsAndBitangents() ? glm::vec4(pMesh->mTangents[iIdx].x, pMesh->mTangents[iIdx].y, pMesh->mTangents[iIdx].z, 1.f) : glm::vec4(0.f);
				glm::vec3 bn = pMesh->HasTangentsAndBitangents() ? glm::vec3(pMesh->mBitangents[iIdx].x, pMesh->mBitangents[iIdx].y, pMesh->mBitangents[iIdx].z) : glm::vec3(0.f);

				tn.w = glm::dot(glm::cross(nrm, glm::vec3(tn)), bn) >= 0.f ? 1.f : -1.f;

				int NewVertexIndex = NewMesh.FindOrAddVertex(vtx, nrm, tc, tn);
				NewShape.VertexIndices.push_back(NewVertexIndex);
			}
		}

		NewMesh.Shapes.push_back(NewShape);

		Material_s NewMat;
		if (pMesh->mMaterialIndex >= 0)
		{
			aiMaterial* pMat = pScene->mMaterials[pMesh->mMaterialIndex];
			aiString Diffuse;
			pMat->GetTexture(aiTextureType_DIFFUSE, 0, &Diffuse);
			NewMat.AlbedoMap = Diffuse.C_Str();

			aiString Normal;
			pMat->GetTexture(aiTextureType_NORMALS, 0, &Normal);
			if(Normal.length == 0)
				pMat->GetTexture(aiTextureType_HEIGHT, 0, &Normal);
			NewMat.NormalMap = Normal.C_Str();

			aiColor3D Color;
			pMat->Get(AI_MATKEY_COLOR_DIFFUSE, Color);
			NewMat.Diffuse.x = Color.r; NewMat.Diffuse.y = Color.g; NewMat.Diffuse.z = Color.b;
			pMat->Get(AI_MATKEY_OPACITY, NewMat.Diffuse.w);

			pMat->Get(AI_MATKEY_COLOR_SPECULAR, Color);
			NewMat.Specular.x = Color.r; NewMat.Specular.y = Color.g; NewMat.Specular.z = Color.b;

			pMat->Get("$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0, NewMat.Metallic);
			pMat->Get("$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0, NewMat.Roughness);

			pMat->Get(AI_MATKEY_COLOR_EMISSIVE, Color);
			NewMat.Emissive = (Color.r + Color.g + Color.b) * .334f;

//#define AI_MATKEY_NAME "?mat.name",0,0
//#define AI_MATKEY_TWOSIDED "$mat.twosided",0,0
//#define AI_MATKEY_SHADING_MODEL "$mat.shadingm",0,0
//#define AI_MATKEY_ENABLE_WIREFRAME "$mat.wireframe",0,0
//#define AI_MATKEY_BLEND_FUNC "$mat.blend",0,0
//#define AI_MATKEY_OPACITY "$mat.opacity",0,0
//#define AI_MATKEY_BUMPSCALING "$mat.bumpscaling",0,0
//#define AI_MATKEY_SHININESS "$mat.shininess",0,0
//#define AI_MATKEY_REFLECTIVITY "$mat.reflectivity",0,0
//#define AI_MATKEY_SHININESS_STRENGTH "$mat.shinpercent",0,0
//#define AI_MATKEY_REFRACTI "$mat.refracti",0,0
//#define AI_MATKEY_COLOR_DIFFUSE "$clr.diffuse",0,0
//#define AI_MATKEY_COLOR_AMBIENT "$clr.ambient",0,0
//#define AI_MATKEY_COLOR_SPECULAR "$clr.specular",0,0
//#define AI_MATKEY_COLOR_EMISSIVE "$clr.emissive",0,0
//#define AI_MATKEY_COLOR_TRANSPARENT "$clr.transparent",0,0
//#define AI_MATKEY_COLOR_REFLECTIVE "$clr.reflective",0,0
//#define AI_MATKEY_GLOBAL_BACKGROUND_IMAGE "?bg.global",0,0


			//$clr.diffuse
			//$clr.specular
			//$clr.shininess
			//$clr.opacity
			//tex.file
		}
		NewMesh.Materials.push_back(NewMat);
	}

	if (bRemoveLastShape)
		NewMesh.PopShape();
	if (bMergeShapes)
		NewMesh.MergeShapes();
	NewMesh.CalcBounds();

	NewMesh.OnLoaded();

	return true;
}

bool Mesh::CreateMesh(const std::string& InName, const std::vector<Mesh::Vertex_t>& vertices, Mesh& NewMesh)
{
	NewMesh.Name = InName;
	NewMesh.AABB[0] = glm::vec3(1.e+6f, 1.e+6f, 1.e+6f);
	NewMesh.AABB[1] = glm::vec3(-1.e+6f, -1.e+6f, -1.e+6f);

	NewMesh.Shapes.emplace_back();
	Shape_s& NewShape = NewMesh.Shapes.back();

	std::vector<int> indices;
	indices.reserve(vertices.size());
	for (GLsizei i = 0; i < (GLsizei)vertices.size(); i++)
	{
		indices.push_back(i);
	}

	std::vector<glm::vec3> Normals;
	Normals.reserve(indices.size());
	for (size_t index = 0; index < indices.size(); index += 3)
	{
		int iv;
		iv = indices[index + 0]; glm::vec3 vtx0 = vertices[iv].Position;
		iv = indices[index + 1]; glm::vec3 vtx1 = vertices[iv].Position;
		iv = indices[index + 2]; glm::vec3 vtx2 = vertices[iv].Position;

		glm::vec3 nrm = glm::normalize(glm::cross(vtx1 - vtx0, vtx2 - vtx0));

		Normals.push_back(nrm);
		Normals.push_back(nrm);
		Normals.push_back(nrm);
	}

	NewShape.VertexIndices.reserve(indices.size());
	for (size_t idx = 0; idx < indices.size(); idx++)
	{
		const auto& index = indices[idx];
		int iv = index;

		glm::vec3 vtx = vertices[iv].Position;
		glm::vec3 nrm = glm::length(vertices[iv].Normal) > 0.01f ? vertices[iv].Normal : Normals[idx];
		glm::vec2 tc = vertices[iv].TexCoord;

		int NewVertexIndex = NewMesh.FindOrAddVertex(vtx, nrm, tc, glm::vec4(1,0,0,1));
		NewShape.VertexIndices.push_back(NewVertexIndex);
	}

	NewMesh.CalcBounds();

	NewMesh.OnLoaded();

	return true;
}

void Mesh::MergeShapes()
{
	Shape_s NewShape;
	int TotalIndices = 0;

	for(auto& shape : Shapes)
		TotalIndices += (GLsizei)shape.VertexIndices.size();

	NewShape.VertexIndices.reserve(TotalIndices);
	for (auto& shape : Shapes)
	{
		for (auto& it : shape.VertexIndices)
		{
			NewShape.VertexIndices.push_back(it);
		}
	}

	Shapes.resize(0);
	Shapes.push_back(NewShape);

	Materials.resize(1);

	CalcBounds();
}

void Mesh::CalcBounds()
{
	AABB[0] = glm::vec3(1.e+6f, 1.e+6f, 1.e+6f);
	AABB[1] = glm::vec3(-1.e+6f, -1.e+6f, -1.e+6f);

	for (auto& shape : Shapes)
	{
		shape.AABB[0] = glm::vec3(1.e+6f, 1.e+6f, 1.e+6f);
		shape.AABB[1] = glm::vec3(-1.e+6f, -1.e+6f, -1.e+6f);

		for (auto idx : shape.VertexIndices)
		{
			shape.AABB[0] = glm::min(shape.AABB[0], Vertices[idx].Position);
			shape.AABB[1] = glm::max(shape.AABB[1], Vertices[idx].Position);
		}

		AABB[0] = glm::min(AABB[0], shape.AABB[0]);
		AABB[1] = glm::max(AABB[1], shape.AABB[1]);
	}
}

#pragma optimize("", off)
void Mesh::OnLoaded()
{
	VertexBuffer = AddBuffer();

	VertexBuffer->UploadData(GetVertices().data(), GetVertices().size() * sizeof(Vertex_t));
}

void RaytraceMesh::OnLoaded()
{
	Mesh::OnLoaded();

	GpuMesh = CreateGpuMesh(this);
	Meshes[Name] = this; 
}

#pragma optimize("", on)