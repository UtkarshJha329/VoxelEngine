#pragma once

#include <unordered_map>
#include <vector>
#include <utility>
#include "TSQueue.h"

#include "glad/glad.h"

#include "ChunksVoxelsDataPoolMetadata.h"

class VoxelsDataPool {

public:

	unsigned int numLODLevels;
	std::vector<unsigned int> LODLevelAndItsMaxSizeInVoxels;

	std::vector<size_t> currentLODLevelOffsetIntoMegaVoxelsArrayInBytes;

	unsigned int megaVoxelsPerFaceDataBufferObjectID;
	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;

	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;

	//unsigned int numVoxelDataPerBucket;
	//unsigned int numBuckets;

	size_t sizeOfPoolInBytes;

	std::vector<TSQueue<unsigned int>> freeDataBucketsPerLODPositionInMegaVoxelsArray;


	VoxelsDataPool(unsigned int _numLODLevels, std::vector<unsigned int> _LODLevelAndItsMaxSizeInVoxels, std::vector<unsigned int> numBucketsPerLOD, unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {

		numLODLevels = _numLODLevels;
		LODLevelAndItsMaxSizeInVoxels = _LODLevelAndItsMaxSizeInVoxels;

		currentLODLevelOffsetIntoMegaVoxelsArrayInBytes.reserve(numLODLevels);
		currentLODLevelOffsetIntoMegaVoxelsArrayInBytes.resize(numLODLevels);

		freeDataBucketsPerLODPositionInMegaVoxelsArray.reserve(numLODLevels);
		freeDataBucketsPerLODPositionInMegaVoxelsArray.resize(numLODLevels);

		sizeOfPoolInBytes = 0;
		for (int i = 0; i < numLODLevels; i++)
		{
			currentLODLevelOffsetIntoMegaVoxelsArrayInBytes[i] = sizeOfPoolInBytes;

			size_t _sizeOfBucketInBytes = sizeof(unsigned int) * LODLevelAndItsMaxSizeInVoxels[i];
			sizeOfPoolInBytes += _sizeOfBucketInBytes * numBucketsPerLOD[i];
		}

		glCreateBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);

		//numVoxelDataPerBucket = _numVoxelDataPerBucket;
		//numBuckets = _numBuckets;

		megaVoxelsPerFaceDataBufferObjectBindingLocation = _megaVoxelsPerFaceDataBufferObjectBindingLocation;

		//sizeOfBucketInBytes = sizeof(unsigned int) * numVoxelDataPerBucket;
		//sizeOfPoolInBytes =  sizeOfBucketInBytes * numBuckets;

		glNamedBufferStorage(megaVoxelsPerFaceDataBufferObjectID, sizeOfPoolInBytes, nullptr,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		void* mappedPointerToBuffer = glMapNamedBufferRange(
			megaVoxelsPerFaceDataBufferObjectID, 0, sizeOfPoolInBytes,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
		megaVoxelsPerFaceDataBufferGPUPointer = reinterpret_cast<uint32_t*>(mappedPointerToBuffer);

		for (unsigned int i = 0; i < numLODLevels; i++)
		{
			for (unsigned int j = 0; j < numBucketsPerLOD[i]; j++)
			{
				size_t currentBucketOffsetInBytes = currentLODLevelOffsetIntoMegaVoxelsArrayInBytes[i] + (j * LODLevelAndItsMaxSizeInVoxels[i] * sizeof(unsigned int));
				freeDataBucketsPerLODPositionInMegaVoxelsArray[i].push(currentBucketOffsetInBytes / (sizeof(unsigned int)));
			}
		}
	}

	~VoxelsDataPool() {
		glUnmapNamedBuffer(megaVoxelsPerFaceDataBufferObjectID);
		glDeleteBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);
	}

	bool GPU_WriteFaceVoxelDataToFreeBucket(const std::vector<unsigned int>& chunkVoxelFaceDataToCopyToPool, FaceVoxelsDataPoolMetadata& chunkFaceVoxelDataPoolMetadata, const unsigned int& curLODLevel) {

		unsigned int newFreeBucketOffsetIntoMegaVoxelArrayIndex;
		if (GetFreeBucket(newFreeBucketOffsetIntoMegaVoxelArrayIndex, curLODLevel)) {
			
			uint32_t* pointerToBucket = megaVoxelsPerFaceDataBufferGPUPointer + newFreeBucketOffsetIntoMegaVoxelArrayIndex;
			memcpy(pointerToBucket, chunkVoxelFaceDataToCopyToPool.data(), chunkVoxelFaceDataToCopyToPool.size() * sizeof(unsigned int));
			
			chunkFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex = newFreeBucketOffsetIntoMegaVoxelArrayIndex;

			return true;
		}
		else {
			return false;
		}

	}

private:

	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayInBytes, const unsigned int& LODLevel) {

		if (!freeDataBucketsPerLODPositionInMegaVoxelsArray[LODLevel].empty()) {
		//if (!freeDataBucketsPerLODPositionInMegaVoxelsArray[0].empty()) {
			freePoolOffsetInMegaVoxelsArrayInBytes = freeDataBucketsPerLODPositionInMegaVoxelsArray[LODLevel].pop();
			//freePoolOffsetInMegaVoxelsArrayInBytes = freeDataBucketsPerLODPositionInMegaVoxelsArray[0].pop();
			return true;
		}

		return false;
	}

};