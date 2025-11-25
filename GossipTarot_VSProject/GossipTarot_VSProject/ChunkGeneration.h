#pragma once

#include <utility>
#include <omp.h>

#include "GLMIncludes.h"
#include "MeshOnGpu.h"

#include "FastNoise/FastNoise.h"
#include "VoxelFunctions.h"

unsigned int GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(const Vector3Int& worldSizeInChunks, const Vector3Int& chunkIndex) {
	return chunkIndex.y * worldSizeInChunks.x * worldSizeInChunks.z + chunkIndex.z * worldSizeInChunks.x + chunkIndex.x;
}

void GenerateChunkAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(const Vector3Int& chunkIndex, std::vector<int>& chunksLODLevel, FastNoise::SmartNode<FastNoise::FractalFBm> fnFractal, const Vector3& currentCameraChunkPosition, const Vector3& halfChunkSize, const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels, VoxelsDataPool& voxelsDataPool, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas, ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata, const Vector3& cameraPosition) {

	int i = chunkIndex.x;
	int j = chunkIndex.y;
	int k = chunkIndex.z;

	{
		bool shouldCreateChunk = false;

		Vector3 chunkPosition = { i, j, k };
		chunkPosition *= chunkSizeInVoxels;

		Vector3 curChunkCentrePosition = chunkPosition + halfChunkSize;

		float xDist = (currentCameraChunkPosition.x - curChunkCentrePosition.x);
		float zDist = (currentCameraChunkPosition.z - curChunkCentrePosition.z);

		float xDistSign = xDist / abs(xDist);
		float zDistSign = zDist / abs(zDist);

		xDist = abs(xDist);
		zDist = abs(zDist);

		int xDistIndex = int(xDist / 32);
		int zDistIndex = int(zDist / 32);

		Vector3Int chunkIndex = Vector3Int{ i, j, k };
		unsigned int flattenedChunkIndex = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

		int LOD_Level = -1;

		if (xDist <= 64.0 && zDist <= 64.0) {

			LOD_Level = 0;
			shouldCreateChunk = true;
			chunksLODLevel[flattenedChunkIndex] = 0;
		}
		//else if ((xDist > 256.0 && xDist <= 512.0) || (zDist > 256.0 && zDist <= 512.0)) {
		else if ((xDist > 256.0) || (zDist > 256.0)) {

			LOD_Level = 3;
			shouldCreateChunk = true;
			chunksLODLevel[flattenedChunkIndex] = 3;
		}
		else if ((xDist > 128.0 && xDist <= 256.0) || (zDist > 128.0 && zDist <= 256.0)) {

			LOD_Level = 2;
			shouldCreateChunk = true;
			chunksLODLevel[flattenedChunkIndex] = 2;
		}
		else if ((xDist > 64.0 && xDist <= 128.0) || (zDist > 64.0 && zDist <= 128.0)) {

			LOD_Level = 1;
			shouldCreateChunk = true;
			chunksLODLevel[flattenedChunkIndex] = 1;
		}

		if (shouldCreateChunk) {

			std::vector<float> noiseOutput(chunkSizeInVoxels.x * chunkSizeInVoxels.z);

			Vector3Int chunkStartWorldPos = Vector3Int{ i * chunkSizeInVoxels.x, j * chunkSizeInVoxels.y, k * chunkSizeInVoxels.z };
			fnFractal->GenUniformGrid2D(noiseOutput.data(), chunkStartWorldPos.x, chunkStartWorldPos.z, chunkSizeInVoxels.x, chunkSizeInVoxels.z, 0.002f, 1337);


			Vector3Int chunkIndex = Vector3Int{ i, j, k };
			unsigned int flattenedChunkIndexForVoxelsDataPoolMetadatas = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

			chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas].packedChunkIndex = chunkIndex.x + (chunkIndex.y << 7) + (chunkIndex.z << 14) + (LOD_Level << 21);

			GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPoolForIndirectDrawCommands(noiseOutput, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, LOD_Level, voxelsDataPool, chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas]);
			//CPU_WriteChunkDataToDrawCommand(chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas], chunksPerFaceIndirectDrawCommands);

			chunksVoxelsDataPoolMetadata.GPU_UploadChunkVoxelsDataPoolMetadatasToTheGPU(flattenedChunkIndexForVoxelsDataPoolMetadatas);
		}
	}
}

void GenerateChunksAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels, std::vector<int>& chunksLODLevel, VoxelsDataPool& voxelsDataPool, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas, ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata, const Vector3& cameraPosition) {

	Vector3 currentCameraChunkPosition = Vector3(int(cameraPosition.x / (chunkSizeInVoxels.x)), 0.0, int(cameraPosition.z / (chunkSizeInVoxels.z)));
	currentCameraChunkPosition *= chunkSizeInVoxels;

	Vector3 halfChunkSize = Vector3(chunkSizeInVoxels / 2);

	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();

	fnFractal->SetSource(fnSimplex);
	fnFractal->SetOctaveCount(5);


	#pragma omp parallel
	{
		#pragma omp for collapse(3) nowait
		for (int j = 0; j < worldSizeInChunks.y; j++)
		{
			for (int k = 0; k < worldSizeInChunks.z; k++)
			{
				for (int i = 0; i < worldSizeInChunks.x; i++)
				{
					GenerateChunkAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(Vector3Int{ i, j, k }, chunksLODLevel, fnFractal, currentCameraChunkPosition, halfChunkSize, worldSizeInChunks, chunkSizeInVoxels, voxelsDataPool, chunksPerFaceIndirectDrawCommands, chunksVoxelsDataPoolMetadatas, chunksVoxelsDataPoolMetadata, cameraPosition);
				}
			}
		}
	}
}

