#pragma once

#include <functional>
#include <utility>
#include <atomic>
#include <future>

#include "GLMIncludes.h"
#include "MeshOnGpu.h"

#include "FastNoise/FastNoise.h"
#include "VoxelFunctions.h"

unsigned int GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(const Vector3Int& worldSizeInChunks, const Vector3Int& chunkIndex) {
	return chunkIndex.y * worldSizeInChunks.x * worldSizeInChunks.z + chunkIndex.z * worldSizeInChunks.x + chunkIndex.x;
}

void CreateNoiseWithPadding(std::vector<float>& noiseOutput, FastNoise::SmartNode<FastNoise::FractalFBm>& fnFractal, Vector3Int chunkSizeInVoxels, int LOD_Level, unsigned int i, unsigned int j, unsigned int k) {
	int extraVoxelsOnOneSide = pow(2, LOD_Level);
	int extraVoxelsTotalPadding = 2 * extraVoxelsOnOneSide;

	noiseOutput.resize((chunkSizeInVoxels.x + extraVoxelsTotalPadding) * (chunkSizeInVoxels.z + extraVoxelsTotalPadding));

	Vector3Int chunkStartWorldPos = Vector3Int{ i * chunkSizeInVoxels.x, j * chunkSizeInVoxels.y, k * chunkSizeInVoxels.z };
	fnFractal->GenUniformGrid2D(noiseOutput.data()
		, chunkStartWorldPos.x - extraVoxelsOnOneSide, chunkStartWorldPos.z - extraVoxelsOnOneSide,
		chunkSizeInVoxels.x + extraVoxelsTotalPadding, chunkSizeInVoxels.z + extraVoxelsTotalPadding,
		0.002f, 1337);

}

int CurChunkLODLevel(
	Vector3Int chunkIndex,
	Vector3 chunkPosition,
	Vector3 worldSizeInChunks,
	Vector3 halfChunkSize,
	Vector3 currentCameraChunkPosition) {

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

	int LOD_Level = -1;

	if (xDist <= 1024.0 && zDist <= 1024.0) {
		return 0;
	}
	else if ((xDist > 2524.0) || (zDist > 2524.0)) {
		return 4;
	}
	//else if ((xDist > 256.0) || (zDist > 256.0)) {
	else if ((xDist > 1524.0 && xDist <= 2524.0) || (zDist > 1524.0 && zDist <= 2524.0)) {
		return 3;
	}
	else if ((xDist > 1224.0 && xDist <= 1524.0) || (zDist > 1224.0 && zDist <= 1524.0)) {
		return 2;
	}
	else if ((xDist > 1024.0 && xDist <= 1224.0) || (zDist > 1024.0 && zDist <= 1224.0)) {
	//else if ((xDist > 96.0 && xDist <= 128.0) || (zDist > 96.0 && zDist <= 128.0)) {
		return 1;
	}


	//if (xDist <= 160.0 && zDist <= 160.0) {
	//	return 0;
	//}
	//else if ((xDist > 1024.0) || (zDist > 1024.0)) {
	//	return 4;
	//}
	////else if ((xDist > 256.0) || (zDist > 256.0)) {
	//else if ((xDist > 512.0 && xDist <= 1024) || (zDist > 512.0 && zDist <= 1024)) {
	//	return 3;
	//}
	//else if ((xDist > 256.0 && xDist <= 512.0) || (zDist > 256.0 && zDist <= 512.0)) {
	//	return 2;
	//}
	//else if ((xDist > 160.0 && xDist <= 256.0) || (zDist > 160.0 && zDist <= 256.0)) {
	////else if ((xDist > 96.0 && xDist <= 128.0) || (zDist > 96.0 && zDist <= 128.0)) {
	//	return 1;
	//}


	//if (xDist <= 64.0 && zDist <= 64.0) {
	//	return 0;
	//}
	//else if ((xDist > 512.0) || (zDist > 512.0)) {
	//	return 4;
	//}
	////else if ((xDist > 256.0) || (zDist > 256.0)) {
	//else if ((xDist > 256.0 && xDist <= 512.0) || (zDist > 256.0 && zDist <= 512.0)) {
	//	return 3;
	//}
	//else if ((xDist > 128.0 && xDist <= 256.0) || (zDist > 128.0 && zDist <= 256.0)) {
	//	return 2;
	//}
	//else if ((xDist > 64.0 && xDist <= 128.0) || (zDist > 64.0 && zDist <= 128.0)) {
	////else if ((xDist > 96.0 && xDist <= 128.0) || (zDist > 96.0 && zDist <= 128.0)) {
	//	return 1;
	//}

	return LOD_Level;
}

void GenerateChunkAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(
	std::atomic<int>& numGeneratingChunks,
	const Vector3Int& chunkIndex, std::vector<int>& chunksLODLevel,
	FastNoise::SmartNode<FastNoise::FractalFBm> fnFractal,
	const Vector3& currentCameraChunkPosition,
	const Vector3& halfChunkSize, const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels,
	VoxelsDataPool& voxelsDataPool,
	ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands,
	std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas,
	ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata,
	const Vector3& cameraPosition) {

	int i = chunkIndex.x;
	int j = chunkIndex.y;
	int k = chunkIndex.z;

	{
		bool shouldCreateChunk = false;

		Vector3 chunkPosition = { i, j, k };
		chunkPosition *= chunkSizeInVoxels;

		Vector3 curChunkCentrePosition = chunkPosition + halfChunkSize;

		Vector3Int chunkIndex = Vector3Int{ i, j, k };
		unsigned int flattenedChunkIndex = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

		int LOD_Level = CurChunkLODLevel(chunkIndex, chunkPosition, worldSizeInChunks, halfChunkSize, currentCameraChunkPosition);
		chunksLODLevel[flattenedChunkIndex] = LOD_Level;
		shouldCreateChunk = true;

		if (shouldCreateChunk) {

			std::vector<float> noiseOutput;
			CreateNoiseWithPadding(noiseOutput, fnFractal, chunkSizeInVoxels, LOD_Level, i, j, k);

			Vector3Int chunkIndex = Vector3Int{ i, j, k };
			unsigned int flattenedChunkIndexForVoxelsDataPoolMetadatas = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

			chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas].packedChunkIndex = chunkIndex.x + (chunkIndex.y << 7) + (chunkIndex.z << 14) + (LOD_Level << 21);

			GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPoolForIndirectDrawCommands(noiseOutput, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, LOD_Level, voxelsDataPool, chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas]);

			chunksVoxelsDataPoolMetadata.GPU_UploadChunkVoxelsDataPoolMetadatasToTheGPU(flattenedChunkIndexForVoxelsDataPoolMetadatas);


			numGeneratingChunks.fetch_add(-1);
		}
	}
}

void GenerateChunkWithSpecificLODAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(
	const Vector3Int& chunkIndex, std::vector<int>& chunksCurrentLODLevel, const unsigned int& wantChunkLODLevel,
	FastNoise::SmartNode<FastNoise::FractalFBm> fnFractal,
	const Vector3& currentCameraChunkPosition,
	const Vector3& halfChunkSize, const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels,
	VoxelsDataPool& voxelsDataPool,
	ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands,
	std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas,
	ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata,
	const Vector3& cameraPosition) {

	int i = chunkIndex.x;
	int j = chunkIndex.y;
	int k = chunkIndex.z;

	{
		unsigned int flattenedChunkIndex = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

		chunksCurrentLODLevel[flattenedChunkIndex] = wantChunkLODLevel;

		std::vector<float> noiseOutput;
		CreateNoiseWithPadding(noiseOutput, fnFractal, chunkSizeInVoxels, wantChunkLODLevel, i, j, k);


		unsigned int flattenedChunkIndexForVoxelsDataPoolMetadatas = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

		chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas].packedChunkIndex = chunkIndex.x + (chunkIndex.y << 7) + (chunkIndex.z << 14) + (wantChunkLODLevel << 21);

		GenerateChunkVoxelPositionsOnGPUAsSSBOAsTriangleWithVoxelDataPoolForIndirectDrawCommands(noiseOutput, chunkIndex, chunkSizeInVoxels, worldSizeInChunks, wantChunkLODLevel, voxelsDataPool, chunksVoxelsDataPoolMetadatas[flattenedChunkIndexForVoxelsDataPoolMetadatas]);

		chunksVoxelsDataPoolMetadata.GPU_UploadChunkVoxelsDataPoolMetadatasToTheGPU(flattenedChunkIndexForVoxelsDataPoolMetadatas);

	}
}

void GenerateChunksAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(std::atomic<int>& numGeneratingChunks, const Vector3Int& worldSizeInChunks, const Vector3Int& chunkSizeInVoxels, std::vector<int>& chunksLODLevel, VoxelsDataPool& voxelsDataPool, ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, std::vector<ChunkVoxelsDataPoolMetadata>& chunksVoxelsDataPoolMetadatas, ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata, const Vector3& cameraPosition, std::vector<std::future<void>>& chunkGenerationFutures) {

	Vector3 currentCameraChunkPosition = Vector3(int(cameraPosition.x / (chunkSizeInVoxels.x)), 0.0, int(cameraPosition.z / (chunkSizeInVoxels.z)));
	currentCameraChunkPosition *= chunkSizeInVoxels;

	Vector3 halfChunkSize = Vector3(chunkSizeInVoxels / 2);

	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();

	fnFractal->SetSource(fnSimplex);
	fnFractal->SetOctaveCount(5);


	//#pragma omp parallel
	{
		//#pragma omp for collapse(3) nowait
		for (int j = 0; j < worldSizeInChunks.y; j++)
		{
			for (int k = 0; k < worldSizeInChunks.z; k++)
			{
				for (int i = 0; i < worldSizeInChunks.x; i++)
				{
					//GenerateChunkAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(Vector3Int{ i, j, k }, chunksLODLevel, fnFractal, currentCameraChunkPosition, halfChunkSize, worldSizeInChunks, chunkSizeInVoxels, voxelsDataPool, chunksPerFaceIndirectDrawCommands, chunksVoxelsDataPoolMetadatas, chunksVoxelsDataPoolMetadata, cameraPosition);

					chunkGenerationFutures.push_back(std::async(
						std::launch::async,
						GenerateChunkAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU,
						std::ref(numGeneratingChunks),
						Vector3Int{ i, j, k },
						std::ref(chunksLODLevel),               // <-- ref
						fnFractal,                              // copied
						currentCameraChunkPosition,
						halfChunkSize,
						worldSizeInChunks,
						chunkSizeInVoxels,
						std::ref(voxelsDataPool),               // <-- ref
						std::ref(chunksPerFaceIndirectDrawCommands), // <-- ref
						std::ref(chunksVoxelsDataPoolMetadatas),     // <-- ref
						std::ref(chunksVoxelsDataPoolMetadata),      // <-- ref
						cameraPosition
					));
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

				unsigned int flattenedChunkIndex = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunkIndex);

				int LOD_Level = CurChunkLODLevel(chunkIndex, chunkPosition, worldSizeInChunks, halfChunkSize, currentCameraChunkPosition);
				if (currentChunkLODLevels.at(flattenedChunkIndex) != LOD_Level) {
					newChunkLODLevels.push_back({ chunkIndex, LOD_Level });
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