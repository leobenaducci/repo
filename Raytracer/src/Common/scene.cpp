#include "stdafx.h"
#include "scene.h"
#include "camera.h"
#include "mesh.h"
#include "texture.h"
#include "glm/gtc/matrix_transform.hpp"

static const glm::mat4 IdentityMatrix4(1.f, 0.f, 0.f, 0.f,
									   0.f, 1.f, 0.f, 0.f,
									   0.f, 0.f, 1.f, 0.f,
									   0.f, 0.f, 0.f, 1.f);

namespace DisplayLists
{
	static bool bInitialized = false;
	static RaytraceMesh ConeMesh, CubeMesh, CylinderMesh, PlaneMesh, SphereMesh, TetraMesh, TorusMesh, TeapotMesh;
	static GLuint Cone = 0;
	static GLuint Cube = 0;
	static GLuint Cylinder = 0;
	static GLuint Sphere = 0;
	static GLuint Teapot = 0;
	static GLuint Tetrahedron = 0;
	static GLuint Torus = 0;
	static GLuint Quad = 0;
	static GLuint FullScreenQuad = 0;

	static void Init();
}

SceneObject::SceneObject()
	: Mesh(nullptr)
	, MeshShape(0)
	, Albedo(nullptr)
	, Gloss(nullptr)
	, NormalMap(nullptr)
{

}


SceneObject* Scene::AddObject(const std::string& Name)
{
	if(DisplayLists::bInitialized == false)
		DisplayLists::Init();

	SceneObject* Obj = new SceneObject;
	Obj->SetName(Name);

	Obj->SetMesh(SceneObject::EMeshType::Cube);
	Obj->SetPosition(glm::vec3(0.f, 0.f, 0.f));
	Obj->SetRotation(glm::vec3(0.f, 0.f, 0.f));
	Obj->SetScale(glm::vec3(1.f, 1.f, 1.f));
	Obj->SetMaterialColor(glm::vec4(1.f, 1.f, 1.f, 1.f));

	Objects.push_back(Obj);

	return Obj;
}

void Scene::RemoveObject(const SceneObject* pObj)
{
	auto it = std::find(Objects.begin(), Objects.end(), pObj);

	if(it != Objects.end())
		Objects.erase(it);
}

void Scene::GetSceneBounds(glm::vec3* pMins, glm::vec3* pMaxs) const
{
	glm::vec3 Mins(FLT_MAX, FLT_MAX, FLT_MAX);
	glm::vec3 Maxs(FLT_MIN, FLT_MIN, FLT_MIN);

	for(auto pObj : Objects)
	{
		Mins = glm::min(Mins, pObj->GetPosition() - pObj->GetScale());
		Maxs = glm::max(Maxs, pObj->GetPosition() + pObj->GetScale());
	}

	if(pMins != nullptr)
		*pMins = Mins;

	if(pMaxs != nullptr)
		*pMaxs = Maxs;
}

///Left handed perspective matrix
glm::mat4 PerspectiveFov(float Fov, float width, float height, float znear, float zfar)
{
/*
	xScale     0          0               0
	0        yScale       0               0
	0          0       zf / (zf - zn)         1
	0          0 - zn*zf / (zf - zn)     0
	where:
	yScale = cot(fovY / 2)
	xScale = yScale / aspect ratio
*/
	const float f = 1.0f / tanf(Fov / 2.0f);
	const float aspect = width / height;

	float mat[16];
	memset(&mat, 0, sizeof(float)*16);

	mat[0] = f / aspect;
	mat[5] = f;
	mat[10] = zfar / (zfar-znear);
	mat[11] = 1.0f;
	mat[14] = -(znear*zfar) / (zfar-znear);

	return *(glm::mat4*)mat;
}

