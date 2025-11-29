#pragma once

#include "Shader.h"
#include "Transform.h"
#include "MeshOnGpu.h"

#include "ChunksVoxelsDataPoolMetadata.h"
#include "VoxelsDataPool.h"
#include "ChunksPerFaceDrawElementsIndirectCommands.h"
#include "ChunksVisibilityFromCulling.h"


void RenderQuad(const ShaderProgram& shaderForRendering, const Mat4x4& quadTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU) {

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(quadTransformMatrix));

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);
	glBindVertexArray(meshOnGPU.VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}

void RenderCube(const ShaderProgram& shaderForRendering, const Mat4x4& cubeTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU) {

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeTransformMatrix));

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);
	glBindVertexArray(meshOnGPU.VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

}


void RenderQuadInstance(const ShaderProgram& shaderForRendering, const Mat4x4& globalTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU, const unsigned int& instanceCount) {

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);
	glBindVertexArray(meshOnGPU.VAO);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instanceCount);

}

void RenderCubeInstance(const ShaderProgram& shaderForRendering, const Mat4x4& globalTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU, const unsigned int& instanceCount) {

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);
	glBindVertexArray(meshOnGPU.VAO);
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, instanceCount);

}

void RenderMeshOnGPUWithNumIndices(const ShaderProgram& shaderForRendering, const Mat4x4& globalTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU, const unsigned int& numIndices) {

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);
	glBindVertexArray(meshOnGPU.VAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

}


void RenderMeshOnGPUWithNumIndicesAndSSBO(const ShaderProgram& shaderForRendering, const Mat4x4& globalTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU, const unsigned int& numIndices, const unsigned int& ssboID, const unsigned int& ssboBindingLocation) {

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboBindingLocation, ssboID);

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);
	glBindVertexArray(meshOnGPU.VAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

}

void RenderMeshOnGPUWithDrawElementsIndirectCommands(const ShaderProgram& shaderForRendering, const Mat4x4& globalTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU, const ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, const unsigned int& ssboID, const unsigned int& ssboBindingLocation) {

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboBindingLocation, ssboID);

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);

	glBindVertexArray(meshOnGPU.VAO);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);

	glMultiDrawElementsIndirect(
		GL_TRIANGLES,
		GL_UNSIGNED_INT,												// Type of data in indicesBuffer
		(const void*)0,													// No offset into draw command buffer
		//chunksPerFaceIndirectDrawCommands.cpu_drawElementsIndirectCommands.size(),	// Indirect Buffer Count
		chunksPerFaceIndirectDrawCommands.numDrawCommandsFilled,	// Indirect Buffer Count
		0																// No stride as data is tightly packed, for now.
	);
}

