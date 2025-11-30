//
//#pragma once
//
//#include <utility>
//
//#include <map>
//#include <mutex>
//
//#include "TSQueue.h"
//
//#include "glad/glad.h"
//
//#include "ChunksVoxelsDataPoolMetadata.h"
//
//
//class VoxelsDataPool {
//
//public:
//
//	unsigned int numVoxelsPerFaceClassifications;
//	std::vector<unsigned int> NumVoxelPerFaceClassification;
//
//	std::vector<unsigned int> currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes;
//
//	unsigned int megaVoxelsPerFaceDataBufferObjectID;
//	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;
//
//	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;
//
//	const unsigned int numVoxelsDataToStore = 50000000;
//	size_t sizeOfPoolInBytes;
//
//	std::vector<TSQueue<unsigned int>> freeDataBucketsPerClassificationPositionInMegaVoxelsArray;
//
//	std::mutex m_mutex;
//
//	VoxelsDataPool(unsigned int _numVoxelsPerFaceClassifications, std::vector<unsigned int> _NumVoxelPerFaceClassification, std::vector<unsigned int> numBucketsPerClassification, unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {
//
//		numVoxelsPerFaceClassifications = _numVoxelsPerFaceClassifications;
//		NumVoxelPerFaceClassification = _NumVoxelPerFaceClassification;
//
//		currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes.resize(numVoxelsPerFaceClassifications);
//		freeDataBucketsPerClassificationPositionInMegaVoxelsArray.resize(numVoxelsPerFaceClassifications);
//
//		sizeOfPoolInBytes = 0;
//		for (int i = 0; i < numVoxelsPerFaceClassifications; i++)
//		{
//			currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes[i] = sizeOfPoolInBytes;
//
//			size_t _sizeOfBucketInBytes = sizeof(unsigned int) * NumVoxelPerFaceClassification[i];
//			sizeOfPoolInBytes += _sizeOfBucketInBytes * numBucketsPerClassification[i];
//		}
//
//		glCreateBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);
//
//		megaVoxelsPerFaceDataBufferObjectBindingLocation = _megaVoxelsPerFaceDataBufferObjectBindingLocation;
//
//		glNamedBufferStorage(megaVoxelsPerFaceDataBufferObjectID, sizeOfPoolInBytes, nullptr,
//			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
//
//		void* mappedPointerToBuffer = glMapNamedBufferRange(
//			megaVoxelsPerFaceDataBufferObjectID, 0, sizeOfPoolInBytes,
//			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
//		);
//		megaVoxelsPerFaceDataBufferGPUPointer = reinterpret_cast<uint32_t*>(mappedPointerToBuffer);
//
//		for (unsigned int i = 0; i < numVoxelsPerFaceClassifications; i++)
//		{
//			for (unsigned int j = 0; j < numBucketsPerClassification[i]; j++)
//			{
//				size_t currentBucketOffsetInBytes = currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes[i] + (j * NumVoxelPerFaceClassification[i] * sizeof(unsigned int));
//				freeDataBucketsPerClassificationPositionInMegaVoxelsArray[i].push(currentBucketOffsetInBytes / (sizeof(unsigned int)));
//			}
//		}
//	}
//
//	~VoxelsDataPool() {
//		glUnmapNamedBuffer(megaVoxelsPerFaceDataBufferObjectID);
//		glDeleteBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);
//	}
//
//	bool GPU_WriteFaceVoxelDataToFreeBucket(const std::vector<unsigned int>& chunkVoxelFaceDataToCopyToPool, FaceVoxelsDataPoolMetadata& chunkFaceVoxelDataPoolMetadata, const unsigned int& numVoxelsPerFace) {
//
//		unsigned int newFreeBucketOffsetIntoMegaVoxelArrayIndex;
//		if (GetFreeBucket(newFreeBucketOffsetIntoMegaVoxelArrayIndex, numVoxelsPerFace)) {
//
//			uint32_t* pointerToBucket = megaVoxelsPerFaceDataBufferGPUPointer + newFreeBucketOffsetIntoMegaVoxelArrayIndex;
//			memcpy(pointerToBucket, chunkVoxelFaceDataToCopyToPool.data(), chunkVoxelFaceDataToCopyToPool.size() * sizeof(unsigned int));
//
//			chunkFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex = newFreeBucketOffsetIntoMegaVoxelArrayIndex;
//
//			return true;
//		}
//		else {
//			return false;
//		}
//
//	}
//
//	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {
//
//		int currentClassificationIndex = VoxelPerFaceClassificationIndex(curFaceVoxelDataPoolMetadata.numVoxelDataInBucket / 3);
//		freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].push(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//
//		curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;
//
//	}
//
//
//	int VoxelPerFaceClassificationIndex(const unsigned int& curNumVoxelsPerFace) {
//
//		for (int i = 0; i < numVoxelsPerFaceClassifications; i++)
//		{
//			if (curNumVoxelsPerFace < NumVoxelPerFaceClassification[i]) {
//
//				//std::cout << "Num Voxels for Face : " << curNumVoxelsPerFace << " Assigned to Index : " << i << " with total capacity : " << NumVoxelPerFaceClassification[i] << std::endl;
//
//				return i;
//			}
//		}
//
//		return -1;
//	}
//
//private:
//
//	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {
//
//		int currentClassificationIndex = VoxelPerFaceClassificationIndex(numVoxelsPerFace);
//
//		std::lock_guard<std::mutex> lock(m_mutex);
//		if (!freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].empty()) {
//			freePoolOffsetInMegaVoxelsArrayIndex = freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].pop();
//			return true;
//		}
//
//		return false;
//	}
//
//};


