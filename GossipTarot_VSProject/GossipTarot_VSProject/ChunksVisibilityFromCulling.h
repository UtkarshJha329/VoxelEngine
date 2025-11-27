#pragma once

#include <vector>

#include "GLMIncludes.h"
#include "glad/glad.h"

#include "Camera.h"

struct Plane {
	glm::vec3 normal;
	float _pad0;            // padding
	glm::vec3 pointOnPlane;
	float _pad1;            // padding
};

struct Frustum
{
	Plane nearPlane;

	Plane rightPlane;
	Plane leftPlane;

	Plane topPlane;
	Plane bottomPlane;
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

		chunksVisibilityData.resize(totalNumChunks, 0);

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
			(const void*)&cameraFrustum,
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

		const float halfVSide = zFar * tanf(fovY);
		const float halfHSide = halfVSide * aspect;

		cameraFrustum.nearPlane = { glm::normalize(camera.cameraPointingDirection), 0.0f, cameraPosition + (camera.cameraPointingDirection * zNear), 0.0f };

		Vector3 vectorInRightPlane = zFar * camera.cameraPointingDirection + halfHSide * camera.cameraRight;
		Vector3 normalOfRightPlane = glm::cross(camera.cameraUp, vectorInRightPlane);

		Vector3 vectorInLeftPlane = zFar * camera.cameraPointingDirection - halfHSide * camera.cameraRight;
		Vector3 normalOfleftPlane = glm::cross(vectorInLeftPlane, camera.cameraUp);

		cameraFrustum.rightPlane = { glm::normalize(normalOfRightPlane), 0.0f, cameraPosition, 0.0f };
		cameraFrustum.leftPlane = { glm::normalize(normalOfleftPlane), 0.0f, cameraPosition, 0.0f };

		Vector3 vectorInTopPlane = zFar * camera.cameraPointingDirection + halfVSide * camera.cameraUp;
		Vector3 normalOfTopPlane = glm::cross(vectorInTopPlane, camera.cameraRight);

		Vector3 vectorInBottomPlane = zFar * camera.cameraPointingDirection - halfVSide * camera.cameraUp;
		Vector3 normalOfBottomPlane = glm::cross(camera.cameraRight, vectorInBottomPlane);

		cameraFrustum.topPlane = { glm::normalize(normalOfTopPlane), 0.0f, cameraPosition, 0.0f };
		cameraFrustum.bottomPlane = { glm::normalize(normalOfBottomPlane), 0.0f, cameraPosition, 0.0f };

		memcpy(gpu_cameraFrustumPointer, &cameraFrustum, sizeof(Frustum));
	}

	void GPU_BindBuffersNeededForChunksVisibilityDataCalculation() {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gpu_cameraFrustumBufferBindingPoint, gpu_cameraFrustumBufferID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, gpu_chunksVisibilityDataBufferBindingPoint, gpu_chunksVisibilityDataBufferID);
	}


};


