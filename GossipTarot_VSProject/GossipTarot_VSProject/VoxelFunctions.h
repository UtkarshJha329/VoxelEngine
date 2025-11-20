#pragma once

#include <iostream>

#include "GLMIncludes.h"
#include "MeshUtils.h"

#include "BakedMeshData.h"
#include "VoxelsDataPool.h"
#include "ChunksPerFaceDrawElementsIndirectCommands.h"

bool VoxelIndexLiesInsideChunk(const Vector3Int& chunkSizeInVoxels, const Vector3Int& curVoxelIndex) {

	return !((curVoxelIndex.x < 0 ||
				curVoxelIndex.y < 0 ||
				curVoxelIndex.z < 0 ||
				curVoxelIndex.x >= chunkSizeInVoxels.x ||
				curVoxelIndex.y >= chunkSizeInVoxels.y ||
				curVoxelIndex.z >= chunkSizeInVoxels.z));

}

bool NoiseExistsInNeighbour(const std::vector<float>& chunkNoise, const Vector3Int& curChunkIndex, const Vector3Int& chunkSizeInVoxels, const Vector3Int& worldSizeInChunks, const Vector3Int& curVoxelIndex, const Vector3Int& directionToCheck) {

	Vector3Int neighbouringVoxelIndex = curVoxelIndex + directionToCheck;

	if (VoxelIndexLiesInsideChunk(chunkSizeInVoxels, neighbouringVoxelIndex)) {
		int neightbourNoiseIndex = neighbouringVoxelIndex.x + (neighbouringVoxelIndex.z * chunkSizeInVoxels.x);
		float voxelHeight = chunkNoise[neightbourNoiseIndex] * chunkSizeInVoxels.y * worldSizeInChunks.y;

		return ((curChunkIndex.y * chunkSizeInVoxels.y) + neighbouringVoxelIndex.y <= voxelHeight);
	}
	return false;
}

void AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(std::vector<unsigned int>& compressedChunkVoxelPositions, unsigned int& numIndices, unsigned int curVoxelCompactPos, const unsigned int& bitshiftedFaceID) {

	curVoxelCompactPos += bitshiftedFaceID;

	unsigned int baseIndex = compressedChunkVoxelPositions.size();
	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);

	numIndices += 3;
}