#pragma once

#include <utility>

#include <unordered_map>
#include <map>
#include <mutex>

#include "TSUnorderedMap.h"

#include "glad/glad.h"

#include "GLMIncludes.h"
#include "glm/gtc/random.hpp"
#include "ChunksVoxelsDataPoolMetadata.h"

class LocalMemoryAllocator {

public:
	struct FreeBucket {
		unsigned int indexOffsetIntoMegaArray;
		unsigned int sizeInIndexCountInMegaArray;
	};

	unsigned int localMemorySizeInIndex;
	unsigned int memoryIndexOffset;

	std::unordered_map<unsigned int, unsigned int> usedBucketsIndexAndSize;
	std::map<unsigned int, unsigned int> freeBucketsSortedByPositionInMegaArray;
	std::mutex m_mutex;

	LocalMemoryAllocator(unsigned int _localMemorySizeInIndex, unsigned int _memoryIndexOffset) {

		localMemorySizeInIndex = _localMemorySizeInIndex;
		memoryIndexOffset = _memoryIndexOffset;

		freeBucketsSortedByPositionInMegaArray.insert({ memoryIndexOffset, localMemorySizeInIndex });
	}

	LocalMemoryAllocator& operator=(const LocalMemoryAllocator&) = delete;

	LocalMemoryAllocator(LocalMemoryAllocator&& other) noexcept
		: localMemorySizeInIndex(other.localMemorySizeInIndex),
		memoryIndexOffset(other.memoryIndexOffset),
		usedBucketsIndexAndSize(std::move(other.usedBucketsIndexAndSize)),
		freeBucketsSortedByPositionInMegaArray(std::move(other.freeBucketsSortedByPositionInMegaArray))
	{
		// m_mutex left default-constructed for this instance
	}

	LocalMemoryAllocator& operator=(LocalMemoryAllocator&& other) noexcept {
		if (this == &other) return *this;
		localMemorySizeInIndex = other.localMemorySizeInIndex;
		memoryIndexOffset = other.memoryIndexOffset;
		usedBucketsIndexAndSize = std::move(other.usedBucketsIndexAndSize);
		freeBucketsSortedByPositionInMegaArray = std::move(other.freeBucketsSortedByPositionInMegaArray);
		// don't try to move the mutex
		return *this;
	}