void CheckChunksForLODChanges(std::vector<int>& currentChunkLODLevels, const Vector3& cameraPosition, const Vector3Int& chunkSizeInVoxels, const Vector3Int& worldSizeInChunks, std::vector < std::pair<Vector3Int, int>>& newChunkLODLevels) {

	Vector3 currentCameraChunkPosition = Vector3(int(cameraPosition.x / (chunkSizeInVoxels.x)), 0.0, int(cameraPosition.z / (chunkSizeInVoxels.z)));
	currentCameraChunkPosition *= chunkSizeInVoxels;
	Vector3 halfChunkSize = Vector3(chunkSizeInVoxels / 2);

	for (int j = 0; j < worldSizeInChunks.y; j++)
	{
		for (int k = 0; k < worldSizeInChunks.z; k++)
		{
			for (int i = 0; i < worldSizeInChunks.x; i++)
			{
				Vector3Int chunkIndex = { i, j, k };
				Vector3 chunkPosition = chunkIndex * chunkSizeInVoxels;

				Vector3 curChunkCentrePosition = chunkPosition + halfChunkSize;

				float xDist = (currentCameraChunkPosition.x - curChunkCentrePosition.x);
				float zDist = (currentCameraChunkPosition.z - curChunkCentrePosition.z);

				float xDistSign = xDist / abs(xDist);
				float zDistSign = zDist / abs(zDist);

				xDist = abs(xDist);
				zDist = abs(zDist);

				int xDistIndex = int(xDist / 32);
				int zDistIndex = int(zDist / 32);


				unsigned int flattenedChunkIndex = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);
				if (xDist <= 64.0 && zDist <= 64.0) {
					if (currentChunkLODLevels.at(flattenedChunkIndex) != 0) {
						newChunkLODLevels.push_back({ chunkIndex, 0 });
					}
				}
				else if ((xDist > 256.0) || (zDist > 256.0)) {
					if (currentChunkLODLevels.at(flattenedChunkIndex) != 3) {
						newChunkLODLevels.push_back({ chunkIndex, 3 });
					}
				}
				else if ((xDist > 128.0 && xDist <= 256.0) || (zDist > 128.0 && zDist <= 256.0)) {
					if (currentChunkLODLevels.at(flattenedChunkIndex) != 2) {
						newChunkLODLevels.push_back({ chunkIndex, 2 });
					}
				}
				else if ((xDist > 64.0 && xDist <= 128.0) || (zDist > 64.0 && zDist <= 128.0)) {
					if (currentChunkLODLevels.at(flattenedChunkIndex) != 1) {
						newChunkLODLevels.push_back({ chunkIndex, 1 });
					}
				}
			}
		}
	}
}

//
//void GenerateChunksOnCPUAndWriteVoxelsDataToGPU(const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels, VoxelsDataPool& voxelsDataPool, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas) {
//
//	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
//	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
//
//	fnFractal->SetSource(fnSimplex);
//	fnFractal->SetOctaveCount(5);
//
//	for (unsigned int j = 0; j < worldSizeInChunks.y; j++)
//	{
//		for (unsigned int k = 0; k < worldSizeInChunks.z; k++)
//		{
//			for (unsigned int i = 0; i < worldSizeInChunks.x; i++)
//			{
//				std::vector<float> noiseOutput(chunkSizeInVoxels.x * chunkSizeInVoxels.z);
//
//				Vector3Int chunkStartWorldPos = Vector3Int{ i * chunkSizeInVoxels.x, j * chunkSizeInVoxels.y, k * chunkSizeInVoxels.z };
//				fnFractal->GenUniformGrid2D(noiseOutput.data(), chunkStartWorldPos.x, chunkStartWorldPos.z, chunkSizeInVoxels.x, chunkSizeInVoxels.z, 0.2f, 1337);
//
//				unsigned int numVoxelDatasPerBucket = chunkSizeInVoxels.x * chunkSizeInVoxels.y * chunkSizeInVoxels.z;
//				unsigned int numBuckets = worldSizeInChunks.x * worldSizeInChunks.y * worldSizeInChunks.z;
//				unsigned int megaVoxelsDataBufferObjectBindingLocation = 2;
//
//
//				Vector3Int chunkIndex = Vector3Int{ i, 0, k };
//				unsigned int flattenedChunkIndexForVoxelsDataPoolMetadatas = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);
//
//				chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas].packedChunkIndex = chunkIndex.x + (chunkIndex.y << 5) + (chunkIndex.z << 10);
//				GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPoolForIndirectDrawCommands(noiseOutput, chunkSizeInVoxels, voxelsDataPool, chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas]);
//			}
//		}
//	}
//}