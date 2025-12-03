//#pragma once
//
//#include <utility>
//
//#include <unordered_map>
//#include <map>
//#include <mutex>
//
//#include "TSUnorderedMap.h"
//
//#include "glad/glad.h"
//
//#include "GLMIncludes.h"
//#include "glm/gtc/random.hpp"
//#include "ChunksVoxelsDataPoolMetadata.h"
//
//class LocalMemoryAllocator {
//
//public:
//	struct FreeBucket {
//		unsigned int indexOffsetIntoMegaArray;
//		unsigned int sizeInIndexCountInMegaArray;
//	};
//
//	unsigned int localMemorySizeInIndex;
//	unsigned int memoryIndexOffset;
//
//	std::unordered_map<unsigned int, unsigned int> usedBucketsIndexAndSize;
//	std::map<unsigned int, unsigned int> freeBucketsSortedByPositionInMegaArray;
//	std::mutex m_mutex;
//
//	LocalMemoryAllocator(unsigned int _localMemorySizeInIndex, unsigned int _memoryIndexOffset) {
//
//		localMemorySizeInIndex = _localMemorySizeInIndex;
//		memoryIndexOffset = _memoryIndexOffset;
//
//		freeBucketsSortedByPositionInMegaArray.insert({ memoryIndexOffset, localMemorySizeInIndex });
//	}
//
//	LocalMemoryAllocator& operator=(const LocalMemoryAllocator&) = delete;
//
//	LocalMemoryAllocator(LocalMemoryAllocator&& other) noexcept
//		: localMemorySizeInIndex(other.localMemorySizeInIndex),
//		memoryIndexOffset(other.memoryIndexOffset),
//		usedBucketsIndexAndSize(std::move(other.usedBucketsIndexAndSize)),
//		freeBucketsSortedByPositionInMegaArray(std::move(other.freeBucketsSortedByPositionInMegaArray))
//	{
//		// m_mutex left default-constructed for this instance
//	}
//
//	LocalMemoryAllocator& operator=(LocalMemoryAllocator&& other) noexcept {
//		if (this == &other) return *this;
//		localMemorySizeInIndex = other.localMemorySizeInIndex;
//		memoryIndexOffset = other.memoryIndexOffset;
//		usedBucketsIndexAndSize = std::move(other.usedBucketsIndexAndSize);
//		freeBucketsSortedByPositionInMegaArray = std::move(other.freeBucketsSortedByPositionInMegaArray);
//		// don't try to move the mutex
//		return *this;
//	}
//
//
//	//void MakeBucketAFreeBucket(const unsigned int& bucketIndexToFree) {
//	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {
//
//		std::lock_guard<std::mutex> lock(m_mutex);
//
//		auto it = usedBucketsIndexAndSize.find(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//		if (it != usedBucketsIndexAndSize.end()) {
//
//			FreeBucket newBucket = { curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex, usedBucketsIndexAndSize[curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex] };
//
//			/// GO BACK
//			/// AND TEST 
//			/// number face bucket properly with unordered map for num voxels for index and not WRONGLY input lod.
//
//			freeBucketsSortedByPositionInMegaArray.insert({ newBucket.indexOffsetIntoMegaArray, newBucket.sizeInIndexCountInMegaArray });
//			//auto itFB = freeBucketsSortedByPositionInMegaArray.begin();
//			//while (itFB != freeBucketsSortedByPositionInMegaArray.end() && itFB->indexOffsetIntoMegaArray < newBucket.indexOffsetIntoMegaArray) {
//			//	itFB++;
//			//}
//			//freeBucketsSortedByPositionInMegaArray.insert(itFB, newBucket);
//
//			usedBucketsIndexAndSize.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//
//			curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;
//
//			//std::cout << "D : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << ", numVoxels : " << newBucket.sizeInIndexCountInMegaArray << std::endl;
//
//			//Defrag();
//		}
//		else {
//			std::cout << "Trying to free bucket that was not in use : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << std::endl;
//		}
//
//	}
//
//
//	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {
//
//		if (numVoxelsPerFace > 0) {
//
//			std::lock_guard<std::mutex> lock(m_mutex);
//
//
//
//			while (!freeBucketsSortedByPositionInMegaArray.empty()) {
//				auto it = freeBucketsSortedByPositionInMegaArray.begin();
//
//				while (it != freeBucketsSortedByPositionInMegaArray.end()) {
//					if (it->second > numVoxelsPerFace) {
//
//						freePoolOffsetInMegaVoxelsArrayIndex = it->first;
//
//						unsigned int remainingSize = it->second - numVoxelsPerFace;
//
//						it = freeBucketsSortedByPositionInMegaArray.erase(it);
//						freeBucketsSortedByPositionInMegaArray.insert({ freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace, remainingSize });
//
//						//std::cout << "Created by SPLITTING LARGE BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex  << ", " << numVoxelsPerFace << std::endl;
//
//						usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });
//
//						return true;
//					}
//					else if (it->second == numVoxelsPerFace) {
//						freePoolOffsetInMegaVoxelsArrayIndex = it->first;
//
//						freeBucketsSortedByPositionInMegaArray.erase(it);
//
//						//std::cout << "Created by DELETING BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex << ", " << numVoxelsPerFace << std::endl;
//						usedBucketsIndexAndSize.insert({freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace});
//
//						return true;
//
//					}
//					else {
//						it++;
//					}
//				}
//
//				std::cout << "Failed to find bucket with valid size." << std::endl;
//				// If stuck in an infinite loop after this, it's because defrag until size did not find any bucket suitable for this size even after defrag, MEMORY NEEDS TO BE INCREASED.
//				//DefragUntilSize(numVoxelsPerFace);
//			}
//
//			return false;
//		}
//	}
//
//	void DefragUntilSize(unsigned int sizeNeeded) {
//
//		while (true)
//		{
//			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
//			current_it++;
//
//			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {
//
//				auto prev_it = std::prev(current_it);
//				if (prev_it->first + prev_it->second == current_it->first) {
//
//					prev_it->second += current_it->second;
//
//					current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);
//
//					if (prev_it->second >= sizeNeeded) {
//						return;
//					}
//				}
//				else {
//					current_it++;
//				}
//			}
//		}
//	}
//
//	void DefragNumTimes(unsigned int numTimes) {
//
//		std::lock_guard<std::mutex> lock(m_mutex);
//
//		for (unsigned int i = 0; i < numTimes; i++)
//		{
//			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
//			current_it++;
//
//			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {
//
//				auto prev_it = std::prev(current_it);
//				if (prev_it->first + prev_it->second == current_it->first) {
//
//					prev_it->second += current_it->second;
//
//					current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);
//				}
//				else {
//					current_it++;
//				}
//			}
//		}
//	}
//};
// 
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
// 
//#pragma once
//
//#include <utility>
//
//#include <unordered_map>
//#include <map>
//#include <queue>
//#include <mutex>
//
//#include "TSUnorderedMap.h"
//
//#include "glad/glad.h"
//
//#include "GLMIncludes.h"
//#include "glm/gtc/random.hpp"
//#include "ChunksVoxelsDataPoolMetadata.h"
//
//class LocalMemoryAllocator {
//
//public:
//	struct FreeBucket {
//		unsigned int indexOffsetIntoMegaArray;
//		unsigned int sizeInIndexCountInMegaArray;
//	};
//
//	unsigned int localMemorySizeInIndex;
//	unsigned int memoryIndexOffset;
//
//	std::unordered_map<unsigned int, unsigned int> usedBucketsIndexAndSize;
//	std::map<unsigned int, std::queue<unsigned int>> freeBucketsSortedByPositionInMegaArray;
//	std::mutex m_mutex;
//
//	LocalMemoryAllocator(unsigned int _localMemorySizeInIndex, unsigned int _memoryIndexOffset) {
//
//		localMemorySizeInIndex = _localMemorySizeInIndex;
//		memoryIndexOffset = _memoryIndexOffset;
//
//		freeBucketsSortedByPositionInMegaArray.insert({ localMemorySizeInIndex, std::queue<unsigned int>{} });
//		freeBucketsSortedByPositionInMegaArray[localMemorySizeInIndex].push(memoryIndexOffset);
//	}
//
//	LocalMemoryAllocator& operator=(const LocalMemoryAllocator&) = delete;
//
//	LocalMemoryAllocator(LocalMemoryAllocator&& other) noexcept
//		: localMemorySizeInIndex(other.localMemorySizeInIndex),
//		memoryIndexOffset(other.memoryIndexOffset),
//		usedBucketsIndexAndSize(std::move(other.usedBucketsIndexAndSize)),
//		freeBucketsSortedByPositionInMegaArray(std::move(other.freeBucketsSortedByPositionInMegaArray))
//	{
//		// m_mutex left default-constructed for this instance
//	}
//
//	LocalMemoryAllocator& operator=(LocalMemoryAllocator&& other) noexcept {
//		if (this == &other) return *this;
//		localMemorySizeInIndex = other.localMemorySizeInIndex;
//		memoryIndexOffset = other.memoryIndexOffset;
//		usedBucketsIndexAndSize = std::move(other.usedBucketsIndexAndSize);
//		freeBucketsSortedByPositionInMegaArray = std::move(other.freeBucketsSortedByPositionInMegaArray);
//		// don't try to move the mutex
//		return *this;
//	}
//
//
//	//void MakeBucketAFreeBucket(const unsigned int& bucketIndexToFree) {
//	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {
//
//		std::lock_guard<std::mutex> lock(m_mutex);
//
//		auto it = usedBucketsIndexAndSize.find(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//		if (it != usedBucketsIndexAndSize.end()) {
//
//			FreeBucket newBucket = { curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex, usedBucketsIndexAndSize[curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex] };
//
//			/// GO BACK
//			/// AND TEST 
//			/// number face bucket properly with unordered map for num voxels for index and not WRONGLY input lod.
//
//			unsigned int curMemorySize = it->second;
//			if (freeBucketsSortedByPositionInMegaArray.contains(curMemorySize)) {
//				freeBucketsSortedByPositionInMegaArray[curMemorySize].push(it->first);
//			}
//			else {
//				freeBucketsSortedByPositionInMegaArray.insert({ curMemorySize, std::queue<unsigned int>{} });
//				freeBucketsSortedByPositionInMegaArray[curMemorySize].push(it->first);
//			}			//auto itFB = freeBucketsSortedByPositionInMegaArray.begin();
//			//while (itFB != freeBucketsSortedByPositionInMegaArray.end() && itFB->indexOffsetIntoMegaArray < newBucket.indexOffsetIntoMegaArray) {
//			//	itFB++;
//			//}
//			//freeBucketsSortedByPositionInMegaArray.insert(itFB, newBucket);
//
//			usedBucketsIndexAndSize.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//
//			curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;
//
//			//std::cout << "D : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << ", numVoxels : " << newBucket.sizeInIndexCountInMegaArray << std::endl;
//
//			//Defrag();
//		}
//		else {
//			std::cout << "Trying to free bucket that was not in use : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << std::endl;
//		}
//
//	}
//
//
//	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {
//
//		if (numVoxelsPerFace > 0) {
//
//			std::lock_guard<std::mutex> lock(m_mutex);
//
//
//
//			auto it = freeBucketsSortedByPositionInMegaArray.lower_bound(numVoxelsPerFace);
//
//			if (it != freeBucketsSortedByPositionInMegaArray.end()) {
//				unsigned int curBucketSize = it->first;
//				if (curBucketSize > numVoxelsPerFace) {
//
//					freePoolOffsetInMegaVoxelsArrayIndex = it->second.front();
//					it->second.pop();
//
//					unsigned int remainingSize = curBucketSize - numVoxelsPerFace;
//
//					if (freeBucketsSortedByPositionInMegaArray.contains(remainingSize)) {
//						freeBucketsSortedByPositionInMegaArray[remainingSize].push(freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace);
//					}
//					else {
//						freeBucketsSortedByPositionInMegaArray.insert({ remainingSize, std::queue<unsigned int>{} });
//						freeBucketsSortedByPositionInMegaArray[remainingSize].push(freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace);
//					}
//
//					//std::cout << "Created by SPLITTING LARGE BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex  << ", " << numVoxelsPerFace << std::endl;
//
//					usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });
//
//					return true;
//				}
//				else if (it->first == numVoxelsPerFace) {
//					freePoolOffsetInMegaVoxelsArrayIndex = it->second.front();
//					it->second.pop();
//
//					//std::cout << "Created by DELETING BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex << ", " << numVoxelsPerFace << std::endl;
//					usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });
//
//					return true;
//
//				}
//			}
//
//			std::cout << "Failed to find bucket with valid size." << std::endl;
//			// If stuck in an infinite loop after this, it's because defrag until size did not find any bucket suitable for this size even after defrag, MEMORY NEEDS TO BE INCREASED.
//			//DefragUntilSize(numVoxelsPerFace);
//
//			return false;
//		}
//	}
//};
// 
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//====FREE LIST : MAP==========================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
 
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