	//void MakeBucketAFreeBucket(const unsigned int& bucketIndexToFree) {
	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {

		std::lock_guard<std::mutex> lock(m_mutex);

		auto it = usedBucketsIndexAndSize.find(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
		if (it != usedBucketsIndexAndSize.end()) {

			FreeBucket newBucket = { curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex, usedBucketsIndexAndSize[curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex] };

			/// GO BACK
			/// AND TEST 
			/// number face bucket properly with unordered map for num voxels for index and not WRONGLY input lod.

			freeBucketsSortedByPositionInMegaArray.insert({ newBucket.indexOffsetIntoMegaArray, newBucket.sizeInIndexCountInMegaArray });
			//auto itFB = freeBucketsSortedByPositionInMegaArray.begin();
			//while (itFB != freeBucketsSortedByPositionInMegaArray.end() && itFB->indexOffsetIntoMegaArray < newBucket.indexOffsetIntoMegaArray) {
			//	itFB++;
			//}
			//freeBucketsSortedByPositionInMegaArray.insert(itFB, newBucket);

			usedBucketsIndexAndSize.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);

			curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;

			//std::cout << "D : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << ", numVoxels : " << newBucket.sizeInIndexCountInMegaArray << std::endl;

			//Defrag();
		}
		else {
			std::cout << "Trying to free bucket that was not in use : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << std::endl;
		}

	}


	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {

		if (numVoxelsPerFace > 0) {

			std::lock_guard<std::mutex> lock(m_mutex);



			while (!freeBucketsSortedByPositionInMegaArray.empty()) {
				auto it = freeBucketsSortedByPositionInMegaArray.begin();

				while (it != freeBucketsSortedByPositionInMegaArray.end()) {
					if (it->second > numVoxelsPerFace) {

						freePoolOffsetInMegaVoxelsArrayIndex = it->first;

						unsigned int remainingSize = it->second - numVoxelsPerFace;

						it = freeBucketsSortedByPositionInMegaArray.erase(it);
						freeBucketsSortedByPositionInMegaArray.insert({ freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace, remainingSize });

						//std::cout << "Created by SPLITTING LARGE BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex  << ", " << numVoxelsPerFace << std::endl;

						usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });

						return true;
					}
					else if (it->second == numVoxelsPerFace) {
						freePoolOffsetInMegaVoxelsArrayIndex = it->first;

						freeBucketsSortedByPositionInMegaArray.erase(it);

						//std::cout << "Created by DELETING BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex << ", " << numVoxelsPerFace << std::endl;
						usedBucketsIndexAndSize.insert({freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace});

						return true;

					}
					else {
						it++;
					}
				}

				std::cout << "Failed to find bucket with valid size." << std::endl;
				// If stuck in an infinite loop after this, it's because defrag until size did not find any bucket suitable for this size even after defrag, MEMORY NEEDS TO BE INCREASED.
				DefragUntilSize(numVoxelsPerFace);
			}

			return false;
		}
	}

	void DefragUntilSize(unsigned int sizeNeeded) {

		while (true)
		{
			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
			current_it++;

			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {

				auto prev_it = std::prev(current_it);
				if (prev_it->first + prev_it->second == current_it->first) {

					prev_it->second += current_it->second;

					current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);

					current_it--;

					if (prev_it->second >= sizeNeeded) {
						return;
					}
				}
				current_it++;
			}
		}
	}
};

class VoxelsDataPool {

public:

	unsigned int megaVoxelsPerFaceDataBufferObjectID;
	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;

	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;

	unsigned int numVoxelsDataToStore = 100000000;
	size_t sizeOfPoolInBytes;


	unsigned int numLocalAllocators = 70;
	std::vector<LocalMemoryAllocator> localMemoryAllocators;
	ThreadSafeUnorderedMap<unsigned int, unsigned int> indexMappedInAllocatorID;

	VoxelsDataPool(unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {

		megaVoxelsPerFaceDataBufferObjectBindingLocation = _megaVoxelsPerFaceDataBufferObjectBindingLocation;
		sizeOfPoolInBytes = numVoxelsDataToStore * sizeof(unsigned int);

		glCreateBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);

		glNamedBufferStorage(megaVoxelsPerFaceDataBufferObjectID, sizeOfPoolInBytes, nullptr,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		void* mappedPointerToBuffer = glMapNamedBufferRange(
			megaVoxelsPerFaceDataBufferObjectID, 0, sizeOfPoolInBytes,
			GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
		megaVoxelsPerFaceDataBufferGPUPointer = reinterpret_cast<uint32_t*>(mappedPointerToBuffer);

		localMemoryAllocators.reserve(numLocalAllocators);
		for (unsigned int i = 0; i < numLocalAllocators; i++)
		{
			unsigned int allocatorSize = numVoxelsDataToStore / numLocalAllocators;
			localMemoryAllocators.emplace_back(allocatorSize, i * allocatorSize);
		}

		std::srand(static_cast<unsigned int>(1667));
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

	//void MakeBucketAFreeBucket(const unsigned int& bucketIndexToFree) {
	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {
		
		if (indexMappedInAllocatorID.contains(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex)) {

			localMemoryAllocators[indexMappedInAllocatorID.get(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex)].MakeBucketAFreeBucket(curFaceVoxelDataPoolMetadata);
			indexMappedInAllocatorID.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
		}
		else {
			std::cout << "Freeing a bucket that is not in use." << std::endl;
		}
	}

private:

	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {

		unsigned int currentRandomAllocatorIndex = glm::linearRand(unsigned int(0), numLocalAllocators - 1);

		if (localMemoryAllocators[currentRandomAllocatorIndex].GetFreeBucket(freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace)) {
			indexMappedInAllocatorID.try_insert(freePoolOffsetInMegaVoxelsArrayIndex, currentRandomAllocatorIndex);
			return true;
		}
		return false;
	}

};