glm::mat4 OrthoMat(float l, float r, float t, float b, float n, float f)
{
	float mat[4][4];
	memset(&mat, 0, sizeof(float)*16);

	mat[0][0] = 2 / (r - l);
	mat[1][1] = 2 / (t - b);
	mat[2][2] = -1 / (f - n);

	mat[3][0] = -(r + l) / (r - l);
	mat[3][1] = -(t + b) / (t - b);
	mat[3][2] = -n / (f - n);
	mat[3][3] = 1;

	return *(glm::mat4*)mat;
}

void Scene::SetParameters(Shader* pShader)
{
	pShader->SetParameter("ViewMatrix", ViewMatrix);
	pShader->SetParameter("ViewProjectionMatrix", ViewProjectionMatrix);
	pShader->SetParameter("ProjectionMatrix", ProjectionMatrix);

	pShader->SetParameter("PrevViewMatrix", PrevViewMatrix);
	pShader->SetParameter("PrevViewProjectionMatrix", PrevViewProjectionMatrix);
	pShader->SetParameter("PrevProjectionMatrix", PrevProjectionMatrix);

	pShader->SetParameter("CameraPos", CameraPos);
	pShader->SetParameter("PrevCameraPos", PrevCameraPos);

	GLint _viewport[4];
	glGetIntegerv(GL_VIEWPORT, _viewport);
	ViewportSize = glm::vec2(static_cast<float>(_viewport[2] - _viewport[0]), static_cast<float>(_viewport[3] - _viewport[1]));

	pShader->SetParameter("ViewportSize", ViewportSize);

	pShader->SetParameter("LightDir", LightDir);

	pShader->SetParameter("CameraToWorld[0]", CameraToWorld[0], 4);
	pShader->SetParameter("PrevCameraToWorld[0]", PrevCameraToWorld[0], 4);
}

void Scene::UpdateTransforms()
{
	if (SceneCamera != nullptr)
	{
		ViewMatrix = SceneCamera->ViewMatrix;
		PrevViewMatrix = SceneCamera->PrevViewMatrix;
		ProjectionMatrix = SceneCamera->ProjectionMatrix;
		PrevProjectionMatrix = SceneCamera->PrevProjectionMatrix;
		ViewProjectionMatrix = SceneCamera->ViewProjectionMatrix;
		PrevViewProjectionMatrix = SceneCamera->PrevViewProjectionMatrix;
		ViewportSize = SceneCamera->ViewportSize;
		CameraPos = SceneCamera->Pos;
		PrevCameraPos = SceneCamera->PrevPos;
		CameraRot = SceneCamera->Rot;
		CameraFOV = SceneCamera->FOV;
		for (int i = 0; i < 4; i++) { CameraToWorld[i] = SceneCamera->CameraToWorld[i]; }
		for (int i = 0; i < 4; i++) { PrevCameraToWorld[i] = SceneCamera->PrevCameraToWorld[i]; }
	}
	else
	{
		PrevCameraPos = CameraPos;

		GLint _viewport[4];

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glGetIntegerv(GL_VIEWPORT, _viewport);

		ViewportSize = glm::vec2(static_cast<float>(_viewport[2] - _viewport[0]), static_cast<float>(_viewport[3] - _viewport[1]));
		if (OverrideProjectionMatrix)
		{
			ProjectionMatrix = *OverrideProjectionMatrix;
		}
		else
		{
			PrevProjectionMatrix = ProjectionMatrix;
			ProjectionMatrix = PerspectiveFov(glm::radians(CameraFOV), ViewportSize.x, ViewportSize.y, 0.1f, 1000.f);
		}

		PrevViewMatrix = ViewMatrix;

		if (SceneCamera != nullptr)
		{
			ViewMatrix = SceneCamera->ViewMatrix;
		}
		else
		{
			ViewMatrix = glm::translate(IdentityMatrix4, CameraPos);
			ViewMatrix = glm::rotate(ViewMatrix, glm::radians(CameraRot.x), glm::vec3(0.f, 1.f, 0.f));
			ViewMatrix = glm::rotate(ViewMatrix, glm::radians(CameraRot.y), glm::vec3(1.f, 0.f, 0.f));
			ViewMatrix = glm::rotate(ViewMatrix, glm::radians(CameraRot.z), glm::vec3(0.f, 0.f, 1.f));
			ViewMatrix = glm::inverse(ViewMatrix);
		}

		PrevViewProjectionMatrix = ViewProjectionMatrix;
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		glm::mat4 InvViewProj = glm::inverse(ViewProjectionMatrix);

		for (int i = 0; i < 4; i++) { PrevCameraToWorld[i] = CameraToWorld[i]; }
		CameraToWorld[0] = InvViewProj * glm::vec4(-1.f, -1.f, 1.f, 1.f);
		CameraToWorld[1] = InvViewProj * glm::vec4(1.f, -1.f, 1.f, 1.f);
		CameraToWorld[2] = InvViewProj * glm::vec4(-1.f, 1.f, 1.f, 1.f);
		CameraToWorld[3] = InvViewProj * glm::vec4(1.f, 1.f, 1.f, 1.f);

		CameraToWorld[0] /= CameraToWorld[0].w;
		CameraToWorld[1] /= CameraToWorld[1].w;
		CameraToWorld[2] /= CameraToWorld[2].w;
		CameraToWorld[3] /= CameraToWorld[3].w;
	}
	
	for(auto it : Objects)
		it->UpdateTransforms();
}