#include "Instrumentor.h"

class LocalMemoryAllocator {

public:

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

	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {

		PROFILE_FUNCTION("Make Bucket A Free Bucket");

		std::lock_guard<std::mutex> lock(m_mutex);

		auto currentUsedBucket = usedBucketsIndexAndSize.find(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
		if (currentUsedBucket != usedBucketsIndexAndSize.end()) {

			auto current_it = freeBucketsSortedByPositionInMegaArray.insert({ curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex, currentUsedBucket->second }).first;


			if (current_it != freeBucketsSortedByPositionInMegaArray.begin()) {
				auto prev_it = std::prev(current_it);
				if (prev_it->first + prev_it->second == current_it->first) {
					prev_it->second += currentUsedBucket->second;

					freeBucketsSortedByPositionInMegaArray.erase(current_it);
					current_it = prev_it;
					//std::cout << "\tMerged with previous free block: " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
				}
			}

			if (current_it != freeBucketsSortedByPositionInMegaArray.end()) {
				auto next_it = std::next(current_it);
				if (current_it->first + current_it->second == next_it->first) {
					current_it->second += next_it->second;

					//std::cout << "\tMerged with next free block: " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
					freeBucketsSortedByPositionInMegaArray.erase(next_it);
				}
			}

			curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;
			usedBucketsIndexAndSize.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
		}
		else {
			std::cout << "Trying to free bucket that was not in use : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << std::endl;
		}

	}


	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {

		PROFILE_FUNCTION("Get Free Bucket");

		if (numVoxelsPerFace > 0) {

			std::lock_guard<std::mutex> lock(m_mutex);

			bool found = false;

			while (!found) {

				if (!freeBucketsSortedByPositionInMegaArray.empty()) {

					auto it = freeBucketsSortedByPositionInMegaArray.begin();

					while (it != freeBucketsSortedByPositionInMegaArray.end()) {
						if (it->second == numVoxelsPerFace) {
							freePoolOffsetInMegaVoxelsArrayIndex = it->first;

							freeBucketsSortedByPositionInMegaArray.erase(it);

							//std::cout << "Created by DELETING BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex << ", " << numVoxelsPerFace << std::endl;
							usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });

							return true;

						}
						else if(it->second> numVoxelsPerFace) {

							freePoolOffsetInMegaVoxelsArrayIndex = it->first;

							unsigned int modifiedIndexOfBlock = freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace;
							unsigned int modifiedSizeOfBlock = it->second - numVoxelsPerFace;

							//std::cout << "Created by SPLITTING LARGE BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex  << ", " << numVoxelsPerFace << std::endl;

							freeBucketsSortedByPositionInMegaArray.erase(it);
							freeBucketsSortedByPositionInMegaArray.insert({ modifiedIndexOfBlock, modifiedSizeOfBlock });

							usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });

							return true;
						}
						else {
							it++;
						}
					}

					std::cout << "Failed to find bucket with valid size : " << numVoxelsPerFace<< std::endl;
					DefragUntilSize(numVoxelsPerFace);

					//return false;
				}
				else {
					std::cout << "No more free buckets available." << std::endl;
					return false;
				}
			}

			return false;
		}
	}

	void DefragUntilSize(unsigned int sizeNeeded) {

		PROFILE_FUNCTION("Defrag Until Size");

		//std::cout << "DEFRAGGING for size : " << sizeNeeded << std::endl;

		while (true)
		{
			std::cout << "DEFRAGGING for size : " << sizeNeeded << std::endl;
			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();

			if (current_it->second >= sizeNeeded) {
				//std::cout << "\tFound block : " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
				return;
			}

			current_it++;

			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {

				if (current_it->second>= sizeNeeded) {
					//std::cout << "\tFound block : " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
					return;
				}
				else {
					auto prev_it = std::prev(current_it);
					if (prev_it->first + prev_it->second == current_it->first) {

						prev_it->second += current_it->second;

						current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);

						if (prev_it->second >= sizeNeeded) {
							//std::cout << "\tFound block after merge: " << prev_it->indexOffsetIntoMegaArray << ", " << prev_it->sizeInIndexCountInMegaArray << std::endl;
							return;
						}
					}
					else {
						current_it++;
					}
				}
			}
		}
	}
};
// 
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//====FREE LIST==========================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
//=======================================================================================================================================
// 
//#pragma once
//
//#include <utility>
//
//#include <unordered_map>
//#include <list>
//#include <mutex>
//
//#include "TSUnorderedMap.h"
//
//#include "glad/glad.h"
//
//#include "GLMIncludes.h"
//#include "glm/gtc/random.hpp"
//#include "ChunksVoxelsDataPoolMetadata.h"
//
//class LocalMemoryAllocator {
//
//public:
//	struct FreeBucket {
//		unsigned int indexOffsetIntoMegaArray;
//		unsigned int sizeInIndexCountInMegaArray;
//	};
//
//	unsigned int localMemorySizeInIndex;
//	unsigned int memoryIndexOffset;
//
//	std::unordered_map<unsigned int, unsigned int> usedBucketsIndexAndSize;
//	std::list<FreeBucket> freeBucketsSortedByPositionInMegaArray;
//	std::mutex m_mutex;
//
//	std::list<FreeBucket>::iterator lastAllocatedFromSpot;
//
//	LocalMemoryAllocator(unsigned int _localMemorySizeInIndex, unsigned int _memoryIndexOffset) {
//
//		localMemorySizeInIndex = _localMemorySizeInIndex;
//		memoryIndexOffset = _memoryIndexOffset;
//
//		freeBucketsSortedByPositionInMegaArray.push_back({ memoryIndexOffset, localMemorySizeInIndex });
//		lastAllocatedFromSpot = freeBucketsSortedByPositionInMegaArray.begin();
//	}
//
//	LocalMemoryAllocator& operator=(const LocalMemoryAllocator&) = delete;
//
//	LocalMemoryAllocator(LocalMemoryAllocator&& other) noexcept
//		: localMemorySizeInIndex(other.localMemorySizeInIndex),
//		memoryIndexOffset(other.memoryIndexOffset),
//		usedBucketsIndexAndSize(std::move(other.usedBucketsIndexAndSize)),
//		freeBucketsSortedByPositionInMegaArray(std::move(other.freeBucketsSortedByPositionInMegaArray))
//	{
//		// m_mutex left default-constructed for this instance
//		lastAllocatedFromSpot = freeBucketsSortedByPositionInMegaArray.begin();
//	}
//
//	LocalMemoryAllocator& operator=(LocalMemoryAllocator&& other) noexcept {
//		if (this == &other) return *this;
//		localMemorySizeInIndex = other.localMemorySizeInIndex;
//		memoryIndexOffset = other.memoryIndexOffset;
//		usedBucketsIndexAndSize = std::move(other.usedBucketsIndexAndSize);
//		freeBucketsSortedByPositionInMegaArray = std::move(other.freeBucketsSortedByPositionInMegaArray);
//		lastAllocatedFromSpot = freeBucketsSortedByPositionInMegaArray.begin();
//		// don't try to move the mutex
//		return *this;
//	}
//
//	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {
//
//		std::lock_guard<std::mutex> lock(m_mutex);
//
//		auto it = usedBucketsIndexAndSize.find(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//		if (it != usedBucketsIndexAndSize.end()) {
//
//			FreeBucket tempBucket = { curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex, it->second };
//
//			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
//			while (current_it != freeBucketsSortedByPositionInMegaArray.end() && current_it->indexOffsetIntoMegaArray < tempBucket.indexOffsetIntoMegaArray) {
//				current_it++;
//			}
//			//std::cout << "Added to free list : " << tempBucket.indexOffsetIntoMegaArray << ", " << tempBucket.sizeInIndexCountInMegaArray << std::endl;
//			current_it = freeBucketsSortedByPositionInMegaArray.insert(current_it, tempBucket);
//
//
//			if (current_it != freeBucketsSortedByPositionInMegaArray.begin()) {
//				auto prev_it = std::prev(current_it);
//				if (prev_it->indexOffsetIntoMegaArray + prev_it->sizeInIndexCountInMegaArray == current_it->indexOffsetIntoMegaArray) {
//					prev_it->sizeInIndexCountInMegaArray += tempBucket.sizeInIndexCountInMegaArray;
//
//					if (lastAllocatedFromSpot == current_it) {
//						lastAllocatedFromSpot = prev_it;
//					}
//
//					freeBucketsSortedByPositionInMegaArray.erase(current_it);
//					current_it = prev_it;
//					//std::cout << "\tMerged with previous free block: " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
//				}
//			}
//
//			if (current_it != freeBucketsSortedByPositionInMegaArray.end()) {
//				auto next_it = std::next(current_it);
//				if (current_it->indexOffsetIntoMegaArray + current_it->sizeInIndexCountInMegaArray == next_it->indexOffsetIntoMegaArray) {
//					current_it->sizeInIndexCountInMegaArray += next_it->sizeInIndexCountInMegaArray;
//
//					if (lastAllocatedFromSpot == next_it) {
//						lastAllocatedFromSpot = current_it;
//					}
//
//					//std::cout << "\tMerged with next free block: " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
//					freeBucketsSortedByPositionInMegaArray.erase(next_it);
//				}
//			}
//
//			//auto itFB = freeBucketsSortedByPositionInMegaArray.begin();
//			//while (itFB != freeBucketsSortedByPositionInMegaArray.end() && itFB->indexOffsetIntoMegaArray < tempBucket.indexOffsetIntoMegaArray) {
//			//	itFB++;
//			//}
//			//freeBucketsSortedByPositionInMegaArray.insert(itFB, tempBucket);
//
//			curFaceVoxelDataPoolMetadata.numVoxelDataInBucket = 0;
//			usedBucketsIndexAndSize.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//		}
//		else {
//			std::cout << "Trying to free bucket that was not in use : " << curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex << std::endl;
//		}
//
//	}
//
//
//	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {
//
//		if (numVoxelsPerFace > 0) {
//
//			std::lock_guard<std::mutex> lock(m_mutex);
//
//			bool found = false;
//
//			while (!found) {
//
//				if (!freeBucketsSortedByPositionInMegaArray.empty()) {
//
//					auto it = lastAllocatedFromSpot;
//
//					if (it == freeBucketsSortedByPositionInMegaArray.end()) {
//						//std::cout << "REACHED END OF FREE LIST : STARTING FROM BEGINNING." << std::endl;
//						it = freeBucketsSortedByPositionInMegaArray.begin();
//					}
//
//					while (it != freeBucketsSortedByPositionInMegaArray.end()) {
//						if (it->sizeInIndexCountInMegaArray == numVoxelsPerFace) {
//							freePoolOffsetInMegaVoxelsArrayIndex = it->indexOffsetIntoMegaArray;
//
//							lastAllocatedFromSpot = freeBucketsSortedByPositionInMegaArray.erase(it);
//
//							//std::cout << "Created by DELETING BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex << ", " << numVoxelsPerFace << std::endl;
//							usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });
//
//							return true;
//
//						}
//						else if(it->sizeInIndexCountInMegaArray > numVoxelsPerFace) {
//
//							freePoolOffsetInMegaVoxelsArrayIndex = it->indexOffsetIntoMegaArray;
//
//							it->indexOffsetIntoMegaArray = freePoolOffsetInMegaVoxelsArrayIndex + numVoxelsPerFace;
//							it->sizeInIndexCountInMegaArray = it->sizeInIndexCountInMegaArray - numVoxelsPerFace;
//
//							//std::cout << "Created by SPLITTING LARGE BLOCK : " << freePoolOffsetInMegaVoxelsArrayIndex  << ", " << numVoxelsPerFace << std::endl;
//
//							usedBucketsIndexAndSize.insert({ freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace });
//							lastAllocatedFromSpot = it;
//
//							return true;
//						}
//						else {
//							it++;
//						}
//					}
//
//					std::cout << "Failed to find bucket with valid size : " << numVoxelsPerFace<< std::endl;
//					lastAllocatedFromSpot = DefragUntilSize(numVoxelsPerFace);;
//					
//
//					//return false;
//				}
//				else {
//					std::cout << "No more free buckets available." << std::endl;
//					return false;
//				}
//			}
//
//			return false;
//		}
//	}
//
//	std::list<FreeBucket>::iterator DefragUntilSize(unsigned int sizeNeeded) {
//
//		//std::cout << "DEFRAGGING for size : " << sizeNeeded << std::endl;
//
//		while (true)
//		{
//			std::cout << "DEFRAGGING for size : " << sizeNeeded << std::endl;
//			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
//
//			if (current_it->sizeInIndexCountInMegaArray >= sizeNeeded) {
//				//std::cout << "\tFound block : " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
//				return current_it;
//			}
//
//			current_it++;
//
//			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {
//
//				if (current_it->sizeInIndexCountInMegaArray >= sizeNeeded) {
//					//std::cout << "\tFound block : " << current_it->indexOffsetIntoMegaArray << ", " << current_it->sizeInIndexCountInMegaArray << std::endl;
//					return current_it;
//				}
//				else {
//					auto prev_it = std::prev(current_it);
//					if (prev_it->indexOffsetIntoMegaArray + prev_it->sizeInIndexCountInMegaArray == current_it->indexOffsetIntoMegaArray) {
//
//						prev_it->sizeInIndexCountInMegaArray += current_it->sizeInIndexCountInMegaArray;
//
//						current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);
//
//						if (prev_it->sizeInIndexCountInMegaArray >= sizeNeeded) {
//							//std::cout << "\tFound block after merge: " << prev_it->indexOffsetIntoMegaArray << ", " << prev_it->sizeInIndexCountInMegaArray << std::endl;
//							return prev_it;
//						}
//					}
//					else {
//						current_it++;
//					}
//				}
//			}
//		}
//	}
//
//	void DefragNumTimes(unsigned int numTimes) {
//
//		std::lock_guard<std::mutex> lock(m_mutex);
//
//		for (unsigned int i = 0; i < numTimes; i++)
//		{
//			auto current_it = freeBucketsSortedByPositionInMegaArray.begin();
//			current_it++;
//
//			while (current_it != freeBucketsSortedByPositionInMegaArray.end()) {
//
//				auto prev_it = std::prev(current_it);
//				if (prev_it->indexOffsetIntoMegaArray + prev_it->sizeInIndexCountInMegaArray == current_it->indexOffsetIntoMegaArray) {
//
//					prev_it->sizeInIndexCountInMegaArray += current_it->sizeInIndexCountInMegaArray;
//
//					current_it = freeBucketsSortedByPositionInMegaArray.erase(current_it);
//				}
//				else {
//					current_it++;
//				}
//			}
//		}
//	}
//};
//// 
// 
// ========================================================================================================== 
// ========================================================================================================== 
// SINGLE LOCAL ALLOCATOR 
// ========================================================================================================== 
// ========================================================================================================== 
// ========================================================================================================== 
 
