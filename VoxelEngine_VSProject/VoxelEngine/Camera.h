#pragma once

#include "Viewport.h"

class Camera {
	
public:
	float fovInDegrees = 45.0f;
	float nearPlaneDistance = 0.1f;
	float farPlaneDistance = 10000.0f;
	Viewport viewport;

	Vector3 targetPosition;

	Vector3 cameraPointingDirection		= Vector3{ 0.0f, 0.0f, 1.0f };	// Also called camera facing direction.
	Vector3 cameraRight					= Vector3{ 1.0f, 0.0f, 0.0f };
	Vector3 cameraUp					= Vector3{ 0.0f, 1.0f, 0.0f };

private:
	Mat4x4 projectionMatrix;


public:

	void SetProjectionMatrixToPerspectiveProjection(float _fovInDegrees, float _nearPlaneDistance, float _farPlaneDistance) {

		fovInDegrees = _fovInDegrees;
		nearPlaneDistance = _nearPlaneDistance;
		farPlaneDistance = _farPlaneDistance;

		projectionMatrix = glm::perspective(glm::radians(fovInDegrees), viewport.GetAspectRatio(), nearPlaneDistance, farPlaneDistance);
	}

	void SetProjectionMatrixToOrthographicProjection(float _nearPlaneDistance, float _farPlaneDistance) {

		nearPlaneDistance = _nearPlaneDistance;
		farPlaneDistance = _farPlaneDistance;

		float xStartCentreZero = 0.0f - viewport.dimensions.x / 2.0f;	// Left
		float xEndCentreZero = 0.0f + viewport.dimensions.x / 2.0f;		// Right
		float yStartCentreZero = 0.0f - viewport.dimensions.y / 2.0f;	// Bottom
		float yEndCentreZero = 0.0f + viewport.dimensions.y / 2.0f;		// Top

		projectionMatrix = glm::ortho(xStartCentreZero, xEndCentreZero, yStartCentreZero, yEndCentreZero, nearPlaneDistance, farPlaneDistance);
	}

	void SetCameraDirectionVectors(const Vector3& worldUp, const Vector3& cameraFacingDirection) {

		cameraPointingDirection = cameraFacingDirection;

		cameraRight = glm::normalize(glm::cross(cameraPointingDirection, worldUp));
		cameraUp = glm::normalize(glm::cross(cameraRight, cameraPointingDirection));

	}

	Mat4x4& GetProjectionMatrix() {
		return projectionMatrix;
	}
};