void RenderMeshOnGPUWithDrawElementsIndirectCommandsWithComputeShader(const ShaderProgram& shaderForRendering, const ShaderProgram& computeShader, const Transform& cameraTransform, Camera& mainCamera, const Mat4x4& globalTransformMatrix, const int& textureIndex, const Vector3& worldSizeInChunks, const MeshOnGPU& meshOnGPU, const ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, const VoxelsDataPool& voxelsDataPool, const ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata) {

	glUseProgram(computeShader.shaderProgramID);

	{
		int mainCameraPointingDirectionLoc = glGetUniformLocation(computeShader.shaderProgramID, "cameraPointingDirectionNormalised");
		glUniform3fv(mainCameraPointingDirectionLoc, 1, glm::value_ptr(mainCamera.cameraPointingDirection));

		int worldSizeInChunksLoc = glGetUniformLocation(computeShader.shaderProgramID, "worldSizeInChunks");
		glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBindingPoint, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);


	glDispatchCompute(128, 1, 1);

	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	// Render World Geometry
	glUseProgram(shaderForRendering.shaderProgramID);


	{
		int mainCameraViewLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "view");		
		Mat4x4 viewMatrix = glm::lookAt(cameraTransform.position, glm::normalize(mainCamera.cameraPointingDirection) + cameraTransform.position, mainCamera.cameraUp);
		glUniformMatrix4fv(mainCameraViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	
		int mainCameraProjectionLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "projection");
		glUniformMatrix4fv(mainCameraProjectionLoc, 1, GL_FALSE, glm::value_ptr(mainCamera.GetProjectionMatrix()));

		int cameraWorldVoxelPositionLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "cameraWorldVoxelPosition");
		glUniform3fv(cameraWorldVoxelPositionLoc, 1, glm::value_ptr(cameraTransform.position));

		int worldSizeInChunksLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "worldSizeInChunks");
		glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));

		int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectBindingLocation, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectID);

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);

	glBindVertexArray(meshOnGPU.VAO);
	glBindBuffer(GL_PARAMETER_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);

	glMultiDrawElementsIndirectCount(
		GL_TRIANGLES,
		GL_UNSIGNED_INT,															// Type of data in indicesBuffer
		(const void*)0,																// No offset into draw command buffer
		0,																			// No offset into draw count buffer
		chunksPerFaceIndirectDrawCommands.cpu_drawElementsIndirectCommands.size(),	// Max Draw Count
		sizeof(DrawElementsIndirectCommand)											// Stride.
	);
}


