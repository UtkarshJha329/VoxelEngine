#pragma once

#include "GLMIncludes.h"
#include "MeshOnGpu.h"

#include "FastNoise/FastNoise.h"
#include "VoxelFunctions.h"

unsigned int GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(const Vector3Int& worldSizeInChunks, const Vector3Int& chunkIndex) {
	return chunkIndex.y * worldSizeInChunks.x * worldSizeInChunks.z + chunkIndex.z * worldSizeInChunks.x + chunkIndex.x;
}

void GenerateChunksAndAddToIndirectRenderCommandVectorOnCPU(const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels, VoxelsDataPool& voxelsDataPool, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas, const Vector3& cameraPosition) {

	Vector3 currentCameraChunkPosition = Vector3(int(cameraPosition.x / (chunkSizeInVoxels.x)), 0.0, int(cameraPosition.z / (chunkSizeInVoxels.z)));
	currentCameraChunkPosition *= chunkSizeInVoxels;

	Vector3 halfChunkSize = Vector3(chunkSizeInVoxels / 2);

	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();

	fnFractal->SetSource(fnSimplex);
	fnFractal->SetOctaveCount(5);

	for (unsigned int j = 0; j < worldSizeInChunks.y; j++)
	{
		for (unsigned int k = 0; k < worldSizeInChunks.z; k++)
		{
			for (unsigned int i = 0; i < worldSizeInChunks.x; i++)
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


				//if (xDistIndex < 2 && zDistIndex < 2) {
				//	shouldCreateChunk = true;
				//}
				//else if ((xDistIndex == 2 && zDistIndex <= 2 && zDistIndex % 2 == 0) || (zDistIndex == 2 && xDistIndex <= 2 && xDistIndex % 2 == 0)) {
				//	shouldCreateChunk = true;
				//}
				//else if ((xDistIndex == 4 && zDistIndex <= 4 && zDistIndex % 4 == 0) || (zDistIndex == 4 && xDistIndex <= 4 && xDistIndex % 4 == 0)) {
				//	shouldCreateChunk = true;
				//}
				
				int LOD_Level = -1;

				if (xDist <= 64.0 && zDist <= 64.0) {

					LOD_Level = 0;
					shouldCreateChunk = true;
				}
				else if ((xDist > 128.0 && xDist <= 256.0) || (zDist > 128.0 && zDist <= 256.0)) {

					LOD_Level = 2;
					shouldCreateChunk = true;
				}
				else if ((xDist > 64.0 && xDist <= 128.0) || (zDist > 64.0 && zDist <= 128.0)) {

					LOD_Level = 1;
					shouldCreateChunk = true;
				}




				//if (xDistIndex < 2 && zDistIndex < 2) {
				//	shouldCreateChunk = true;
				//}
				//else if ((xDistIndex == 2 && zDistIndex <= 2 && zDistIndex % 2 == 0) || (zDistIndex == 2 && xDistIndex <= 2 && xDistIndex % 2 == 0)) {
				//	shouldCreateChunk = true;
				//}
				//else if ((xDistIndex == 4 && zDistIndex <= 4 && zDistIndex % 4 == 0) || (zDistIndex == 4 && xDistIndex <= 4 && xDistIndex % 4 == 0)) {
				//	shouldCreateChunk = true;
				//}

				if (shouldCreateChunk) {

					std::vector<float> noiseOutput(chunkSizeInVoxels.x * chunkSizeInVoxels.z);

					Vector3Int chunkStartWorldPos = Vector3Int{ i * chunkSizeInVoxels.x, j * chunkSizeInVoxels.y, k * chunkSizeInVoxels.z };
					fnFractal->GenUniformGrid2D(noiseOutput.data(), chunkStartWorldPos.x, chunkStartWorldPos.z, chunkSizeInVoxels.x, chunkSizeInVoxels.z, 0.002f, 1337);


					Vector3Int chunkIndex = Vector3Int{ i, j, k };
					unsigned int flattenedChunkIndexForVoxelsDataPoolMetadatas = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

					chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas].packedChunkIndex = chunkIndex.x + (chunkIndex.y << 5) + (chunkIndex.z << 10);
					GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPoolForIndirectDrawCommands(noiseOutput, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, LOD_Level, voxelsDataPool, chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas]);
					WriteChunkDataToDrawCommand(chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas], chunksPerFaceIndirectDrawCommands);
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