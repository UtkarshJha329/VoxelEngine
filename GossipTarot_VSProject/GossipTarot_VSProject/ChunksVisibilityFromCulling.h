#pragma once

#include <vector>

#include "GLMIncludes.h"
#include "glad/glad.h"

#include "Camera.h"

struct Plane
{
	glm::vec3 normal;
	float distFromOriginToPlane;

	Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distFromOriginToPlane(glm::dot(normal, p1))
	{
	}

	float getSignedDistanceToPlane(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distFromOriginToPlane;
	}
};

struct Frustum
{
	Plane topFace;
	Plane bottomFace;

	Plane rightFace;
	Plane leftFace;

	Plane farFace;
	Plane nearFace;
};

struct ChunksVisiblityFromCulling {

public:

	unsigned int gpu_cameraFrustumBufferID;
	unsigned int gpu_cameraFrustumBufferBindingPoint;
	Frustum* gpu_cameraFrustumPointer;

	unsigned int gpu_chunksVisibilityDataBufferID;
	unsigned int gpu_chunksVisibilityDataBufferBindingPoint;
	unsigned int* gpu_chunksVisibilityDataBufferPointer;
	std::vector<unsigned int> chunksVisibilityData;

	Frustum cameraFrustum;


	ChunksVisiblityFromCulling(unsigned int totalNumChunks, unsigned int _gpu_cameraFrustumBufferBindingPoint, unsigned int _gpu_chunksVisibilityDataBufferBindingPoint) {

		chunksVisibilityData.resize(totalNumChunks, 1);

		gpu_chunksVisibilityDataBufferBindingPoint = _gpu_chunksVisibilityDataBufferBindingPoint;

		glCreateBuffers(1, &gpu_chunksVisibilityDataBufferID);

		size_t sizeOfChunksVisibilityBufferInBytes = chunksVisibilityData.size() * sizeof(unsigned int);

		glNamedBufferStorage(gpu_chunksVisibilityDataBufferID,
			sizeOfChunksVisibilityBufferInBytes,
			(const void*)chunksVisibilityData.data(),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		void* mappedPointerToBufferA = glMapNamedBufferRange(
			gpu_chunksVisibilityDataBufferID, 0, sizeOfChunksVisibilityBufferInBytes,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
		gpu_chunksVisibilityDataBufferPointer = reinterpret_cast<unsigned int*>(mappedPointerToBufferA);


		gpu_cameraFrustumBufferBindingPoint = _gpu_cameraFrustumBufferBindingPoint;
		glCreateBuffers(1, &gpu_cameraFrustumBufferID);

		glNamedBufferStorage(gpu_cameraFrustumBufferID,
			sizeof(Frustum),
			(const void*) &cameraFrustum,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		void* mappedPointerToBufferB = glMapNamedBufferRange(
			gpu_cameraFrustumBufferID, 0, sizeof(Frustum),
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
		gpu_cameraFrustumPointer = reinterpret_cast<Frustum*>(mappedPointerToBufferB);

		if (gpu_cameraFrustumPointer == nullptr) {
			std::cout << "Failed to create a pointer on the gpu to 'gpu_cameraFrustumPointer'." << std::endl;
		}


	}

	~ChunksVisiblityFromCulling() {
		glDeleteBuffers(1, &gpu_chunksVisibilityDataBufferID);
	}

	void UpdateCameraFrustumOnCPUAndGPU(const Camera& camera, const glm::vec3& cameraPosition) {

		float aspect = camera.viewport.dimensions.x / camera.viewport.dimensions.y;
		float zNear = camera.nearPlaneDistance;
		float zFar = camera.farPlaneDistance;
		float fovY = glm::radians(camera.fovInDegrees);

		const float halfVSide = zFar * tanf(fovY * .5f);
		const float halfHSide = halfVSide * aspect;
		const glm::vec3 frontMultFar = zFar * camera.cameraPointingDirection;

		cameraFrustum.nearFace = { cameraPosition + zNear * camera.cameraPointingDirection, camera.cameraPointingDirection };
		cameraFrustum.farFace = { cameraPosition + frontMultFar, -camera.cameraPointingDirection };
		cameraFrustum.rightFace = { cameraPosition,
								glm::cross(frontMultFar - camera.cameraRight * halfHSide, camera.cameraUp) };
		cameraFrustum.leftFace = { cameraPosition,
								glm::cross(camera.cameraUp,frontMultFar + camera.cameraRight * halfHSide) };
		cameraFrustum.topFace = { cameraPosition,
								glm::cross(camera.cameraRight, frontMultFar - camera.cameraUp * halfVSide) };
		cameraFrustum.bottomFace = { cameraPosition,
								glm::cross(frontMultFar + camera.cameraUp * halfVSide, camera.cameraRight) };


		memcpy(gpu_cameraFrustumPointer, &cameraFrustum, sizeof(Frustum));
	}

	void GPU_BindBuffersNeededForChunksVisibilityDataCalculation() {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gpu_cameraFrustumBufferBindingPoint, gpu_cameraFrustumBufferID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gpu_chunksVisibilityDataBufferBindingPoint, gpu_chunksVisibilityDataBufferID);
	}


};


