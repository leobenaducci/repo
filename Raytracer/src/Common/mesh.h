#pragma once

#include "buffer.h"

class Mesh
{
public:
	struct Shape_s
	{
		std::vector<int32_t> VertexIndices;
		size_t FirstVertex;
		glm::vec3 AABB[2];
	};

	struct Material_s
	{
		std::string Name;
		glm::vec4 Diffuse = glm::vec4(1.f);
		glm::vec4 Specular = glm::vec4(1.f);
		float Metallic = 0.f;
		float Roughness = 1.f;
		float Emissive = 0.f;
		std::string AlbedoMap;
		std::string GlossMap;
		std::string NormalMap;		
	};

	std::vector<Material_s> Materials;

	struct Vertex_t
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec4 Tangent;

		bool operator==(const Vertex_t& A) const { return Position == A.Position && Normal == A.Normal && TexCoord == A.TexCoord && Tangent == A.Tangent; };
	};

	Mesh();
	Mesh(const Mesh&) = delete;
	Mesh(const Mesh&&) = delete;
	~Mesh();

	const std::vector<Shape_s>& GetShapes() const { return Shapes; }
	const std::vector<Vertex_t>& GetVertices() const { return Vertices; }
	
	void GetAABB(int Shape, glm::vec3& AABBMin, glm::vec3& AABBMax) const { AABBMin = Shapes[Shape].AABB[0]; AABBMax = Shapes[Shape].AABB[1]; }

	std::string GetMaterialAlbedo(int Shape) const { return Materials[Shape].AlbedoMap; }
	std::string GetMaterialNormalMap(int Shape) const { return Materials[Shape].NormalMap; }
	glm::vec4 GetMaterialDiffuse(int Shape) const { return Materials[Shape].Diffuse; }
	glm::vec4 GetMaterialSpecular(int Shape) const { return Materials[Shape].Specular; }
	
	float GetMaterialMetalness(int Shape) const { return Materials[Shape].Metallic; }
	float GetMaterialRoughness(int Shape) const { return Materials[Shape].Roughness; }
	float GetMaterialEmissive(int Shape) const { return Materials[Shape].Emissive; }	

	size_t GetNumShapes() const { return Shapes.size(); }
	void PopShape() { if (Shapes.size() > 1) { Shapes.pop_back(); } }
	void MergeShapes();

	Buffer* AddBuffer() { AdditionalBuffers.emplace_back(std::make_unique<Buffer>()); return AdditionalBuffers.back().get(); }

	const Buffer* GetVertexBuffer() const { return VertexBuffer; };

	static bool LoadMesh(const std::string& FileName, Mesh& NewMesh, float Scale = 1.f, bool bMergeShapes = false, bool bRemoveLastShape = false);
	static bool CreateMesh(const std::string& InName, const std::vector<Mesh::Vertex_t>& vertices, Mesh& NewMesh);

	virtual void OnLoaded();
protected:

	void CalcBounds();
	size_t FindOrAddVertex(glm::vec3 Position, glm::vec3 Normal, glm::vec2 TexCoord, glm::vec4 Tangent);

	std::string Name;
	std::vector<Vertex_t> Vertices;
	std::vector<Shape_s> Shapes;
	glm::vec3 AABB[2];

	Buffer* VertexBuffer;

	std::vector<std::unique_ptr<Buffer>> AdditionalBuffers;
};