void Scene::Render(Shader* pShader, uint32_t ShaderFlags, std::function<void(Shader*, uint32_t)> SetShaderParameters)
{
	if (pShader == nullptr)
		return;

	for(auto it : Objects)
		it->Render(pShader, ShaderFlags, std::bind(&Scene::SetParameters, this, pShader), SetShaderParameters);
}

void Scene::DrawFullscreenQuad(Shader* pShader, uint32_t ShaderFlags, std::function<void(Shader*, uint32_t)> SetShaderParameters)
{
	if (DisplayLists::bInitialized == false)
		DisplayLists::Init();

	if(pShader != nullptr)
		pShader->Use(ShaderFlags);

	if (SetShaderParameters)
		SetShaderParameters(pShader, ShaderFlags);
	
	glCallList(DisplayLists::FullScreenQuad);
}

void Scene::RenderMesh(Mesh* pMesh, size_t MeshShape)
{
	if (MeshShape >= pMesh->GetNumShapes())
		return;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Tangents
	glClientActiveTexture(GL_TEXTURE3);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER_ARB, pMesh->GetVertexBuffer()->GetBufferId());

	glVertexPointer(3, GL_FLOAT, sizeof(Mesh::Vertex_t), (void*)0);
	glNormalPointer(GL_FLOAT, sizeof(Mesh::Vertex_t), (void*)(sizeof(float) * 3));

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Mesh::Vertex_t), (void*)(sizeof(float) * 6));

	//Tangents
	glClientActiveTexture(GL_TEXTURE3);
	glTexCoordPointer(4, GL_FLOAT, sizeof(Mesh::Vertex_t), (void*)(sizeof(float) * 8));

	const auto& Shape = pMesh->GetShapes()[MeshShape];
	glDrawArrays(GL_TRIANGLES, Shape.FirstVertex, Shape.VertexIndices.size());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//Tangents
	glClientActiveTexture(GL_TEXTURE3);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


void SceneObject::UpdateTransforms()
{
	PrevWorldMatrix = WorldMatrix;
	WorldMatrix = glm::translate(IdentityMatrix4, Position);
	WorldMatrix = glm::rotate(WorldMatrix, glm::radians(Rotation.z), glm::vec3(0.f, 0.f, 1.f));
	WorldMatrix = glm::rotate(WorldMatrix, glm::radians(Rotation.y), glm::vec3(1.f, 0.f, 0.f));
	WorldMatrix = glm::rotate(WorldMatrix, glm::radians(Rotation.x), glm::vec3(0.f, 1.f, 0.f));
	WorldMatrix = glm::scale(WorldMatrix, Scale);

	NormalMatrix = glm::inverse( glm::transpose(WorldMatrix) );
}


