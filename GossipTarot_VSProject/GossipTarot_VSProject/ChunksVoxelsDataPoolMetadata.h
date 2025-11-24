#pragma once

#include "glad/glad.h"

struct FaceVoxelsDataPoolMetadata {

public:

	unsigned int voxelDataBucketOffsetIntoMegaArrayIndex = 0;
	unsigned int numVoxelDataInBucket = 0;
};

struct ChunkVoxelsDataPoolMetadata {

public:

	unsigned int packedChunkIndex;

	FaceVoxelsDataPoolMetadata topFaceVoxelsDataPoolMetadata;
	FaceVoxelsDataPoolMetadata bottomFaceVoxelsDataPoolMetadata;
	FaceVoxelsDataPoolMetadata leftFaceVoxelsDataPoolMetadata;
	FaceVoxelsDataPoolMetadata rightFaceVoxelsDataPoolMetadata;
	FaceVoxelsDataPoolMetadata frontFaceVoxelsDataPoolMetadata;
	FaceVoxelsDataPoolMetadata backFaceVoxelsDataPoolMetadata;
};

class ChunksVoxelsDataPoolMetadata {
	
public:

	std::vector<ChunkVoxelsDataPoolMetadata> chunksVoxelsDataPoolMetadatas;

	size_t sizeOfChunksVoxelsDataPoolMetadatasInBytes;

	ChunkVoxelsDataPoolMetadata* gpu_chunksVoxelsDataPoolMetadatasPointer;

	unsigned int gpu_chunksVoxelsDataPoolMetadatasBindingPoint;
	unsigned int gpu_chunksVoxelsDataPoolMetadatasBufferID;

	ChunksVoxelsDataPoolMetadata(const Vector3& worldSizeInChunks, const unsigned int& bindingLocation) {

		gpu_chunksVoxelsDataPoolMetadatasBindingPoint = bindingLocation;

		unsigned int totalNumChunks = worldSizeInChunks.x * worldSizeInChunks.y * worldSizeInChunks.z;
		chunksVoxelsDataPoolMetadatas.resize(totalNumChunks);

		sizeOfChunksVoxelsDataPoolMetadatasInBytes = totalNumChunks * sizeof(ChunkVoxelsDataPoolMetadata);

		glCreateBuffers(1, &gpu_chunksVoxelsDataPoolMetadatasBufferID);

		glNamedBufferStorage(gpu_chunksVoxelsDataPoolMetadatasBufferID, sizeOfChunksVoxelsDataPoolMetadatasInBytes, nullptr,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		void* mappedPointerToBuffer = glMapNamedBufferRange(
			gpu_chunksVoxelsDataPoolMetadatasBufferID, 0, sizeOfChunksVoxelsDataPoolMetadatasInBytes,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
		gpu_chunksVoxelsDataPoolMetadatasPointer = reinterpret_cast<ChunkVoxelsDataPoolMetadata*>(mappedPointerToBuffer);

		if (gpu_chunksVoxelsDataPoolMetadatasPointer == nullptr) {
			std::cout << "Failed to create a pointer on the gpu to 'gpu_chunksVoxelsDataPoolMetadatasPointer'." << std::endl;
		}
	}

	~ChunksVoxelsDataPoolMetadata() {
		glUnmapNamedBuffer(gpu_chunksVoxelsDataPoolMetadatasBufferID);
		glDeleteBuffers(1, &gpu_chunksVoxelsDataPoolMetadatasBufferID);
	}

	void GPU_UploadChunksVoxelsDataPoolMetadatasToTheGPU() {
		memcpy(gpu_chunksVoxelsDataPoolMetadatasPointer, chunksVoxelsDataPoolMetadatas.data(), sizeOfChunksVoxelsDataPoolMetadatasInBytes);
	}

	void GPU_UploadChunkVoxelsDataPoolMetadatasToTheGPU(unsigned int chunkVoxelDataPoolMetadatasIndex) {
		memcpy(gpu_chunksVoxelsDataPoolMetadatasPointer + chunkVoxelDataPoolMetadatasIndex, &chunksVoxelsDataPoolMetadatas[chunkVoxelDataPoolMetadatasIndex], sizeof(ChunkVoxelsDataPoolMetadata));
	}

};