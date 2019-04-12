#pragma once

class Camera
{
public:
	glm::vec3 Pos = glm::vec3(0.f, 0.f, -64.f);
	glm::vec3 PrevPos = glm::vec3(0.f, 0.f, -64.f);
	glm::vec3 Rot = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 Jitter = glm::vec3(0.f, 0.f, 0.f);
	float FOV = 60.f;

	bool bPerspectiveProjection = true;

	glm::mat4* OverrideProjectionMatrix = nullptr;

	glm::vec4 CameraToWorld[4];
	glm::vec4 PrevCameraToWorld[4];

	const glm::mat4& ViewMatrix = _ViewMatrix;
	const glm::mat4& PrevViewMatrix = _PrevViewMatrix;
	const glm::mat4& ProjectionMatrix = _ProjectionMatrix;
	const glm::mat4& PrevProjectionMatrix = _PrevProjectionMatrix;
	const glm::mat4& ViewProjectionMatrix = _ViewProjectionMatrix;
	const glm::mat4& PrevViewProjectionMatrix = _PrevViewProjectionMatrix;
	const glm::vec2& ViewportSize = _ViewportSize;

	void Update(float FrameDelta, bool bCaptureInput);

	void SetOverrideViewportSize(glm::vec2* NewViewportSize) { if (bOverrideViewportSize = !!NewViewportSize) _ViewportSize = *NewViewportSize; }
	void SetLookAtTarget(glm::vec3* NewLookAtTarget) { if (bLookAtMatrix = !!NewLookAtTarget) LookAtTarget = *NewLookAtTarget; }

protected:
	int PrevMouseX = 0, PrevMouseY = 0;
	float MouseMovementFactor = 20.f;

	glm::mat4 _ViewMatrix, _PrevViewMatrix;
	glm::mat4 _ProjectionMatrix, _PrevProjectionMatrix;
	glm::mat4 _ViewProjectionMatrix, _PrevViewProjectionMatrix;
	glm::vec2 _ViewportSize;

	glm::vec3 LookAtTarget;

	bool bOverrideViewportSize = false;
	bool bLookAtMatrix = false;
};