class VoxelsDataPool {

public:

	unsigned int megaVoxelsPerFaceDataBufferObjectID;
	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;

	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;

	unsigned int numVoxelsDataToStore = 40000000;
	size_t sizeOfPoolInBytes;


	unsigned int numLocalAllocators = 1;
	unsigned int currentAllocatorBucketIndex = 0;
	
	LocalMemoryAllocator localMemoryAllocator{ numVoxelsDataToStore, 0 };

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

		localMemoryAllocator.MakeBucketAFreeBucket(curFaceVoxelDataPoolMetadata);
	}

private:

	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {


		if (localMemoryAllocator.GetFreeBucket(freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace)) {
			return true;
		}


		return false;
	}

}; 
// 
// ========================================================================================================== 
// ========================================================================================================== 
// MULTIPLE LOCAL ALLOCATORS 
// ========================================================================================================== 
// ========================================================================================================== 
// ========================================================================================================== 
// 
//class VoxelsDataPool {
//
//public:
//
//	unsigned int megaVoxelsPerFaceDataBufferObjectID;
//	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;
//
//	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;
//
//	unsigned int numVoxelsDataToStore = 40000000;
//	size_t sizeOfPoolInBytes;
//
//
//	unsigned int numLocalAllocators = 40;
//	std::vector<LocalMemoryAllocator> localMemoryAllocators;
//	ThreadSafeUnorderedMap<unsigned int, unsigned int> indexMappedInAllocatorID;
//
//	std::atomic<unsigned int> totalNumAssigned = 0;
//
//	std::atomic<unsigned int> currentAllocatorIndex = 0;
//
//	VoxelsDataPool(unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {
//
//		megaVoxelsPerFaceDataBufferObjectBindingLocation = _megaVoxelsPerFaceDataBufferObjectBindingLocation;
//		sizeOfPoolInBytes = numVoxelsDataToStore * sizeof(unsigned int);
//
//		glCreateBuffers(1, &megaVoxelsPerFaceDataBufferObjectID);
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
//		localMemoryAllocators.reserve(numLocalAllocators);
//		for (unsigned int i = 0; i < numLocalAllocators; i++)
//		{
//			unsigned int allocatorSize = numVoxelsDataToStore / numLocalAllocators;
//			localMemoryAllocators.emplace_back(allocatorSize, i * allocatorSize);
//		}
//
//		std::srand(static_cast<unsigned int>(1667));
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
//			totalNumAssigned.fetch_add(numVoxelsPerFace);
//
//			return true;
//		}
//		else {
//			return false;
//		}
//
//	}
//
//	//void MakeBucketAFreeBucket(const unsigned int& bucketIndexToFree) {
//	void MakeBucketAFreeBucket(FaceVoxelsDataPoolMetadata& curFaceVoxelDataPoolMetadata) {
//
//		if (indexMappedInAllocatorID.contains(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex)) {
//
//			localMemoryAllocators[indexMappedInAllocatorID.get(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex)].MakeBucketAFreeBucket(curFaceVoxelDataPoolMetadata);
//			indexMappedInAllocatorID.erase(curFaceVoxelDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex);
//		}
//		else {
//			std::cout << "Freeing a bucket that is not in use." << std::endl;
//		}
//	}
//
//private:
//
//	bool GetFreeBucket(unsigned int& freePoolOffsetInMegaVoxelsArrayIndex, const unsigned int& numVoxelsPerFace) {
//
//		while (true) {
//			//unsigned int currentRandomAllocatorIndex = glm::linearRand(unsigned int(0), numLocalAllocators - 1);
//			unsigned int currentRandomAllocatorIndex = currentAllocatorIndex.fetch_add(1) % numLocalAllocators;
//
//			if (localMemoryAllocators[currentRandomAllocatorIndex].GetFreeBucket(freePoolOffsetInMegaVoxelsArrayIndex, numVoxelsPerFace)) {
//				indexMappedInAllocatorID.insert_or_assign(freePoolOffsetInMegaVoxelsArrayIndex, currentRandomAllocatorIndex);
//				return true;
//			}
//		}
//		return false;
//	}
//
//};

