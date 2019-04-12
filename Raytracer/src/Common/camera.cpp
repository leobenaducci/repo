#include "stdafx.h"
#include "camera.h"

#include "glm/gtc/matrix_transform.hpp"

static const glm::mat4 IdentityMatrix4(1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f);

void Camera::Update(float FrameDelta, bool bCaptureInput)
{
	PrevPos = Pos;

	//KEYBOARD MOVEMENT
	if (bCaptureInput)
	{
		float MovementFactor = (GetAsyncKeyState(VK_SHIFT) ? 8.f : 2.f);
	
		//I KNOW :)
		glm::mat4 NewViewMatrix;
		NewViewMatrix = glm::rotate(NewViewMatrix, glm::radians(Rot.x), glm::vec3(0.f, 1.f, 0.f));
		NewViewMatrix = glm::rotate(NewViewMatrix, glm::radians(Rot.y), glm::vec3(1.f, 0.f, 0.f));
		NewViewMatrix = glm::rotate(NewViewMatrix, glm::radians(Rot.z), glm::vec3(0.f, 0.f, 1.f));
	
		if (GetAsyncKeyState('W'))
		{
			Pos.x += NewViewMatrix[2].x * 20 * FrameDelta * MovementFactor;
			Pos.y += NewViewMatrix[2].y * 20 * FrameDelta * MovementFactor;
			Pos.z += NewViewMatrix[2].z * 20 * FrameDelta * MovementFactor;
		}
		if (GetAsyncKeyState('S'))
		{
			Pos.x -= NewViewMatrix[2].x * 20 * FrameDelta * MovementFactor;
			Pos.y -= NewViewMatrix[2].y * 20 * FrameDelta * MovementFactor;
			Pos.z -= NewViewMatrix[2].z * 20 * FrameDelta * MovementFactor;
		}
		if (GetAsyncKeyState('A'))
		{
			Pos.x += sinf(glm::radians(Rot.x - 90)) * 20 * FrameDelta * MovementFactor;
			Pos.z += cosf(glm::radians(Rot.x - 90)) * 20 * FrameDelta * MovementFactor;
		}
		if (GetAsyncKeyState('D'))
		{
			Pos.x += sinf(glm::radians(Rot.x + 90)) * 20 * FrameDelta * MovementFactor;
			Pos.z += cosf(glm::radians(Rot.x + 90)) * 20 * FrameDelta * MovementFactor;
		}
		if (GetAsyncKeyState('Q'))
			Pos.y += 20 * FrameDelta * MovementFactor;
		if (GetAsyncKeyState('E'))
			Pos.y -= 20 * FrameDelta * MovementFactor;
	}
	
	//MOUSE HACK
	{
		POINT MousePos;
		GetCursorPos(&MousePos);

		if (GetAsyncKeyState(VK_RBUTTON) && bCaptureInput)
		{
			Rot.x += (MousePos.x - PrevMouseX) * FrameDelta * MouseMovementFactor;
			Rot.y += (MousePos.y - PrevMouseY) * FrameDelta * MouseMovementFactor;

			SetCursorPos(PrevMouseX, PrevMouseY);
			GetCursorPos(&MousePos);
		}

		PrevMouseX = MousePos.x;
		PrevMouseY = MousePos.y;
	}

	//view matrix
	_PrevViewMatrix = _ViewMatrix;
	if (bLookAtMatrix)
	{
		_ViewMatrix = glm::lookAt(Pos, LookAtTarget, vec3(0, 1, 0));
	}
	else
	{
		_ViewMatrix = glm::translate(IdentityMatrix4, Pos + Jitter);
		_ViewMatrix = glm::rotate(_ViewMatrix, glm::radians(Rot.x), glm::vec3(0.f, 1.f, 0.f));
		_ViewMatrix = glm::rotate(_ViewMatrix, glm::radians(Rot.y), glm::vec3(1.f, 0.f, 0.f));
		_ViewMatrix = glm::rotate(_ViewMatrix, glm::radians(Rot.z), glm::vec3(0.f, 0.f, 1.f));
	}
	_ViewMatrix = glm::inverse(_ViewMatrix);


	GLint _viewport[4];
	glGetIntegerv(GL_VIEWPORT, _viewport);
	if (!bOverrideViewportSize)
	{
		_ViewportSize = glm::vec2(static_cast<float>(_viewport[2] - _viewport[0]), static_cast<float>(_viewport[3] - _viewport[1]));
	}

	//projection matrix
	_PrevProjectionMatrix = _ProjectionMatrix;
	if (OverrideProjectionMatrix)
	{
		_ProjectionMatrix = *OverrideProjectionMatrix;
	}
	else if(bPerspectiveProjection)
	{
		_ProjectionMatrix = PerspectiveFov(glm::radians(FOV), _ViewportSize.x, _ViewportSize.y, 0.1f, 1000.f);
	}
	else
	{
		_ProjectionMatrix = OrthoMat(-_ViewportSize.x/2, _ViewportSize.x/2, -_ViewportSize.y/2, _ViewportSize.y/2, 1.f, 1000.f);
	}

	//view-projection matrix
	_PrevViewProjectionMatrix = _ViewProjectionMatrix;
	_ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	memcpy(PrevCameraToWorld, CameraToWorld, sizeof(CameraToWorld));

	glm::mat4 InvViewProj = glm::inverse(ViewProjectionMatrix);
	CameraToWorld[0] = InvViewProj * glm::vec4(-1.f, -1.f, 1.f, 1.f);
	CameraToWorld[1] = InvViewProj * glm::vec4(1.f, -1.f, 1.f, 1.f);
	CameraToWorld[2] = InvViewProj * glm::vec4(-1.f, 1.f, 1.f, 1.f);
	CameraToWorld[3] = InvViewProj * glm::vec4(1.f, 1.f, 1.f, 1.f);

	CameraToWorld[0] /= CameraToWorld[0].w;
	CameraToWorld[1] /= CameraToWorld[1].w;
	CameraToWorld[2] /= CameraToWorld[2].w;
	CameraToWorld[3] /= CameraToWorld[3].w;
}
