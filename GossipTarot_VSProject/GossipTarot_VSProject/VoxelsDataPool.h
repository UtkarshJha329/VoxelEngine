#pragma once

#include <unordered_map>
#include <utility>

#include <List>
#include <mutex>


#include "glad/glad.h"

#include "ChunksVoxelsDataPoolMetadata.h"

class VoxelsDataPool {

public:

	struct FreeBucket {
		unsigned int indexOffsetIntoMegaArray;
		unsigned int sizeInIndexCountInMegaArray;
	};

	unsigned int megaVoxelsPerFaceDataBufferObjectID;
	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;

	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;

	const unsigned int numVoxelsDataToStore = 300000;
	size_t sizeOfPoolInBytes;

	std::unordered_map<unsigned int, unsigned int> usedBucketsIndexAndSize;
	std::list<FreeBucket> freeBucketsSortedByPositionInMegaArray;
	std::mutex m_mutex;

	VoxelsDataPool(unsigned int _numVoxelsPerFaceClassifications, std::vector<unsigned int> _NumVoxelPerFaceClassification, std::vector<unsigned int> numBucketsPerClassification, unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {

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

		freeBucketsSortedByPositionInMegaArray.push_back({0, numVoxelsDataToStore});
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

		std::lock_guard<std::mutex> lock(m_mutex);

		auto it = usedBucketsIndexAndSize.find(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
		if (it != usedBucketsIndexAndSize.end()) {

			FreeBucket newBucket = { curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex, usedBucketsIndexAndSize[curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex] };

			/// GO BACK
			/// AND TEST 
			/// number face bucket properly with unordered map for num voxels for index and not WRONGLY input lod.

			//freeBucketsSortedByPositionInMegaArray.push_back({ newBucket.indexOffsetIntoMegaArray, newBucket.sizeInIndexCountInMegaArray });
			auto itFB = freeBucketsSortedByPositionInMegaArray.begin();
			while (itFB != freeBucketsSortedByPositionInMegaArray.end() && itFB->indexOffsetIntoMegaArray < newBucket.indexOffsetIntoMegaArray) {
				itFB++;
			}
			freeBucketsSortedByPositionInMegaArray.insert(itFB, newBucket);

			usedBucketsIndexAndSize.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);

			curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;

			//std::cout << "D : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << ", numVoxels : " << newBucket.sizeInIndexCountInMegaArray << std::endl;

			//Defrag();
		}
		else {
			std::cout << "Trying to free bucket that was not in use : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << std::endl;
		}

	}

private:

	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {

		if (numVoxelsPerFace > 0) {

			std::lock_guard<std::mutex> lock(m_mutex);

			bool found = false;

			while (!found) {

				if (!freeBucketsSortedByPositionInMegaArray.empty()) {

					auto it = freeBucketsSortedByPositionInMegaArray.begin();

					while (it != freeBucketsSortedByPositionInMegaArray.end()) {
						if (it->sizeInIndexCountInMegaArray > numVoxelsPerFace) {

							freePoolOffsetInMegaVoxelsArrayIndex = it->indexOffsetIntoMegaArray;

							it->indexOffsetIntoMegaArray = freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace;
							it->sizeInIndexCountInMegaArray = it->sizeInIndexCountInMegaArray - numVoxelsPerFace;

							//std::cout << "Created by SPLITTING LARGE BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex  << ", " << numVoxelsPerFace << std::endl;

							usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });

							return true;
						}
						else if (it->sizeInIndexCountInMegaArray == numVoxelsPerFace) {
							freePoolOffsetInMegaVoxelsArrayIndex = it->indexOffsetIntoMegaArray;

							freeBucketsSortedByPositionInMegaArray.erase(it);

							//std::cout << "Created by DELETING BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex << ", " << numVoxelsPerFace << std::endl;
							usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });

							return true;

						}
						else {
							it++;
						}
					}
					
					DefragUntilSize(numVoxelsPerFace);
					std::cout << "Failed to find bucket with valid size." << std::endl;

					//return false;
				}
			}
			return false;
		}
	}

	void Defrag() {
		
		unsigned int numDefragItterations = 1;
		for (int i = 0; i < numDefragItterations; i++)
		{
			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
			current_it++;

			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {

				auto prev_it = std::prev(current_it);
				if (prev_it->indexOffsetIntoMegaArray + prev_it->sizeInIndexCountInMegaArray == current_it->indexOffsetIntoMegaArray) {

					prev_it->sizeInIndexCountInMegaArray += current_it->sizeInIndexCountInMegaArray;

					current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);

					current_it--;
				}
				current_it++;
			}
		}
	}

	void DefragUntilSize(unsigned int sizeNeeded) {
		
		while(true)
		{
			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
			current_it++;

			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {

				auto prev_it = std::prev(current_it);
				if (prev_it->indexOffsetIntoMegaArray + prev_it->sizeInIndexCountInMegaArray == current_it->indexOffsetIntoMegaArray) {

					prev_it->sizeInIndexCountInMegaArray += current_it->sizeInIndexCountInMegaArray;

					current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);

					current_it--;

					if (prev_it->sizeInIndexCountInMegaArray >= sizeNeeded) {
						return;
					}
				}
				current_it++;
			}
		}
	}

};