void SceneObject::Render(Shader* pShader, uint32_t ShaderFlags, std::function<void(Shader*)> SetSceneParameters, std::function<void(Shader*, uint32_t)> SetShaderParameters)
{
	if (pShader == nullptr)
		return;

	pShader->Use(ShaderFlags);
	SetSceneParameters(pShader);

	if (SetShaderParameters)
		SetShaderParameters(pShader, ShaderFlags);

	pShader->SetParameter("PrevWorldMatrix", PrevWorldMatrix);
	pShader->SetParameter("WorldMatrix", WorldMatrix);
	pShader->SetParameter("NormalMatrix", NormalMatrix);

	pShader->SetParameter("MaterialColor", MaterialColor);
	pShader->SetParameter("MaterialProperties", MaterialProperties);

	if (Albedo) Albedo->Bind(0); else Texture::White().Bind(0);
	if (Gloss) Gloss->Bind(1); else Texture::Black().Bind(1);
	if (NormalMap) NormalMap->Bind(2); else Texture::IdentityNormalMap().Bind(2);

	if (SetObjectParameters)
		SetObjectParameters(pShader, ShaderFlags);

	switch (MeshType)
	{
	case EMeshType::Cone:
		glCallList(DisplayLists::Cone);
		break;
	case EMeshType::Cube:
		glCallList(DisplayLists::Cube);
		break;
	case EMeshType::Cylinder:
		glCallList(DisplayLists::Cylinder);
		break;
	case EMeshType::Sphere:
		glCallList(DisplayLists::Sphere);
		break;
	case EMeshType::Teapot:
		glCallList(DisplayLists::Teapot);
		break;
	case EMeshType::Tetrahedron:
		glCallList(DisplayLists::Tetrahedron);
		break;
	case EMeshType::Torus:
		glCallList(DisplayLists::Torus);
		break;
	case EMeshType::Quad:
		glCallList(DisplayLists::Quad);
		break;
	case EMeshType::Mesh:
		if (Mesh != nullptr)
		{
			Scene::RenderMesh(Mesh, MeshShape);
		}
		break;
	}
}

void DisplayLists::Init()
{
	bInitialized = true;

	static const std::string Path("../../resources/Common/");

	Mesh::LoadMesh(Path + "cone.obj", ConeMesh);
	Mesh::LoadMesh(Path + "cube.obj", CubeMesh);
	Mesh::LoadMesh(Path + "cylinder.obj", CylinderMesh);
	Mesh::LoadMesh(Path + "plane.obj", PlaneMesh);
	Mesh::LoadMesh(Path + "sphere.obj", SphereMesh);
	Mesh::LoadMesh(Path + "tetrahedron.obj", TetraMesh);
	Mesh::LoadMesh(Path + "torus.obj", TorusMesh);
	Mesh::LoadMesh(Path + "teapot.obj", TeapotMesh);

	Cone = glGenLists(1);
	glNewList(Cone, GL_COMPILE);
		Scene::RenderMesh(&ConeMesh, 0);
	glEndList();

	Cube = glGenLists(1);
	glNewList(Cube, GL_COMPILE);
		Scene::RenderMesh(&CubeMesh, 0);
	glEndList();

	Cylinder = glGenLists(1);
	glNewList(Cylinder, GL_COMPILE);
		Scene::RenderMesh(&CylinderMesh, 0);
	glEndList();

	Sphere = glGenLists(1);
	glNewList(Sphere, GL_COMPILE);
		Scene::RenderMesh(&SphereMesh, 0);
	glEndList();

	Teapot = glGenLists(1);
	glNewList(Teapot, GL_COMPILE);
		Scene::RenderMesh(&TeapotMesh, 0);
	glEndList();

	Tetrahedron = glGenLists(1);
	glNewList(Tetrahedron, GL_COMPILE);
		Scene::RenderMesh(&TetraMesh, 0);
	glEndList();

	Torus = glGenLists(1);
	glNewList(Torus, GL_COMPILE);
		Scene::RenderMesh(&TorusMesh, 0);
	glEndList();

	Quad = glGenLists(1);
	glNewList(Quad, GL_COMPILE);
		Scene::RenderMesh(&PlaneMesh, 0);
	glEndList();

	FullScreenQuad = glGenLists(1);
	glNewList(FullScreenQuad, GL_COMPILE);
		glBegin(GL_TRIANGLES);
			glTexCoord2f(-1*.5f+.5f, 3*.5f+.5f); glVertex2f(-1, -1);
			glTexCoord2f( 3*.5f+.5f,-1*.5f+.5f); glVertex2f( 3, -1);
			glTexCoord2f(-1*.5f+.5f, 3*.5f+.5f); glVertex2f(-1, 3);
		glEnd();
	glEndList();
}

