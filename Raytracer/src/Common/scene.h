#pragma once

#include "buffer.h"
#include "raytrace.h"

class ObjectMaterial
{
public:
	ObjectMaterial() {}
	~ObjectMaterial() {}

private:

};

class Mesh;

class SceneObject
{
public:
	enum class EMeshType
	{
		Cone,
		Cube,
		Cylinder,
		Sphere,
		Teapot,
		Tetrahedron,
		Torus,
		Quad,
		Mesh,
	};

	SceneObject();
	~SceneObject() {}

	void SetPosition(const glm::vec3& InPos) { Position = InPos; }
	const glm::vec3& GetPosition() const { return Position; }

	void SetRotation(const glm::vec3& InRot) { Rotation = InRot; }
	const glm::vec3& GetRotation() const { return Rotation; }

	void SetScale(const glm::vec3& InScale) { Scale = InScale; }
	const glm::vec3& GetScale() const { return Scale; }

	void SetMesh(const EMeshType InMeshType, Mesh* pMesh = nullptr, int InShape = 0) { MeshType = InMeshType; Mesh = pMesh; MeshShape = InShape; }
	const EMeshType& GetMeshType() const { return MeshType; }
	const Mesh* GetMesh() const { return Mesh; }
	const size_t GetMeshShape() const { return MeshShape; }

	void SetName(const std::string& InName) { Name = InName; }
	const std::string& GetName() const { return Name; }

	void SetMaterialColor(const glm::vec4& InMaterialColor) { MaterialColor = InMaterialColor; }
	const glm::vec4& GetMaterialColor() const { return MaterialColor; }

	void SetMaterialProperties(const glm::vec4& InMaterialProperties) { MaterialProperties = InMaterialProperties; }
	const glm::vec4& GetMaterialProperties() const { return MaterialProperties; }

	void SetAlbedoTexture(class Texture* albedo) { Albedo = albedo; }
	const Texture* GetAlbedoTexture() const { return Albedo; }

	void SetGlossTexture(class Texture* gloss) { Gloss = gloss; }
	const Texture* GetGlossTexture() const { return Gloss; }

	void SetNormalMap(class Texture* normalMap) { NormalMap = normalMap; }
	const Texture* GetNormalMap() const { return NormalMap; }

	void Render(Shader* pShader, uint32_t ShaderFlags, std::function<void(Shader*)> SetSceneParameters, std::function<void(Shader*, uint32_t)> SetShaderParameters);

	void UpdateTransforms();

	std::function<void(Shader*, uint32_t)> SetObjectParameters;

//private:
	std::string Name;

	glm::vec3 Position;
	glm::vec3 Rotation;
	glm::vec3 Scale;

	EMeshType MeshType;
	Mesh* Mesh;
	size_t MeshShape;

	glm::vec4 MaterialColor;
	glm::vec4 MaterialProperties;

	glm::mat4 WorldMatrix;
	glm::mat4 NormalMatrix;

	glm::mat4 PrevWorldMatrix;

	Texture* Albedo;
	Texture* Gloss;
	Texture* NormalMap;

	friend class Scene;
};

class Scene
{
public:
	Scene() : OverrideProjectionMatrix(nullptr) {}

	~Scene()
	{
		for(auto it : Objects)
			delete it;
	}

	void SetCamera(const glm::vec3& Pos, const glm::vec3& Rot, const float FOV)
	{
		CameraPos = Pos;
		CameraRot = Rot;
		CameraFOV = FOV;
	}

	void SetCamera(class Camera* pCamera)
	{
		SceneCamera = pCamera;
	}

	SceneObject* AddObject(const std::string& Name);
	void RemoveObject(const SceneObject* pObj);

	void GetSceneBounds(glm::vec3* pMins, glm::vec3* pMaxs) const;

	void SetLightDir(glm::vec3 InLightDir) { LightDir = InLightDir; }
	const glm::vec3& GetLightDir() const { return LightDir; }

	void SetParameters(Shader* pShader);
	void Render(Shader* pShader, uint32_t ShaderFlags, std::function<void(Shader*, uint32_t)> SetShaderParameters = nullptr);
	void UpdateTransforms();

	void OverrideProjection(glm::mat4* OverrideProj) { OverrideProjectionMatrix = OverrideProj; }
	
	const glm::mat4& GetViewMatrix() const { return ViewMatrix; }
	const glm::mat4& GetProjectionMatrix() const { return ProjectionMatrix; }
	const glm::mat4& GetViewProjectionMatrix() const { return ViewProjectionMatrix; }	

	static void DrawFullscreenQuad(Shader* pShader, uint32_t ShaderFlags, std::function<void(Shader*, uint32_t)> SetShaderParameters);
	static void RenderMesh(Mesh* pMesh, size_t MeshShape);

	static void RenderMeshType(SceneObject::EMeshType Type);

public:
	glm::vec4 CameraToWorld[4];
	glm::vec4 PrevCameraToWorld[4];

protected:
	std::vector<SceneObject*> Objects;

	class Camera* SceneCamera;

	glm::vec3 CameraPos;
	glm::vec3 PrevCameraPos;
	glm::vec3 CameraRot;
	float CameraFOV;

	glm::vec3 LightDir;

	glm::vec2 ViewportSize;

	glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

	//for temporal effects
	glm::mat4 PrevViewMatrix, PrevProjectionMatrix, PrevViewProjectionMatrix;

	glm::mat4* OverrideProjectionMatrix;
};


class RaytraceScene : public Scene
{
public:
	RaytraceScene() : ObjectsBuffer(GL_DYNAMIC_READ), BvhBuffer(GL_DYNAMIC_READ), MeshesBuffer(GL_STATIC_READ) {}
	void UpdateRaytraceScene();

	void SetRaytraceParameters(Shader& RaytraceShader);

private:
	Buffer ObjectsBuffer;
	Buffer BvhBuffer;
	Buffer MeshesBuffer;

	std::vector<GpuMesh_s> GpuMeshes;
	std::vector<GpuObject_s> GpuObjects;
};

glm::mat4 PerspectiveFov(float Fov, float width, float height, float znear, float zfar);
glm::mat4 OrthoMat(float l, float r, float t, float b, float n, float f);