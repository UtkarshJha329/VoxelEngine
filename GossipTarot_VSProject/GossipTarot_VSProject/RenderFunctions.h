#pragma once

#include "Shader.h"
#include "Transform.h"
#include "MeshOnGpu.h"

#include "ChunksVoxelsDataPoolMetadata.h"
#include "VoxelsDataPool.h"
#include "ChunksPerFaceDrawElementsIndirectCommands.h"

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

void RenderMeshOnGPUWithDrawElementsIndirectCommandsWithComputeShader(const ShaderProgram& shaderForRendering, const ShaderProgram& computeShader, const Transform& cameraTransform, Camera& mainCamera, const Mat4x4& globalTransformMatrix, const int& textureIndex, const MeshOnGPU& meshOnGPU, const ChunksPerFaceIndirectDrawCommands& chunksPerFaceIndirectDrawCommands, const VoxelsDataPool& voxelsDataPool, const ChunksVoxelsDataPoolMetadata& chunksVoxelsDataPoolMetadata) {

	glUseProgram(computeShader.shaderProgramID);

	int mainCameraPointingDirectionLoc = glGetUniformLocation(computeShader.shaderProgramID, "cameraPointingDirectionNormalised");
	glUniform3fv(mainCameraPointingDirectionLoc, 1, glm::value_ptr(mainCamera.cameraPointingDirection));

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBindingPoint, chunksVoxelsDataPoolMetadata.gpu_chunksVoxelsDataPoolMetadatasBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsBufferID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferBindingPoint, chunksPerFaceIndirectDrawCommands.gpu_drawElementsIndirectCommandsDrawCountBufferID);


	glDispatchCompute(16, 1, 1);

	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	// Render World Geometry
	glUseProgram(shaderForRendering.shaderProgramID);

	int mainCameraViewLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "view");
	Mat4x4 viewMatrix = glm::lookAt(cameraTransform.position, glm::normalize(mainCamera.cameraPointingDirection) + cameraTransform.position, mainCamera.cameraUp);
	glUniformMatrix4fv(mainCameraViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	int mainCameraProjectionLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "projection");
	glUniformMatrix4fv(mainCameraProjectionLoc, 1, GL_FALSE, glm::value_ptr(mainCamera.GetProjectionMatrix()));

	int cameraWorldVoxelPositionLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "cameraWorldVoxelPosition");
	glUniform3fv(cameraWorldVoxelPositionLoc, 1, glm::value_ptr(cameraTransform.position));

	int modelLoc = glGetUniformLocation(shaderForRendering.shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(globalTransformMatrix));

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