void Scene::RenderMeshType(SceneObject::EMeshType Type)
{
	switch (Type)
	{
	case SceneObject::EMeshType::Cube:
		glCallList(DisplayLists::Cube);
		break;
	}
}


void RaytraceScene::UpdateRaytraceScene()
{
	//Buffer ObjectsBuffer;
	//Buffer BvhBuffer;
	//Buffer MeshesBuffer;

	GpuObjects.resize(0);
	GpuObjects.reserve(Objects.size());

	static bool bDoOnce = true;
	std::map<SceneObject::EMeshType, RaytraceMesh*> StandardMeshes;
	StandardMeshes[SceneObject::EMeshType::Cone] = &DisplayLists::ConeMesh;
	StandardMeshes[SceneObject::EMeshType::Cube] = &DisplayLists::CubeMesh;
	StandardMeshes[SceneObject::EMeshType::Cylinder] = &DisplayLists::CylinderMesh;
	StandardMeshes[SceneObject::EMeshType::Quad] = &DisplayLists::PlaneMesh;
	StandardMeshes[SceneObject::EMeshType::Sphere] = &DisplayLists::SphereMesh;
	StandardMeshes[SceneObject::EMeshType::Tetrahedron] = &DisplayLists::TetraMesh;	
	StandardMeshes[SceneObject::EMeshType::Torus] = &DisplayLists::TorusMesh;
	StandardMeshes[SceneObject::EMeshType::Teapot] = &DisplayLists::TeapotMesh;

	size_t CurrentMeshesSize = GpuMeshes.size();

	if (bDoOnce)
	{
		bDoOnce = false;
		GpuMeshes.reserve(1024);
		for (auto it : StandardMeshes)
		{
			GpuMeshes.push_back(it.second->GpuMesh);
		}
	}

	for (size_t i = 0; i < Objects.size(); i++)
	{
		SceneObject* Obj = Objects[i];

		GpuObject_s NewObject;
		NewObject.SetTransform(Obj->GetPosition(), Obj->GetRotation(), Obj->GetScale());

		if (Obj->GetMeshType() == SceneObject::EMeshType::Mesh && Obj->GetMesh() != nullptr)
		{
			size_t MeshIdx = (size_t)-1;
			auto GpuMeshIt = std::find(GpuMeshes.begin(), GpuMeshes.end(), ((RaytraceMesh*)Obj->GetMesh())->GpuMesh);
			if (GpuMeshIt == GpuMeshes.end())
			{
				GpuMeshes.push_back(((RaytraceMesh*)Obj->GetMesh())->GpuMesh);
				GpuMeshIt = GpuMeshes.end() - 1;
				MeshIdx = GpuMeshes.size() - 1;
			}
			else
			{
				MeshIdx = std::distance(GpuMeshes.begin(), GpuMeshIt);
			}

			if (MeshIdx == (size_t)-1)
				continue;

			NewObject.MeshShape = ivec2(MeshIdx, Obj->GetMeshShape());

			Obj->GetMesh()->GetAABB(Obj->GetMeshShape(), NewObject.AABBMin, NewObject.AABBMax);
			TransformAABB(NewObject.AABBMin, NewObject.AABBMax, NewObject.GetTransformRef());
		}
		else
		{
			auto StandardMeshIt = StandardMeshes.find(Obj->GetMeshType());
			if (StandardMeshIt == StandardMeshes.end())
				continue;

			size_t MeshIdx = (size_t)-1;
			auto GpuMeshIt = std::find(GpuMeshes.begin(), GpuMeshes.end(), StandardMeshIt->second->GpuMesh);
			if (GpuMeshIt != GpuMeshes.end())
			{
				MeshIdx = std::distance(GpuMeshes.begin(), GpuMeshIt);
			}

			if (MeshIdx == (size_t)-1)
				continue;

			NewObject.MeshShape = ivec2(MeshIdx, 0);

			StandardMeshIt->second->GetAABB(0, NewObject.AABBMin, NewObject.AABBMax);
			TransformAABB(NewObject.AABBMin, NewObject.AABBMax, NewObject.GetTransformRef());
		}

		NewObject.Color = Objects[i]->GetMaterialColor();
		NewObject.RoughnessMetallic = vec2(Objects[i]->GetMaterialProperties().y, Objects[i]->GetMaterialProperties().x);
		NewObject.EmissiveUnused = vec2(Objects[i]->GetMaterialProperties().z, 0.f);
		NewObject.Albedo = Obj->GetAlbedoTexture() ? Obj->GetAlbedoTexture()->GetTextureHandle() : Texture::White().GetTextureHandle();
		NewObject.Gloss = Obj->GetGlossTexture() ? Obj->GetGlossTexture()->GetTextureHandle() : Texture::Black().GetTextureHandle();

		GpuObjects.push_back(NewObject);
	}

	ObjectsBuffer.UploadData(GpuObjects.data(), GpuObjects.size() * sizeof(GpuObject_s));

	if(GpuMeshes.size() != CurrentMeshesSize)
		MeshesBuffer.UploadData(GpuMeshes.data(), GpuMeshes.size() * sizeof(GpuMesh_s));

	std::vector<BvhNode_s> BvhTree = CreateBvhTree(GpuObjects.size(), GpuObjects.data());
	const GpuBvhTree_s& GpuBvhTree = GenerateGpuBvhTree(BvhTree);
	BvhBuffer.UploadData(GpuBvhTree.Nodes, GpuBvhTree.Size * sizeof(GpuBvhNode_s));
}