void GenerateCommonChunkMeshOnGPU(const Vector3Int& chunkSizeInVoxels, MeshOnGPU& meshOnGPU) {

	unsigned int totalNumFaces = (chunkSizeInVoxels.y / 2) * (chunkSizeInVoxels.z / 2) * (chunkSizeInVoxels.x / 2) * 6;
	unsigned int totalNumIndices = totalNumFaces * 3;

	std::vector<unsigned int> indices(totalNumIndices);

	for (unsigned int i = 0; i < totalNumFaces; i++)
	{
		unsigned int currentIndex = i << 2;
		indices[(3 * i) + 0] = (currentIndex + 0);
		indices[(3 * i) + 1] = (currentIndex + 1);
		indices[(3 * i) + 2] = (currentIndex + 2);
	}

	glGenVertexArrays(1, &meshOnGPU.VAO);
	glBindVertexArray(meshOnGPU.VAO);

	glGenBuffers(1, &meshOnGPU.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshOnGPU.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
}

void WriteFaceVoxelDataToFreeBucketAndFillMetadata(VoxelsDataPool& voxelsDataPool,	std::vector<unsigned int>& compressedChunkFaceVoxelPositions, FaceVoxelsDataPoolMetadata& curChunkFaceVoxelsDataPoolMetadata, unsigned int& numFaceIndices) {
	if (voxelsDataPool.WriteFaceVoxelDataToFreeBucket(compressedChunkFaceVoxelPositions, curChunkFaceVoxelsDataPoolMetadata)) {
		curChunkFaceVoxelsDataPoolMetadata.numVoxelDataInBucket = numFaceIndices;
	}
	else {
		std::cout << "ERROR : " << "Failed to write data to voxel data pool." << std::endl;
	}
}

void GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPoolForIndirectDrawCommands(const std::vector<float>& chunkNoise, const Vector3Int& chunkIndex, const Vector3Int& chunkSizeInVoxels, const Vector3Int& worldSizeInChunks, const int& curChunkLODLevel, VoxelsDataPool& voxelsDataPool, ChunkVoxelsDataPoolMetadata& curChunkVoxelsDataPoolMetadata) {

	const unsigned int bitShiftPosX = 0;
	const unsigned int bitShiftPosY = 5;
	const unsigned int bitShiftPosZ = 10;
	const unsigned int bitShiftPosFace = 15;

	//const unsigned int bitShiftPosX = 0;
	//const unsigned int bitShiftPosY = 6;
	//const unsigned int bitShiftPosZ = 12;
	//const unsigned int bitShiftPosFace = 18;

	const unsigned int bitShiftFaceIDTop = 0;
	const unsigned int bitShiftFaceIDBottom = 1;
	const unsigned int bitShiftFaceIDLeft = 2;
	const unsigned int bitShiftFaceIDRight = 3;
	const unsigned int bitShiftFaceIDFront = 4;
	const unsigned int bitShiftFaceIDBack = 5;

	std::vector<unsigned int> compressedChunkTopFaceVoxelPositions;
	std::vector<unsigned int> compressedChunkBottomFaceVoxelPositions;
	std::vector<unsigned int> compressedChunkLeftFaceVoxelPositions;
	std::vector<unsigned int> compressedChunkRightFaceVoxelPositions;
	std::vector<unsigned int> compressedChunkFrontFaceVoxelPositions;
	std::vector<unsigned int> compressedChunkBackFaceVoxelPositions;
	unsigned int numTopFaceIndicesInChunk = 0;
	unsigned int numBottomFaceIndicesInChunk = 0;
	unsigned int numLeftFaceIndicesInChunk = 0;
	unsigned int numRightFaceIndicesInChunk = 0;
	unsigned int numFrontFaceIndicesInChunk = 0;
	unsigned int numBackFaceIndicesInChunk = 0;


	int currentVoxelLODSize = pow(2, curChunkLODLevel);

	for (int z = 0; z < chunkSizeInVoxels.z; z += currentVoxelLODSize) {
		for (int x = 0; x < chunkSizeInVoxels.x; x += currentVoxelLODSize) {
			int currentNoiseIndex = x + (z * chunkSizeInVoxels.x);
			float voxelHeight = chunkNoise[currentNoiseIndex] * (chunkSizeInVoxels.y * worldSizeInChunks.y);

			//if (voxelHeight > 80) {
			//	std::cout << voxelHeight << std::endl;
			//}

			for (int y = 0; y < chunkSizeInVoxels.y; y += currentVoxelLODSize)
			{
				if ((chunkSizeInVoxels.y * chunkIndex.y) + y <= voxelHeight) {
					unsigned int curVoxelCompactPos = (x << bitShiftPosX);
					curVoxelCompactPos += (y << bitShiftPosY);
					curVoxelCompactPos += (z << bitShiftPosZ);

					if (!NoiseExistsInNeighbour(chunkNoise, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, Vector3Int{ x, y, z }, Vector3Int{ 0, 1, 0 } * currentVoxelLODSize)) {
						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(compressedChunkTopFaceVoxelPositions, numTopFaceIndicesInChunk, curVoxelCompactPos, (bitShiftFaceIDTop << bitShiftPosFace));
					}
					if (!NoiseExistsInNeighbour(chunkNoise, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, Vector3Int{ x, y, z }, Vector3Int{ 0, -1, 0 } * currentVoxelLODSize)) {
						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(compressedChunkBottomFaceVoxelPositions, numBottomFaceIndicesInChunk, curVoxelCompactPos, (bitShiftFaceIDBottom << bitShiftPosFace));
					}
					if (!NoiseExistsInNeighbour(chunkNoise, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, Vector3Int{ x, y, z }, Vector3Int{ -1, 0, 0 } * currentVoxelLODSize)) {
						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(compressedChunkLeftFaceVoxelPositions, numLeftFaceIndicesInChunk, curVoxelCompactPos, (bitShiftFaceIDLeft << bitShiftPosFace));
					}
					if (!NoiseExistsInNeighbour(chunkNoise, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, Vector3Int{ x, y, z }, Vector3Int{ 1, 0, 0 } * currentVoxelLODSize)) {
						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(compressedChunkRightFaceVoxelPositions, numRightFaceIndicesInChunk, curVoxelCompactPos, (bitShiftFaceIDRight << bitShiftPosFace));
					}
					if (!NoiseExistsInNeighbour(chunkNoise, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, 1 } * currentVoxelLODSize)) {
						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(compressedChunkFrontFaceVoxelPositions, numFrontFaceIndicesInChunk, curVoxelCompactPos, (bitShiftFaceIDFront << bitShiftPosFace));
					}
					if (!NoiseExistsInNeighbour(chunkNoise, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, -1 } * currentVoxelLODSize)) {
						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBOOfIndirectDrawCommands(compressedChunkBackFaceVoxelPositions, numBackFaceIndicesInChunk, curVoxelCompactPos, (bitShiftFaceIDBack << bitShiftPosFace));
					}
				}
			}
		}
	}

	WriteFaceVoxelDataToFreeBucketAndFillMetadata(voxelsDataPool, compressedChunkTopFaceVoxelPositions, curChunkVoxelsDataPoolMetadata.topFaceVoxelsDataPoolMetadata, numTopFaceIndicesInChunk);
	WriteFaceVoxelDataToFreeBucketAndFillMetadata(voxelsDataPool, compressedChunkBottomFaceVoxelPositions, curChunkVoxelsDataPoolMetadata.bottomFaceVoxelsDataPoolMetadata, numBottomFaceIndicesInChunk);
	WriteFaceVoxelDataToFreeBucketAndFillMetadata(voxelsDataPool, compressedChunkLeftFaceVoxelPositions, curChunkVoxelsDataPoolMetadata.leftFaceVoxelsDataPoolMetadata, numLeftFaceIndicesInChunk);
	WriteFaceVoxelDataToFreeBucketAndFillMetadata(voxelsDataPool, compressedChunkRightFaceVoxelPositions, curChunkVoxelsDataPoolMetadata.rightFaceVoxelsDataPoolMetadata, numRightFaceIndicesInChunk);
	WriteFaceVoxelDataToFreeBucketAndFillMetadata(voxelsDataPool, compressedChunkFrontFaceVoxelPositions, curChunkVoxelsDataPoolMetadata.frontFaceVoxelsDataPoolMetadata, numFrontFaceIndicesInChunk);
	WriteFaceVoxelDataToFreeBucketAndFillMetadata(voxelsDataPool, compressedChunkBackFaceVoxelPositions, curChunkVoxelsDataPoolMetadata.backFaceVoxelsDataPoolMetadata, numBackFaceIndicesInChunk);
}

