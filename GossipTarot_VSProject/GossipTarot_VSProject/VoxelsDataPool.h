
#pragma once

#include <utility>

#include <map>
#include <mutex>

#include "TSQueue.h"

#include "glad/glad.h"

#include "ChunksVoxelsDataPoolMetadata.h"


class VoxelsDataPool {

public:

	unsigned int numVoxelsPerFaceClassifications;
	std::vector<unsigned int> NumVoxelPerFaceClassification;

	std::vector<unsigned int> currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes;

	unsigned int megaVoxelsPerFaceDataBufferObjectID;
	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;

	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;

	const unsigned int numVoxelsDataToStore = 50000000;
	size_t sizeOfPoolInBytes;

	std::vector<TSQueue<unsigned int>> freeDataBucketsPerClassificationPositionInMegaVoxelsArray;

	std::mutex m_mutex;

	VoxelsDataPool(unsigned int _numVoxelsPerFaceClassifications, std::vector<unsigned int> _NumVoxelPerFaceClassification, std::vector<unsigned int> numBucketsPerClassification, unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {

		numVoxelsPerFaceClassifications = _numVoxelsPerFaceClassifications;
		NumVoxelPerFaceClassification = _NumVoxelPerFaceClassification;

		currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes.resize(numVoxelsPerFaceClassifications);
		freeDataBucketsPerClassificationPositionInMegaVoxelsArray.resize(numVoxelsPerFaceClassifications);

		sizeOfPoolInBytes = 0;
		for (int i = 0; i < numVoxelsPerFaceClassifications; i++)
		{
			currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes[i] = sizeOfPoolInBytes;

			size_t _sizeOfBucketInBytes = sizeof(unsigned int) * NumVoxelPerFaceClassification[i];
			sizeOfPoolInBytes += _sizeOfBucketInBytes * numBucketsPerClassification[i];
		}

		glCreateBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);

		megaVoxelsPerFaceDataBufferObjectBindingLocation = _megaVoxelsPerFaceDataBufferObjectBindingLocation;

		glNamedBufferStorage(megaVoxelsPerFaceDataBufferObjectID, sizeOfPoolInBytes, nullptr,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		void* mappedPointerToBuffer = glMapNamedBufferRange(
			megaVoxelsPerFaceDataBufferObjectID, 0, sizeOfPoolInBytes,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
		megaVoxelsPerFaceDataBufferGPUPointer = reinterpret_cast<uint32_t*>(mappedPointerToBuffer);

		for (unsigned int i = 0; i < numVoxelsPerFaceClassifications; i++)
		{
			for (unsigned int j = 0; j < numBucketsPerClassification[i]; j++)
			{
				size_t currentBucketOffsetInBytes = currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes[i] + (j * NumVoxelPerFaceClassification[i] * sizeof(unsigned int));
				freeDataBucketsPerClassificationPositionInMegaVoxelsArray[i].push(currentBucketOffsetInBytes / (sizeof(unsigned int)));
			}
		}
	}

	~VoxelsDataPool() {
		glUnmapNamedBuffer(megaVoxelsPerFaceDataBufferObjectID);
		glDeleteBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);
	}

	bool GPU_WriteFaceVoxelDataToFreeBucket(const std::vector<unsigned int>& chunkVoxelFaceDataToCopyToPool, FaceVoxelsDataPoolMetadata& chunkFaceVoxelDataPoolMetadata, const unsigned int& numVoxelsPerFace) {

		unsigned int newFreeBucketOffsetIntoMegaVoxelArrayIndex;
		if (GetFreeBucket(newFreeBucketOffsetIntoMegaVoxelArrayIndex, numVoxelsPerFace)) {

			uint32_t* pointerToBucket = megaVoxelsPerFaceDataBufferGPUPointer + newFreeBucketOffsetIntoMegaVoxelArrayIndex;
			memcpy(pointerToBucket, chunkVoxelFaceDataToCopyToPool.data(), chunkVoxelFaceDataToCopyToPool.size() * sizeof(unsigned int));

			chunkFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex = newFreeBucketOffsetIntoMegaVoxelArrayIndex;

			return true;
		}
		else {
			return false;
		}

	}

	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {

		int currentClassificationIndex = VoxelPerFaceClassificationIndex(curFaceVoxelDataPoolMetadata.numVoxelDataInBucket / 3);
		freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].push(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);

		curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;

	}


	int VoxelPerFaceClassificationIndex(const unsigned int& curNumVoxelsPerFace) {

		for (int i = 0; i < numVoxelsPerFaceClassifications; i++)
		{
			if (curNumVoxelsPerFace < NumVoxelPerFaceClassification[i]) {

				//std::cout << "Num Voxels for Face : " << curNumVoxelsPerFace << " Assigned to Index : " << i << " with total capacity : " << NumVoxelPerFaceClassification[i] << std::endl;

				return i;
			}
		}

		return -1;
	}

private:

	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {

		int currentClassificationIndex = VoxelPerFaceClassificationIndex(numVoxelsPerFace);

		std::lock_guard<std::mutex> lock(m_mutex);
		if (!freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].empty()) {
			freePoolOffsetInMegaVoxelsArrayIndex = freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].pop();
			return true;
		}

		return false;
	}

};