void RaytraceScene::SetRaytraceParameters(Shader& RaytraceShader)
{
	UpdateTransforms();

	RaytraceShader.SetParameter("Scene.Tree", BvhBuffer.GetBufferAddress());
	RaytraceShader.SetParameter("Scene.NumObjects", GpuObjects.size());
	RaytraceShader.SetParameter("Scene.Objects", ObjectsBuffer.GetBufferAddress());
	RaytraceShader.SetParameter("Meshes", MeshesBuffer.GetBufferAddress());
	RaytraceShader.SetParameter("CameraPos", CameraPos);
	RaytraceShader.SetParameter("PrevCameraPos", PrevCameraPos);
	RaytraceShader.SetParameter("ViewMatrix", ViewMatrix);
	RaytraceShader.SetParameter("CameraToWorld[0]", CameraToWorld[0], 4);
	RaytraceShader.SetParameter("PrevCameraToWorld[0]", PrevCameraToWorld[0], 4);
	RaytraceShader.SetParameter("ViewProjectionMatrix", ViewProjectionMatrix);
	RaytraceShader.SetParameter("PrevViewProjectionMatrix", PrevViewProjectionMatrix);
	RaytraceShader.SetParameter("LightDir", LightDir);

	GLint _viewport[4];
	glGetIntegerv(GL_VIEWPORT, _viewport);
	ViewportSize = glm::vec2(static_cast<float>(_viewport[2] - _viewport[0]), static_cast<float>(_viewport[3] - _viewport[1]));

	RaytraceShader.SetParameter("ViewportSize", ViewportSize);
}