void CPU_WriteFaceDataToDrawCommands(const unsigned int& packedChunkIndex, const FaceVoxelsDataPoolMetadata& faceVoxelsDataPoolMetadata, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands) {
	chunksPerFaceIndirectDrawCommands.cpu_drawElementsIndirectCommands[chunksPerFaceIndirectDrawCommands.numDrawCommandsFilled] = {
		.count = faceVoxelsDataPoolMetadata.numVoxelDataInBucket,
		.instanceCount = 1,
		.firstIndex = 0,
		.baseVertex = static_cast<int>(faceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArray),
		.baseInstance = packedChunkIndex
	};

	chunksPerFaceIndirectDrawCommands.numDrawCommandsFilled++;
}

void WriteChunkDataToDrawCommand(const ChunkVoxelsDataPoolMetadata& curChunkVoxelsDataPoolMetadata, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands) {

	CPU_WriteFaceDataToDrawCommands(curChunkVoxelsDataPoolMetadata.packedChunkIndex, curChunkVoxelsDataPoolMetadata.topFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
	CPU_WriteFaceDataToDrawCommands(curChunkVoxelsDataPoolMetadata.packedChunkIndex, curChunkVoxelsDataPoolMetadata.bottomFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
	CPU_WriteFaceDataToDrawCommands(curChunkVoxelsDataPoolMetadata.packedChunkIndex, curChunkVoxelsDataPoolMetadata.leftFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
	CPU_WriteFaceDataToDrawCommands(curChunkVoxelsDataPoolMetadata.packedChunkIndex, curChunkVoxelsDataPoolMetadata.rightFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
	CPU_WriteFaceDataToDrawCommands(curChunkVoxelsDataPoolMetadata.packedChunkIndex, curChunkVoxelsDataPoolMetadata.frontFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
	CPU_WriteFaceDataToDrawCommands(curChunkVoxelsDataPoolMetadata.packedChunkIndex, curChunkVoxelsDataPoolMetadata.backFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
}

void FillDrawCommandsForChunksBasedOnCameraViewDirection(const Vector3& cameraForward, VoxelsDataPool& voxelsDataPool, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, std::vector<ChunkVoxelsDataPoolMetadata> chunksVoxelsDataPoolMetadatas) {

	Vector3 cameraPointingDirectionNormalised = glm::normalize(cameraForward);

	//std::cout << cameraPointingDirectionNormalised.x << ", " << cameraPointingDirectionNormalised.y << ", " << cameraPointingDirectionNormalised.z << std::endl;

	float topFaceDot = glm::dot(Vector3{ 0, 1, 0 }, cameraPointingDirectionNormalised);
	float bottomFaceDot = glm::dot(Vector3{ 0, -1, 0 }, cameraPointingDirectionNormalised);
	float leftFaceDot = glm::dot(Vector3{ -1, 0, 0 }, cameraPointingDirectionNormalised);
	float rightFaceDot = glm::dot(Vector3{ 1, 0, 0 }, cameraPointingDirectionNormalised);
	float frontFaceDot = glm::dot(Vector3{ 0, 0, 1 }, cameraPointingDirectionNormalised);
	float backFaceDot = glm::dot(Vector3{ 0, 0, -1 }, cameraPointingDirectionNormalised);

	chunksPerFaceIndirectDrawCommands.numDrawCommandsFilled = 0;

	if (topFaceDot <= 0.0f) {
		//std::cout << "Can see top faces." << topFaceDot << std::endl;
		for (int i = 0; i < chunksVoxelsDataPoolMetadatas.size(); i++)
		{
			CPU_WriteFaceDataToDrawCommands(chunksVoxelsDataPoolMetadatas[i].packedChunkIndex, chunksVoxelsDataPoolMetadatas[i].topFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
		}
	}
	if (bottomFaceDot <= 0.0f) {
		//std::cout << "Can see bottom faces." << bottomFaceDot << std::endl;
		for (int i = 0; i < chunksVoxelsDataPoolMetadatas.size(); i++)
		{
			CPU_WriteFaceDataToDrawCommands(chunksVoxelsDataPoolMetadatas[i].packedChunkIndex, chunksVoxelsDataPoolMetadatas[i].bottomFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
		}
	}
	if (leftFaceDot <= 0.0f) {
		//std::cout << "Can see left faces." << leftFaceDot << std::endl;
		for (int i = 0; i < chunksVoxelsDataPoolMetadatas.size(); i++)
		{
			CPU_WriteFaceDataToDrawCommands(chunksVoxelsDataPoolMetadatas[i].packedChunkIndex, chunksVoxelsDataPoolMetadatas[i].leftFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
		}
	}
	if (rightFaceDot <= 0.0f) {
		//std::cout << "Can see right faces." << rightFaceDot << std::endl;
		for (int i = 0; i < chunksVoxelsDataPoolMetadatas.size(); i++)
		{
			CPU_WriteFaceDataToDrawCommands(chunksVoxelsDataPoolMetadatas[i].packedChunkIndex, chunksVoxelsDataPoolMetadatas[i].rightFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
		}
	}
	if (frontFaceDot <= 0.0f) {
		//std::cout << "Can see front faces." << frontFaceDot << std::endl;
		for (int i = 0; i < chunksVoxelsDataPoolMetadatas.size(); i++)
		{
			CPU_WriteFaceDataToDrawCommands(chunksVoxelsDataPoolMetadatas[i].packedChunkIndex, chunksVoxelsDataPoolMetadatas[i].frontFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
		}
	}
	if (backFaceDot <= 0.0f) {
		//std::cout << "Can see back faces." << backFaceDot << std::endl;
		for (int i = 0; i < chunksVoxelsDataPoolMetadatas.size(); i++)
		{
			CPU_WriteFaceDataToDrawCommands(chunksVoxelsDataPoolMetadatas[i].packedChunkIndex, chunksVoxelsDataPoolMetadatas[i].backFaceVoxelsDataPoolMetadata, chunksPerFaceIndirectDrawCommands);
		}
	}

	chunksPerFaceIndirectDrawCommands.GPU_UpdateIndirectCommandsBuffer(chunksPerFaceIndirectDrawCommands.numDrawCommandsFilled);
	//std::cout << chunksPerFaceIndirectDrawCommands.numDrawCommandsFilled << std::endl;

}

//void AddChunkToDrawCommand(const Vector3Int& chunkIndex, const ChunkVoxelsDataPoolMetadata& curChunkVoxelsDataPoolMetadata, IndirectDrawCommands& indirectDrawcommands) {
//
//	unsigned int packedChunkIndex = chunkIndex.x + (chunkIndex.y << 5) + (chunkIndex.z << 10);
//
//	indirectDrawcommands.cpu_drawElementsIndirectCommands.push_back({
//		.count = curChunkVoxelsDataPoolMetadata.numVoxelDataInBucket,
//		.instanceCount = 1,
//		.firstIndex = 0,
//		.baseVertex = static_cast<int>(curChunkVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArray),
//		//.baseVertex = 0,
//		.baseInstance = packedChunkIndex
//		});
//}


//void AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(std::vector<unsigned int>& compressedChunkVoxelPositions, std::vector<unsigned int>& indices, unsigned int curVoxelCompactPos, const unsigned int& bitshiftedFaceID) {
//
//	curVoxelCompactPos += bitshiftedFaceID;
//
//	unsigned int baseIndex = compressedChunkVoxelPositions.size();
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//
//	unsigned int currentIndex = baseIndex << 2;
//	indices.push_back(currentIndex + 0);
//	indices.push_back(currentIndex + 1);
//	indices.push_back(currentIndex + 2);
//}

//void GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPool(const std::vector<float>& chunkNoise, const Vector3Int& chunkSizeInVoxels, VoxelsDataPool& voxelsDataPool, MeshOnGPU& meshOnGPU, unsigned int& numIndicesInChunk, ChunkVoxelsDataPoolMetadata& curChunkVoxelsDataPoolMetadata) {
//
//	const unsigned int bitShiftPosX = 0;
//	const unsigned int bitShiftPosY = 5;
//	const unsigned int bitShiftPosZ = 10;
//	const unsigned int bitShiftPosFace = 15;
//
//	//const unsigned int bitShiftPosX = 0;
//	//const unsigned int bitShiftPosY = 6;
//	//const unsigned int bitShiftPosZ = 12;
//	//const unsigned int bitShiftPosFace = 18;
//
//	const unsigned int bitShiftFaceIDTop = 0;
//	const unsigned int bitShiftFaceIDBottom = 1;
//	const unsigned int bitShiftFaceIDLeft = 2;
//	const unsigned int bitShiftFaceIDRight = 3;
//	const unsigned int bitShiftFaceIDFront = 4;
//	const unsigned int bitShiftFaceIDBack = 5;
//
//	numIndicesInChunk = 0;
//	std::vector<unsigned int> compressedChunkVoxelPositions;
//	std::vector<unsigned int> indices;
//
//	for (int z = 0; z < chunkSizeInVoxels.z; z++) {
//		for (int x = 0; x < chunkSizeInVoxels.x; x++) {
//			int currentNoiseIndex = x + (z * chunkSizeInVoxels.x);
//			float voxelHeight = chunkNoise[currentNoiseIndex] * chunkSizeInVoxels.y;
//
//			for (int y = 0; y < chunkSizeInVoxels.y; y++)
//			{
//				if (y <= voxelHeight) {
//					unsigned int curVoxelCompactPos = (x << bitShiftPosX);
//					curVoxelCompactPos += (y << bitShiftPosY);
//					curVoxelCompactPos += (z << bitShiftPosZ);
//
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDTop << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, -1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBottom << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ -1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDLeft << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDRight << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, 1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDFront << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, -1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBack << bitShiftPosFace));
//					}
//				}
//			}
//		}
//	}
//
//	glGenVertexArrays(1, &meshOnGPU.VAO);
//	glBindVertexArray(meshOnGPU.VAO);
//
//	glGenBuffers(1, &meshOnGPU.EBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshOnGPU.EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
//
//	numIndicesInChunk = indices.size();
//
//	if (voxelsDataPool.WriteDataToFreePool(compressedChunkVoxelPositions, curChunkVoxelsDataPoolMetadata)) {
//		curChunkVoxelsDataPoolMetadata.numVoxelDataInPool = indices.size();
//	}
//	else {
//		std::cout << "ERROR : " << "Failed to write data to voxel data pool." << std::endl;
//	}
//}

//void GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangle(const std::vector<float>& chunkNoise, const Vector3Int& chunkSizeInVoxels, MeshOnGPU& meshOnGPU, unsigned int& numIndicesInChunk, unsigned int& megaVoxelsQuadPositionsBufferObjectID) {
//
//	const unsigned int bitShiftPosX = 0;
//	const unsigned int bitShiftPosY = 5;
//	const unsigned int bitShiftPosZ = 10;
//	const unsigned int bitShiftPosFace = 15;
//
//	//const unsigned int bitShiftPosX = 0;
//	//const unsigned int bitShiftPosY = 6;
//	//const unsigned int bitShiftPosZ = 12;
//	//const unsigned int bitShiftPosFace = 18;
//
//	const unsigned int bitShiftFaceIDTop = 0;
//	const unsigned int bitShiftFaceIDBottom = 1;
//	const unsigned int bitShiftFaceIDLeft = 2;
//	const unsigned int bitShiftFaceIDRight = 3;
//	const unsigned int bitShiftFaceIDFront = 4;
//	const unsigned int bitShiftFaceIDBack = 5;
//
//	numIndicesInChunk = 0;
//	std::vector<unsigned int> compressedChunkVoxelPositions;
//	std::vector<unsigned int> indices;
//
//	for (int z = 0; z < chunkSizeInVoxels.z; z++) {
//		for (int x = 0; x < chunkSizeInVoxels.x; x++) {
//			int currentNoiseIndex = x + (z * chunkSizeInVoxels.x);
//			float voxelHeight = chunkNoise[currentNoiseIndex] * chunkSizeInVoxels.y;
//
//			for (int y = 0; y < chunkSizeInVoxels.y; y++)
//			{
//				if (y <= voxelHeight) {
//					unsigned int curVoxelCompactPos = (x << bitShiftPosX);
//					curVoxelCompactPos += (y << bitShiftPosY);
//					curVoxelCompactPos += (z << bitShiftPosZ);
//
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDTop << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, -1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBottom << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ -1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDLeft << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDRight << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, 1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDFront << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, -1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangleForSSBO(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBack << bitShiftPosFace));
//					}
//				}
//			}
//		}
//	}
//
//	glGenVertexArrays(1, &meshOnGPU.VAO);
//	glBindVertexArray(meshOnGPU.VAO);
//
//	glGenBuffers(1, &meshOnGPU.EBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshOnGPU.EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
//
//	numIndicesInChunk = indices.size();
//
//	size_t sizeOfQuadPositionsBufferInBytes = compressedChunkVoxelPositions.size() * sizeof(compressedChunkVoxelPositions[0]);
//	glCreateBuffers(1, &megaVoxelsQuadPositionsBufferObjectID);
//	glNamedBufferStorage(megaVoxelsQuadPositionsBufferObjectID, sizeOfQuadPositionsBufferInBytes, nullptr,
//		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
//
//	// Map once
//	void* megaVoxelsQuadPositionsBufferGPUPointer = glMapNamedBufferRange(
//		megaVoxelsQuadPositionsBufferObjectID, 0, sizeOfQuadPositionsBufferInBytes,
//		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
//	);
//
//	memcpy(megaVoxelsQuadPositionsBufferGPUPointer, compressedChunkVoxelPositions.data(), sizeOfQuadPositionsBufferInBytes);
//
//	glUnmapNamedBuffer(megaVoxelsQuadPositionsBufferObjectID);
//}

//
//void GenerateFaceQuadMeshOnGPU(MeshOnGPU& faceQuadMeshOnGPU) {
//
//	MeshOnCPU curFaceMeshOnCPU;
//
//	for (int i = 0; i < 6; i++)
//	{
//		curFaceMeshOnCPU.indices.push_back(QuadIndices[i]);
//	}
//
//	for (int i = 0; i < 20; i++)
//	{
//		curFaceMeshOnCPU.vertices.push_back(QuadVerticesWithTexCoords[i]);
//	};
//
//	CreateMeshOnGPU(curFaceMeshOnCPU, faceQuadMeshOnGPU);
//}

//void GenerateChunkVoxelPositionsOnGPUAsVBO(const MeshOnGPU& meshOnGPU, unsigned int& numVoxelsInChunk, const std::vector<float>& chunkNoise, const Vector3Int& chunkSizeInVoxels) {
//
//	numVoxelsInChunk = 0;
//	std::vector<unsigned int> compressedChunkVoxelPositions;
//
//	for (int z = 0; z < chunkSizeInVoxels.z; z++) {
//		for (int x = 0; x < chunkSizeInVoxels.x; x++) {
//			int currentNoiseIndex = x + z * chunkSizeInVoxels.x;
//			float voxelHeight = chunkNoise[currentNoiseIndex] * chunkSizeInVoxels.y;
//
//			for (int y = 0; y < chunkSizeInVoxels.y; y++)
//			{
//				if (y <= voxelHeight) {
//					unsigned int curVoxelCompactPos = (x << 0);
//					curVoxelCompactPos += (y << 5);
//					curVoxelCompactPos += (z << 10);
//
//					compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//					numVoxelsInChunk++;
//				}
//			}
//		}
//	}
//
//	glBindVertexArray(meshOnGPU.VAO);
//	unsigned int voxelPositionsInstanceVBO;
//	glGenBuffers(1, &voxelPositionsInstanceVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, voxelPositionsInstanceVBO);
//	glBufferData(GL_ARRAY_BUFFER, compressedChunkVoxelPositions.size() * sizeof(compressedChunkVoxelPositions[0]), compressedChunkVoxelPositions.data(), GL_STATIC_DRAW);
//
//	glEnableVertexAttribArray(2);
//	glBindBuffer(GL_ARRAY_BUFFER, voxelPositionsInstanceVBO);
//	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, (void*)0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glVertexAttribDivisor(2, 1);
//}

//void AddBitShiftFaceIDToCompressedVoxelPosition(std::vector<unsigned int>& compressedChunkVoxelPositions, unsigned int curVoxelCompactPos, const unsigned int& bitshiftedFaceID, unsigned int& numQuadsInChunk) {
//	curVoxelCompactPos += bitshiftedFaceID;
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//	numQuadsInChunk++;
//}
//
//void GenerateChunkVoxelInstancePositionsOnGPUAsVBO(const MeshOnGPU& meshOnGPU, unsigned int& numQuadsInChunk, const std::vector<float>& chunkNoise, const Vector3Int& chunkSizeInVoxels) {
//
//	const unsigned int bitShiftPosX = 0;
//	const unsigned int bitShiftPosY = 5;
//	const unsigned int bitShiftPosZ = 10;
//	const unsigned int bitShiftPosFace = 15;
//
//	const unsigned int bitShiftFaceIDTop = 0;
//	const unsigned int bitShiftFaceIDBottom = 1;
//	const unsigned int bitShiftFaceIDLeft = 2;
//	const unsigned int bitShiftFaceIDRight = 3;
//	const unsigned int bitShiftFaceIDFront = 4;
//	const unsigned int bitShiftFaceIDBack = 5;
//
//	numQuadsInChunk = 0;
//	std::vector<unsigned int> compressedChunkVoxelPositions;
//
//	for (int z = 0; z < chunkSizeInVoxels.z; z++) {
//		for (int x = 0; x < chunkSizeInVoxels.x; x++) {
//			int currentNoiseIndex = x + (z * chunkSizeInVoxels.x);
//			float voxelHeight = chunkNoise[currentNoiseIndex] * chunkSizeInVoxels.y;
//
//			for (int y = 0; y < chunkSizeInVoxels.y; y++)
//			{
//				if (y <= voxelHeight) {
//					unsigned int curVoxelCompactPos = (x << bitShiftPosX);
//					curVoxelCompactPos += (y << bitShiftPosY);
//					curVoxelCompactPos += (z << bitShiftPosZ);
//
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPosition(compressedChunkVoxelPositions, curVoxelCompactPos, (bitShiftFaceIDTop << bitShiftPosFace), numQuadsInChunk);
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, -1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPosition(compressedChunkVoxelPositions, curVoxelCompactPos, (bitShiftFaceIDBottom << bitShiftPosFace), numQuadsInChunk);
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ -1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPosition(compressedChunkVoxelPositions, curVoxelCompactPos, (bitShiftFaceIDLeft << bitShiftPosFace), numQuadsInChunk);
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPosition(compressedChunkVoxelPositions, curVoxelCompactPos, (bitShiftFaceIDRight << bitShiftPosFace), numQuadsInChunk);
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, 1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPosition(compressedChunkVoxelPositions, curVoxelCompactPos, (bitShiftFaceIDFront << bitShiftPosFace), numQuadsInChunk);
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, -1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPosition(compressedChunkVoxelPositions, curVoxelCompactPos, (bitShiftFaceIDBack << bitShiftPosFace), numQuadsInChunk);
//					}
//				}
//			}
//		}
//	}
//
//	glBindVertexArray(meshOnGPU.VAO);
//	unsigned int voxelPositionsInstanceVBO;
//	glGenBuffers(1, &voxelPositionsInstanceVBO);
//	glBindBuffer(GL_ARRAY_BUFFER, voxelPositionsInstanceVBO);
//	glBufferData(GL_ARRAY_BUFFER, compressedChunkVoxelPositions.size() * sizeof(compressedChunkVoxelPositions[0]), compressedChunkVoxelPositions.data(), GL_STATIC_DRAW);
//
//	glEnableVertexAttribArray(2);
//	glBindBuffer(GL_ARRAY_BUFFER, voxelPositionsInstanceVBO);
//	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, (void*)0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glVertexAttribDivisor(2, 1);
//}
//
//
//
//
//void AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(std::vector<unsigned int>& compressedChunkVoxelPositions, std::vector<unsigned int>& indices, unsigned int curVoxelCompactPos, const unsigned int& bitshiftedFaceID) {
//
//	curVoxelCompactPos += bitshiftedFaceID;
//
//	unsigned int initialIndex = compressedChunkVoxelPositions.size();
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//
//	indices.push_back(initialIndex + 0);
//	indices.push_back(initialIndex + 1);
//	indices.push_back(initialIndex + 2);
//	indices.push_back(initialIndex + 2);
//	indices.push_back(initialIndex + 3);
//	indices.push_back(initialIndex + 0);
//}
//
//void GenerateChunkVoxelPositionsOnGPUAsVBO(MeshOnGPU& meshOnGPU, unsigned int& numIndicesInChunk, const std::vector<float>& chunkNoise, const Vector3Int& chunkSizeInVoxels) {
//
//	const unsigned int bitShiftPosX = 0;
//	const unsigned int bitShiftPosY = 5;
//	const unsigned int bitShiftPosZ = 10;
//	const unsigned int bitShiftPosFace = 15;
//
//	//const unsigned int bitShiftPosX = 0;
//	//const unsigned int bitShiftPosY = 6;
//	//const unsigned int bitShiftPosZ = 12;
//	//const unsigned int bitShiftPosFace = 18;
//
//	const unsigned int bitShiftFaceIDTop = 0;
//	const unsigned int bitShiftFaceIDBottom = 1;
//	const unsigned int bitShiftFaceIDLeft = 2;
//	const unsigned int bitShiftFaceIDRight = 3;
//	const unsigned int bitShiftFaceIDFront = 4;
//	const unsigned int bitShiftFaceIDBack = 5;
//
//	numIndicesInChunk = 0;
//	std::vector<unsigned int> compressedChunkVoxelPositions;
//	std::vector<unsigned int> indices;
//
//	for (int z = 0; z < chunkSizeInVoxels.z; z++) {
//		for (int x = 0; x < chunkSizeInVoxels.x; x++) {
//			int currentNoiseIndex = x + (z * chunkSizeInVoxels.x);
//			float voxelHeight = chunkNoise[currentNoiseIndex] * chunkSizeInVoxels.y;
//
//			for (int y = 0; y < chunkSizeInVoxels.y; y++)
//			{
//				if (y <= voxelHeight) {
//					unsigned int curVoxelCompactPos = (x << bitShiftPosX);
//					curVoxelCompactPos += (y << bitShiftPosY);
//					curVoxelCompactPos += (z << bitShiftPosZ);
//
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDTop << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, -1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBottom << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ -1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDLeft << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDRight << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, 1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDFront << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, -1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAlongWithExtra3Vertices(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBack << bitShiftPosFace));
//					}
//				}
//			}
//		}
//	}
//
//	glGenVertexArrays(1, &meshOnGPU.VAO);
//	glBindVertexArray(meshOnGPU.VAO);
//
//	glBindVertexArray(meshOnGPU.VAO);
//	glGenBuffers(1, &meshOnGPU.VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, meshOnGPU.VBO);
//	glBufferData(GL_ARRAY_BUFFER, compressedChunkVoxelPositions.size() * sizeof(compressedChunkVoxelPositions[0]), compressedChunkVoxelPositions.data(), GL_STATIC_DRAW);
//
//	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
//	glEnableVertexAttribArray(0);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	glGenBuffers(1, &meshOnGPU.EBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshOnGPU.EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
//
//	numIndicesInChunk = indices.size();
//}
//
//
//
//
//
//void AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(std::vector<unsigned int>& compressedChunkVoxelPositions, std::vector<unsigned int>& indices, unsigned int curVoxelCompactPos, const unsigned int& bitshiftedFaceID) {
//
//	curVoxelCompactPos += bitshiftedFaceID;
//	
//	unsigned int initialIndex = compressedChunkVoxelPositions.size();
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//	compressedChunkVoxelPositions.push_back(curVoxelCompactPos);
//
//	indices.push_back(initialIndex + 0);
//	indices.push_back(initialIndex + 1);
//	indices.push_back(initialIndex + 2);
//}
//
//void GenerateChunkVoxelPositionsOnGPUAsVBOAsTriangle(MeshOnGPU& meshOnGPU, unsigned int& numIndicesInChunk, const std::vector<float>& chunkNoise, const Vector3Int& chunkSizeInVoxels) {
//
//	const unsigned int bitShiftPosX = 0;
//	const unsigned int bitShiftPosY = 5;
//	const unsigned int bitShiftPosZ = 10;
//	const unsigned int bitShiftPosFace = 15;
//
//	//const unsigned int bitShiftPosX = 0;
//	//const unsigned int bitShiftPosY = 6;
//	//const unsigned int bitShiftPosZ = 12;
//	//const unsigned int bitShiftPosFace = 18;
//
//	const unsigned int bitShiftFaceIDTop = 0;
//	const unsigned int bitShiftFaceIDBottom = 1;
//	const unsigned int bitShiftFaceIDLeft = 2;
//	const unsigned int bitShiftFaceIDRight = 3;
//	const unsigned int bitShiftFaceIDFront = 4;
//	const unsigned int bitShiftFaceIDBack = 5;
//
//	numIndicesInChunk = 0;
//	std::vector<unsigned int> compressedChunkVoxelPositions;
//	std::vector<unsigned int> indices;
//
//	for (int z = 0; z < chunkSizeInVoxels.z; z++) {
//		for (int x = 0; x < chunkSizeInVoxels.x; x++) {
//			int currentNoiseIndex = x + (z * chunkSizeInVoxels.x);
//			float voxelHeight = chunkNoise[currentNoiseIndex] * chunkSizeInVoxels.y;
//
//			for (int y = 0; y < chunkSizeInVoxels.y; y++)
//			{
//				if (y <= voxelHeight) {
//					unsigned int curVoxelCompactPos = (x << bitShiftPosX);
//					curVoxelCompactPos += (y << bitShiftPosY);
//					curVoxelCompactPos += (z << bitShiftPosZ);
//
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDTop << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, -1, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBottom << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ -1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDLeft << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 1, 0, 0 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDRight << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, 1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDFront << bitShiftPosFace));
//					}
//					if (!NoiseExistsInNeighbour(chunkNoise, chunkSizeInVoxels, Vector3Int{ x, y, z }, Vector3Int{ 0, 0, -1 })) {
//						AddBitShiftFaceIDToCompressedVoxelPositionAsTriangle(compressedChunkVoxelPositions, indices, curVoxelCompactPos, (bitShiftFaceIDBack << bitShiftPosFace));
//					}
//				}
//			}
//		}
//	}
//
//	glGenVertexArrays(1, &meshOnGPU.VAO);
//	glBindVertexArray(meshOnGPU.VAO);
//
//	glBindVertexArray(meshOnGPU.VAO);
//	glGenBuffers(1, &meshOnGPU.VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, meshOnGPU.VBO);
//	glBufferData(GL_ARRAY_BUFFER, compressedChunkVoxelPositions.size() * sizeof(compressedChunkVoxelPositions[0]), compressedChunkVoxelPositions.data(), GL_STATIC_DRAW);
//
//	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
//	glEnableVertexAttribArray(0);
//
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	glGenBuffers(1, &meshOnGPU.EBO);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshOnGPU.EBO);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
//
//	numIndicesInChunk = indices.size();
//}