void RenderMeshOnGPUWithDrawElementsIndirectCommandsWithComputeShaderAndCullingComputeShader(
	const ShaderProgram& shaderForRendering, const ShaderProgram& voxelsIndirectDrawComputShader,
	const ShaderProgram& frustumCullingComputeShader, ChunksVisiblityFromCulling& chunksVisibilityFromCulling,
	const ShaderProgram& occlusionCullingComputeShader, const ShaderProgram& occlusionCullingVertAndFragShaders,
	const ShaderProgram& chunkVisibilityResetComputeShader,
	const Transform& cameraTransform, Camera& mainCamera,
	const Mat4x4& globalTransformMatrix,
	const int& textureIndex,
	const Vector3& worldSizeInChunks,
	const MeshOnGPU& meshOnGPU,
	const ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands,
	const VoxelsDataPool& voxelsDataPool, const ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata,
	bool& _freezeCulling, bool& _drawBoundingBox) {

	Vector3Int computeDispatchThreads = { 512, 1, 1 };

	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	glDepthFunc(GL_LEQUAL);


	// Run Indirect Draw Compute Shader On Vissible Chunks
	glUseProgram(voxelsIndirectDrawComputShader.shaderProgramID);

	{
		int mainCameraPointingDirectionLoc = glGetUniformLocation(voxelsIndirectDrawComputShader.shaderProgramID, "cameraPointingDirectionNormalised");
		glUniform3fv(mainCameraPointingDirectionLoc, 1, glm::value_ptr(mainCamera.cameraPointingDirection));

		int worldSizeInChunksLoc = glGetUniformLocation(voxelsIndirectDrawComputShader.shaderProgramID, "worldSizeInChunks");
		glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBindingPoint, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferBindingPoint, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferID);

	glDispatchCompute(computeDispatchThreads.x, computeDispatchThreads.y, computeDispatchThreads.z);
	//glDispatchCompute(128, 1, 1);

	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	// Render World Geometry Using Indirect Draw
	glUseProgram(shaderForRendering.shaderProgramID);


	{
		int mainCameraViewLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "view");
		Mat4x4 viewMatrix = glm::lookAt(cameraTransform.position, glm::normalize(mainCamera.cameraPointingDirection) + cameraTransform.position, mainCamera.cameraUp);
		glUniformMatrix4fv(mainCameraViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

		int mainCameraProjectionLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "projection");
		glUniformMatrix4fv(mainCameraProjectionLoc, 1, GL_FALSE, glm::value_ptr(mainCamera.GetProjectionMatrix()));

		int cameraWorldVoxelPositionLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "cameraWorldVoxelPosition");
		glUniform3fv(cameraWorldVoxelPositionLoc, 1, glm::value_ptr(cameraTransform.position));

		int worldSizeInChunksLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "worldSizeInChunks");
		glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));

		int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectBindingLocation, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectID);

	glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);

	glBindVertexArray(meshOnGPU.VAO);
	glBindBuffer(GL_PARAMETER_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);
	
	//glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glMultiDrawElementsIndirectCount(
		GL_TRIANGLES,
		GL_UNSIGNED_INT,															// Type of data in indicesBuffer
		(const void*)0,																// No offset into draw command buffer
		0,																			// No offset into draw count buffer
		chunksPerFaceIndirectDrawCommands.cpu_drawElementsIndirectCommands.size(),	// Max Draw Count
		sizeof(DrawElementsIndirectCommand)											// Stride.
	);

	//================================================================================================================================================
	//================================================================================================================================================
	bool freezeCulling = _freezeCulling;
	bool drawBoundingBox = _drawBoundingBox;
	//================================================================================================================================================
	//================================================================================================================================================

	if (!freezeCulling) {
		// Reset Chunk Visibility.
		if (true) {
			glUseProgram(chunkVisibilityResetComputeShader.shaderProgramID);

			{
				int worldSizeInChunksLoc = glGetUniformLocation(chunkVisibilityResetComputeShader.shaderProgramID, "worldSizeInChunks");
				glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));

			}

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferBindingPoint, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferID);

			glDispatchCompute(computeDispatchThreads.x, computeDispatchThreads.y, computeDispatchThreads.z);
			glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		}



		//================================================================================================================================================
		//================================================================================================================================================
		bool performFrustumCulling = true;
		//================================================================================================================================================
		//================================================================================================================================================


		if (performFrustumCulling) {

			// Perform Frustum Culling.
			glUseProgram(frustumCullingComputeShader.shaderProgramID);

			{
				int mainCameraPositionLoc = glGetUniformLocation(frustumCullingComputeShader.shaderProgramID, "cameraPosition");
				glUniform3fv(mainCameraPositionLoc, 1, glm::value_ptr(cameraTransform.position));

				int mainCameraPointingDirectionLoc = glGetUniformLocation(frustumCullingComputeShader.shaderProgramID, "cameraPointingDirectionNormalised");
				glUniform3fv(mainCameraPointingDirectionLoc, 1, glm::value_ptr(mainCamera.cameraPointingDirection));

				int worldSizeInChunksLoc = glGetUniformLocation(frustumCullingComputeShader.shaderProgramID, "worldSizeInChunks");
				glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));
			}

			chunksVisibilityFromCulling.UpdateCameraFrustumOnCPUAndGPU(mainCamera, cameraTransform.position);
			chunksVisibilityFromCulling.GPU_BindBuffersNeededForChunksVisibilityDataCalculation();

			glDispatchCompute(computeDispatchThreads.x, computeDispatchThreads.y, computeDispatchThreads.z);
			glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		}

		//================================================================================================================================================
		//================================================================================================================================================
		bool performOcclusionCulling = true;
		//================================================================================================================================================
		//================================================================================================================================================

		if (performOcclusionCulling) {

			// Perform Occlusion Culling.
			glUseProgram(occlusionCullingComputeShader.shaderProgramID);

			{
				int mainCameraPointingDirectionLoc = glGetUniformLocation(occlusionCullingComputeShader.shaderProgramID, "cameraPointingDirectionNormalised");
				glUniform3fv(mainCameraPointingDirectionLoc, 1, glm::value_ptr(mainCamera.cameraPointingDirection));

				int worldSizeInChunksLoc = glGetUniformLocation(occlusionCullingComputeShader.shaderProgramID, "worldSizeInChunks");
				glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));
			}

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBindingPoint, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBufferID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferBindingPoint, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferID);

			glDispatchCompute(computeDispatchThreads.x, computeDispatchThreads.y, computeDispatchThreads.z);

			glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

			// Test Chunk Bounds for visibility And Perform Occlusion Culling.
			glUseProgram(occlusionCullingVertAndFragShaders.shaderProgramID);

			{
				int mainCameraViewLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "view");
				Mat4x4 viewMatrix = glm::lookAt(cameraTransform.position, glm::normalize(mainCamera.cameraPointingDirection) + cameraTransform.position, mainCamera.cameraUp);
				glUniformMatrix4fv(mainCameraViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

				int mainCameraProjectionLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "projection");
				glUniformMatrix4fv(mainCameraProjectionLoc, 1, GL_FALSE, glm::value_ptr(mainCamera.GetProjectionMatrix()));

				int cameraWorldVoxelPositionLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "cameraWorldVoxelPosition");
				glUniform3fv(cameraWorldVoxelPositionLoc, 1, glm::value_ptr(cameraTransform.position));

				int worldSizeInChunksLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "worldSizeInChunks");
				glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));

				int modelLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "model");
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

			}


			//glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);

			glBindVertexArray(meshOnGPU.VAO);
			glBindBuffer(GL_PARAMETER_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectBindingLocation, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectID);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferBindingPoint, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferID);

			glDepthMask(GL_FALSE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			glDisable(GL_CULL_FACE);

			glMultiDrawElementsIndirectCount(
				GL_TRIANGLES,
				GL_UNSIGNED_INT,															// Type of data in indicesBuffer
				(const void*)0,																// No offset into draw command buffer
				0,																			// No offset into draw count buffer
				chunksPerFaceIndirectDrawCommands.cpu_drawElementsIndirectCommands.size(),	// Max Draw Count
				sizeof(DrawElementsIndirectCommand)											// Stride.
			);

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			glDepthMask(GL_TRUE);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glEnable(GL_CULL_FACE);
		}
	}

	if(drawBoundingBox) {
		// Draw bounding box

		glUseProgram(occlusionCullingVertAndFragShaders.shaderProgramID);

		{
			int mainCameraViewLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "view");
			Mat4x4 viewMatrix = glm::lookAt(cameraTransform.position, glm::normalize(mainCamera.cameraPointingDirection) + cameraTransform.position, mainCamera.cameraUp);
			glUniformMatrix4fv(mainCameraViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

			int mainCameraProjectionLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "projection");
			glUniformMatrix4fv(mainCameraProjectionLoc, 1, GL_FALSE, glm::value_ptr(mainCamera.GetProjectionMatrix()));

			int cameraWorldVoxelPositionLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "cameraWorldVoxelPosition");
			glUniform3fv(cameraWorldVoxelPositionLoc, 1, glm::value_ptr(cameraTransform.position));

			int worldSizeInChunksLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "worldSizeInChunks");
			glUniform3fv(worldSizeInChunksLoc, 1, glm::value_ptr(worldSizeInChunks));

			int modelLoc = glGetUniformLocation(occlusionCullingVertAndFragShaders.shaderProgramID, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

		}


		//glBindTexture(GL_TEXTURE_2D, Texture::textures[textureIndex].textureID);

		glBindVertexArray(meshOnGPU.VAO);
		glBindBuffer(GL_PARAMETER_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectBindingLocation, voxelsDataPool.megaVoxelsPerFaceDataBufferObjectID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferBindingPoint, chunksVisibilityFromCulling.gpu_chunksVisibilityDataBufferID);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glMultiDrawElementsIndirectCount(
			GL_TRIANGLES,
			GL_UNSIGNED_INT,															// Type of data in indicesBuffer
			(const void*)0,																// No offset into draw command buffer
			0,																			// No offset into draw count buffer
			chunksPerFaceIndirectDrawCommands.cpu_drawElementsIndirectCommands.size(),	// Max Draw Count
			sizeof(DrawElementsIndirectCommand)											// Stride.
		);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

}