/// ========================================================================================================================================================== 
/// ========================================================================================================================================================== 
/// ========================================================================================================================================================== 
/// =VOXEL CLASSIFICATION BLOCKS============================================================================================================================== 
/// ========================================================================================================================================================== 
/// ========================================================================================================================================================== 
/// ========================================================================================================================================================== 
//
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
//class VoxelsDataPool {
//
//public:
//
//	unsigned int numClassifications;
//	std::vector<unsigned int> NumVoxelPerFaceClassification;
//	std::vector<unsigned int> perClassificationStats;
//
//	std::vector<unsigned int> currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes;
//
//	unsigned int megaVoxelsPerFaceDataBufferObjectID;
//	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation;
//
//	uint32_t* megaVoxelsPerFaceDataBufferGPUPointer;
//
//	size_t sizeOfPoolInBytes;
//
//	std::vector<TSQueue<unsigned int>> freeDataBucketsPerClassificationPositionInMegaVoxelsArray;
//
//	std::mutex m_mutex;
//
//	VoxelsDataPool(unsigned int _numVoxelsPerFaceClassifications, std::vector<unsigned int> _NumVoxelPerFaceClassification, std::vector<unsigned int> numBucketsPerClassification, unsigned int _megaVoxelsPerFaceDataBufferObjectBindingLocation) {
//
//		numClassifications = _numVoxelsPerFaceClassifications;
//		NumVoxelPerFaceClassification = _NumVoxelPerFaceClassification;
//
//		perClassificationStats.resize(numClassifications);
//
//		currentFaceClassficationOffsetIntoMegaVoxelsArrayInBytes.resize(numClassifications);
//		freeDataBucketsPerClassificationPositionInMegaVoxelsArray.resize(numClassifications);
//
//		sizeOfPoolInBytes = 0;
//		for (int i = 0; i < numClassifications; i++)
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
//		for (unsigned int i = 0; i < numClassifications; i++)
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
//		for (int i = 0; i < numClassifications; i++)
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
//			perClassificationStats[currentClassificationIndex]++;
//			freePoolOffsetInMegaVoxelsArrayIndex = freeDataBucketsPerClassificationPositionInMegaVoxelsArray[currentClassificationIndex].pop();
//			return true;
//		}
//
//		return false;
//	}
//
//};