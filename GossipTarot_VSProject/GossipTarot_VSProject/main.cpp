
#include <iostream>

#include <future>

#include <sstream>
#include <string>
#include <iomanip>

#include <cmath>
#include <chrono>

#include <omp.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "GlobalConstants.h"

#include "GLMIncludes.h"

#include "Transform.h"
#include "MeshUtils.h"
#include "Shader.h"
#include "TextureUtils.h"
#include "Camera.h"

#include "RenderUI.h"
#include "UISimulation.h"
#include "UIText.h"

#include "Input.h"

#include "FastNoise/FastNoise.h"
#include "flecs/flecs.h"

#include "BakedMeshData.h"
#include "VoxelFunctions.h"
#include "ChunkGeneration.h"
#include "ChunksVisibilityFromCulling.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void UpdateKeyStates(GLFWwindow* window) {

	//---------------------------------Keyboard Input--------------------------------------

	int wKeyState = glfwGetKey(window, GLFW_KEY_W);
	SetKeyBasedOnState(KeyCode::KEY_W, wKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int sKeyState = glfwGetKey(window, GLFW_KEY_S);
	SetKeyBasedOnState(KeyCode::KEY_S, sKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int dKeyState = glfwGetKey(window, GLFW_KEY_D);
	SetKeyBasedOnState(KeyCode::KEY_D, dKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int aKeyState = glfwGetKey(window, GLFW_KEY_A);
	SetKeyBasedOnState(KeyCode::KEY_A, aKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int pKeyState = glfwGetKey(window, GLFW_KEY_P);
	SetKeyBasedOnState(KeyCode::KEY_P, pKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int qKeyState = glfwGetKey(window, GLFW_KEY_Q);
	SetKeyBasedOnState(KeyCode::KEY_Q, qKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int eKeyState = glfwGetKey(window, GLFW_KEY_E);
	SetKeyBasedOnState(KeyCode::KEY_E, eKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int escKeyState = glfwGetKey(window, GLFW_KEY_ESCAPE);
	SetKeyBasedOnState(KeyCode::KEY_ESCAPE, escKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	int spaceKeyState = glfwGetKey(window, GLFW_KEY_SPACE);
	SetKeyBasedOnState(KeyCode::KEY_SPACE, spaceKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

	//---------------------------------------Mouse Input------------------------------------------

	int mouseButtonLeftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	SetKeyBasedOnState(KeyCode::MOUSE_BUTTON_LEFT, mouseButtonLeftState > 0 ? PRESSED_OR_HELD : RELEASED);

	int mouseButtonRightState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	SetKeyBasedOnState(KeyCode::MOUSE_BUTTON_RIGHT, mouseButtonRightState > 0 ? PRESSED_OR_HELD : RELEASED);

	mouseXFromPreviousFrame = mouseX;
	mouseYFromPreviousFrame = mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	mouseXDelta = mouseX - mouseXFromPreviousFrame;
	mouseYDelta = mouseY - mouseYFromPreviousFrame;
}

void RenderDeltaTimeInMS(const ShaderProgram& shaderForRendering, const int& rootUIRectIndex, const MeshOnGPU& meshOnGPU, const float& deltaTime) {

	Vector3 deltaTimeTextPosition = { -200.0f, -60.0f, 0.0f };
	float deltaTimeInMs = deltaTime * 1000.0f;
	float roundedDeltaTimeInMs = std::round(deltaTimeInMs * std::pow(10, 2)) / std::pow(10, 2);
	std::stringstream deltaTimeStringStream;
	deltaTimeStringStream << std::fixed << std::setprecision(2) << roundedDeltaTimeInMs;
	std::string deltaTimeString = deltaTimeStringStream.str() + " ms.";

	RenderText(shaderForRendering, AnchorPosition::TopRight, meshOnGPU, deltaTimeString, deltaTimeTextPosition, rootUIRectIndex);
}

bool CurrentlyGeneratingAChunk(std::vector<std::future<void>>& chunkGenerationFutures) {

	for (auto& curChunkGenerationFuture : chunkGenerationFutures)
	{
		if (curChunkGenerationFuture.valid()) {
			if (curChunkGenerationFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
				return true;
			}
		}
	}

	return false;
}

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Enable face culling
	glEnable(GL_CULL_FACE);

	// Cull only back faces
	glCullFace(GL_BACK);

	// Define front face as counter-clockwise
	glFrontFace(GL_CCW);





	std::string woodContainerTexturePath = "Assets/Textures/WoodContainer.jpg";
	int woodContainerTextureIndex = TextureFromFile(woodContainerTexturePath);

	std::string stickmanTexturePath = "Assets/Textures/Stickman.jpg";
	int stickmanTextureIndex = TextureFromFile(stickmanTexturePath);

	std::string courierFontPath = "Assets/Fonts/cour.ttf";



	Shader simpleVertexShaderWithCameraWithTextures;
	simpleVertexShaderWithCameraWithTextures.shaderFilePath = "Assets/Shaders/SimpleShaderWithCameraWithTextures/SimpleShaderWithCameraWithTextures.vert";
	simpleVertexShaderWithCameraWithTextures.shaderType = SHADER_TYPE::VERTEX_SHADER;
	simpleVertexShaderWithCameraWithTextures.CreateShader();

	Shader simpleFragmentShaderWithCameraWithTextures;
	simpleFragmentShaderWithCameraWithTextures.shaderFilePath = "Assets/Shaders/SimpleShaderWithCameraWithTextures/SimpleShaderWithCameraWithTextures.frag";
	simpleFragmentShaderWithCameraWithTextures.shaderType = SHADER_TYPE::FRAGMENT_SHADER;
	simpleFragmentShaderWithCameraWithTextures.CreateShader();

	std::vector<Shader> shadersForThisShaderProgram = { simpleVertexShaderWithCameraWithTextures, simpleFragmentShaderWithCameraWithTextures };
	ShaderProgram simpleVertexAndFragmentWithCameraWithTexturesShaderProgram;

	simpleVertexAndFragmentWithCameraWithTexturesShaderProgram.CreateShaderProgram(shadersForThisShaderProgram);

	Shader textRenderingVertexShaderWithCameraWithTextures;
	textRenderingVertexShaderWithCameraWithTextures.shaderFilePath = "Assets/Shaders/TextRenderingShaderWithCameraWithTextures/TextRenderingShaderWithCameraWithTextures.vert";
	textRenderingVertexShaderWithCameraWithTextures.shaderType = SHADER_TYPE::VERTEX_SHADER;
	textRenderingVertexShaderWithCameraWithTextures.CreateShader();

	Shader textRenderingFragmentShaderWithCameraWithTextures;
	textRenderingFragmentShaderWithCameraWithTextures.shaderFilePath = "Assets/Shaders/TextRenderingShaderWithCameraWithTextures/TextRenderingShaderWithCameraWithTextures.frag";
	textRenderingFragmentShaderWithCameraWithTextures.shaderType = SHADER_TYPE::FRAGMENT_SHADER;
	textRenderingFragmentShaderWithCameraWithTextures.CreateShader();

	std::vector<Shader> shadersForTextRenderingShaderProgram = { textRenderingVertexShaderWithCameraWithTextures, textRenderingFragmentShaderWithCameraWithTextures };
	ShaderProgram textRenderingVertexAndFragmentWithCameraWithTexturesShaderProgram;

	textRenderingVertexAndFragmentWithCameraWithTexturesShaderProgram.CreateShaderProgram(shadersForTextRenderingShaderProgram);



	Shader voxelVertexShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw;
	voxelVertexShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw.shaderFilePath = "Assets/Shaders/CompressedVoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw/VoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw.vert";
	voxelVertexShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw.shaderType = SHADER_TYPE::VERTEX_SHADER;
	voxelVertexShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw.CreateShader();

	Shader voxelFragmentShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw;
	voxelFragmentShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw.shaderFilePath = "Assets/Shaders/CompressedVoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw/VoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw.frag";
	voxelFragmentShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw.shaderType = SHADER_TYPE::FRAGMENT_SHADER;
	voxelFragmentShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw.CreateShader();

	std::vector<Shader> shadersForVoxelPerFaceSSBODataAsTriangleIndirectDrawShaderProgram = { voxelVertexShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw, voxelFragmentShaderWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDraw };
	ShaderProgram voxelVertexAndFragmentWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDrawShaderProgram;

	voxelVertexAndFragmentWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDrawShaderProgram.CreateShaderProgram(shadersForVoxelPerFaceSSBODataAsTriangleIndirectDrawShaderProgram);

	Shader voxelBackfaceCullingComputeShader;
	voxelBackfaceCullingComputeShader.shaderFilePath = "Assets/Shaders/CompressedVoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw/VoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw.comp";
	voxelBackfaceCullingComputeShader.shaderType = SHADER_TYPE::COMPUTE_SHADER;
	voxelBackfaceCullingComputeShader.CreateShader();

	std::vector<Shader> shaderForVoxelBackfaceCullingComputeShaderProgram = { voxelBackfaceCullingComputeShader };
	ShaderProgram voxelBackfaceCullingComputeShaderProgram;

	voxelBackfaceCullingComputeShaderProgram.CreateShaderProgram(shaderForVoxelBackfaceCullingComputeShaderProgram);


	Shader cullingComputeShader;
	cullingComputeShader.shaderFilePath = "Assets/Shaders/CompressedVoxelWithTexturesShaderPerFaceSSBODataAsTriangleIndirectDraw/FrustumCulling.comp";
	cullingComputeShader.shaderType = SHADER_TYPE::COMPUTE_SHADER;
	cullingComputeShader.CreateShader();

	std::vector<Shader> shaderForCullingComputeShaderProgram = { cullingComputeShader };
	ShaderProgram cullingComputeShaderProgram;

	cullingComputeShaderProgram.CreateShaderProgram(shaderForCullingComputeShaderProgram);


	MeshOnCPU simpleQuadMeshCPU;
	for (int i = 0; i < 20; i++)
	{
		//simpleQuadMeshCPU.vertices.push_back(vertices[i]);
		simpleQuadMeshCPU.vertices.push_back(QuadVerticesWithTexCoords[i]);
	};

	for (int i = 0; i < 6; i++)
	{
		simpleQuadMeshCPU.indices.push_back(QuadIndices[i]);
	}

	MeshOnGPU simpleQuadMeshGPU;

	CreateMeshOnGPU(simpleQuadMeshCPU, simpleQuadMeshGPU);




	Transform modelTransform;
	modelTransform.position = { 0.0f, 0.0f, 3.0f };
	modelTransform.rotation = { 0.0f, 0.0f, 0.0f };
	modelTransform.scale = { 1.0f, 1.0f, 1.0f };



	Transform cameraTransform;
	cameraTransform.position = { 0.0f, 32.0f, -3.0f };
	cameraTransform.rotation = { 0.0f, 45.0f, 0.0f };
	cameraTransform.scale = { 1.0f, 1.0f, 1.0f };

	Camera mainCamera;
	mainCamera.viewport.dimensions = { WINDOW_WIDTH, WINDOW_HEIGHT };
	mainCamera.SetProjectionMatrixToPerspectiveProjection(mainCamera.fovInDegrees, mainCamera.nearPlaneDistance, mainCamera.farPlaneDistance);

	Vector3 cameraFront = Vector3{ 0.0f, 0.0f, 1.0f };

	glm::quat rotQuat = glm::quat(glm::radians(cameraTransform.rotation));
	Mat4x4 rotationMatrix = glm::mat4_cast(rotQuat);
	cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));

	mainCamera.SetCameraDirectionVectors(Transform::worldUp, cameraFront);


	Transform uiModelTransform;
	uiModelTransform.position = { 0.0f, 0.0f, 0.0f };
	uiModelTransform.rotation = { 0.0f, 0.0f, 0.0f };
	uiModelTransform.scale = { 100.0f, 100.0f, 100.0f };

	Camera uiCamera;
	uiCamera.viewport.dimensions = { WINDOW_WIDTH, WINDOW_HEIGHT };
	uiCamera.SetProjectionMatrixToOrthographicProjection(0.1f, 1000.0f);

	Transform uiCameraTransform;
	uiCameraTransform.position = { 0.0f, 0.0f, -3.0f };
	uiCameraTransform.rotation = { 0.0f, 0.0f, 0.0f };
	uiCameraTransform.scale = { 1.0f, 1.0f, 1.0f };

	const int rootUIRectIndex = UI_Rect::uiRects.size();
	Vector3 rootStart = { -WINDOW_WIDTH / 2.0f, -WINDOW_HEIGHT / 2.0f, 0.0f };
	Vector3 rootEnd = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.0f };
	UI_Rect::uiRects.push_back({ rootUIRectIndex, rootStart, rootEnd, MiddleMiddle });
	UI_Rect::uiRects[0].worldStartPos = rootStart;
	UI_Rect::uiRects[0].worldEndPos = rootEnd;

	InitializeCharacterRects(courierFontPath, rootUIRectIndex);

	int numChildrenUIRects = 4;
	for (int i = 0; i < numChildrenUIRects; i++)
	{
		if (i == 2) {
			continue;
		}
		UI_Rect someChildUIRect;
		someChildUIRect.index = UI_Rect::uiRects.size();

		if (i == 0) {
			//someChildUIRect.anchorPosition = TopMiddle;
			someChildUIRect.anchorPosition = TopLeft;

			someChildUIRect.start = { 0.0f, 0.0f, 0.0f };
			someChildUIRect.end = { 100.0f, -100.0f, 0.0f };
		}
		else if (i == 1) {
			//someChildUIRect.anchorPosition = MiddleLeft;
			someChildUIRect.anchorPosition = BottomLeft;

			someChildUIRect.start = { 0.0f, 0.0f, 0.0f };
			someChildUIRect.end = { 100.0f, 100.0f, 0.0f };
		}
		else if (i == 2) {
			//someChildUIRect.anchorPosition = MiddleRight;
			someChildUIRect.anchorPosition = TopRight;

			someChildUIRect.start = { 0.0f, 0.0f, 0.0f };
			someChildUIRect.end = { -100.0f, -100.0f, 0.0f };
		}
		else if (i == 3) {
			//someChildUIRect.anchorPosition = BottomMiddle;
			someChildUIRect.anchorPosition = BottomRight;

			someChildUIRect.start = { 0.0f, 0.0f, 0.0f };
			someChildUIRect.end = { -100.0f, 100.0f, 0.0f };
		}

		AddUIRectAsChildToUIRect(someChildUIRect, rootUIRectIndex);
	}



	// VVV LIMITED BY COMPRESSION TO INT IN PACKED CHUNK INDEX AND NUMBER OF COMPUTE THREADS DISPATCHED! Currently limited by each coordinate having 7 bits, i.e max 127 for each coord but with compute threads dispatched it is limited to __.
	Vector3Int worldSizeInChunks = { 64, 16, 64 };
	Vector3Int chunkSizeInVoxels = { 32, 32, 32 };

	Vector3Int centreVoxelPositionInWorld = { (worldSizeInChunks.x * chunkSizeInVoxels.x) / 2, (worldSizeInChunks.y * chunkSizeInVoxels.y) / 2, (worldSizeInChunks.z * chunkSizeInVoxels.z) / 2 };
	cameraTransform.position = { centreVoxelPositionInWorld.x, centreVoxelPositionInWorld.y, centreVoxelPositionInWorld.z };

	const unsigned int numLODLevels = 4;

	unsigned int numVoxelDatasOnLOD_Zero_Bucket = (chunkSizeInVoxels.x / 2) * (chunkSizeInVoxels.y / 2) * (chunkSizeInVoxels.z / 2);
	std::vector<unsigned int> LODLevelAndChunksMaxSizeInVoxels = {
		numVoxelDatasOnLOD_Zero_Bucket,
		numVoxelDatasOnLOD_Zero_Bucket,
		numVoxelDatasOnLOD_Zero_Bucket,
		numVoxelDatasOnLOD_Zero_Bucket
	};

	unsigned int totalNumChunks = worldSizeInChunks.x * worldSizeInChunks.y * worldSizeInChunks.z;
	std::vector<unsigned int> numBucketsPerLOD = {
		0,
		0,
		0,
		0
	};

	Vector3 currentCameraChunkPosition = Vector3(int(cameraTransform.position.x / (chunkSizeInVoxels.x)), 0.0, int(cameraTransform.position.z / (chunkSizeInVoxels.z)));
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


				if (xDist <= 64.0 && zDist <= 64.0) {

					numBucketsPerLOD[0]++;
				}
				//else if ((xDist > 256.0 && xDist <= 512.0) || (zDist > 256.0 && zDist <= 512.0)) {
				else if ((xDist > 256.0) || (zDist > 256.0)) {

					numBucketsPerLOD[3]++;
				}
				else if ((xDist > 128.0 && xDist <= 256.0) || (zDist > 128.0 && zDist <= 256.0)) {

					numBucketsPerLOD[2]++;
				}
				else if ((xDist > 64.0 && xDist <= 128.0) || (zDist > 64.0 && zDist <= 128.0)) {

					numBucketsPerLOD[1]++;
				}
			}
		}
	}

	for (int i = 0; i < numLODLevels; i++)
	{
		LODLevelAndChunksMaxSizeInVoxels[i] /= unsigned int(pow(2, i));
		numBucketsPerLOD[i] *= 6;
		numBucketsPerLOD[i] += 100;
	}	

	numBucketsPerLOD[3] = (totalNumChunks * 6);

	//unsigned int numVoxelDatasPerBucket = (chunkSizeInVoxels.x / 2) * (chunkSizeInVoxels.y / 2) * (chunkSizeInVoxels.z / 2);
	unsigned int numVoxelDatasPerBucket = (chunkSizeInVoxels.x / 2) * (chunkSizeInVoxels.y / 2) * (chunkSizeInVoxels.z / 2);
	unsigned int numBuckets = worldSizeInChunks.x * worldSizeInChunks.y * worldSizeInChunks.z * 6;
	unsigned int megaVoxelsPerFaceDataBufferObjectBindingLocation = 2;

	size_t sizeOfPoolInBytes = numBuckets * numVoxelDatasPerBucket * sizeof(unsigned int);

	//VoxelsDataPool voxelsDataPool(numVoxelDatasPerBucket, numBuckets, megaVoxelsPerFaceDataBufferObjectBindingLocation);
	VoxelsDataPool voxelsDataPool(numLODLevels, LODLevelAndChunksMaxSizeInVoxels, numBucketsPerLOD, megaVoxelsPerFaceDataBufferObjectBindingLocation);


	unsigned int numFaces = worldSizeInChunks.x * worldSizeInChunks.y * worldSizeInChunks.z * 6;
	unsigned int indirectDrawCommandsBufferBindingPoint = 4;
	unsigned int indirectDrawCommandsDrawCountBufferBindingPoint = 5;
	ChunksPerFaceIndirectDrawCommands  chunksPerFaceIndirectDrawCommands(numFaces, indirectDrawCommandsBufferBindingPoint, indirectDrawCommandsDrawCountBufferBindingPoint);

	MeshOnGPU commonChunkMeshOnGPU;
	GenerateCommonChunkMeshOnGPU(chunkSizeInVoxels, commonChunkMeshOnGPU);
	
	std::vector<int> chunksLODLevel;
	chunksLODLevel.reserve(totalNumChunks);
	chunksLODLevel.resize(totalNumChunks);

	std::vector<std::future<void>> chunkGenerationFutures;

	unsigned int chunksVoxelsDataPoolMetadatasBindingPoint = 3;
	ChunksVoxelsDataPoolMetadata chunksVoxelsDataPoolMetadatas(worldSizeInChunks, chunksVoxelsDataPoolMetadatasBindingPoint);
	GenerateChunksAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU(worldSizeInChunks, chunkSizeInVoxels, chunksLODLevel, voxelsDataPool, chunksPerFaceIndirectDrawCommands, chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas, chunksVoxelsDataPoolMetadatas, cameraTransform.position, chunkGenerationFutures);

	chunksPerFaceIndirectDrawCommands.GPU_InitCommandBuffer();

	for (auto& future : chunkGenerationFutures)
	{
		try {
			if (future.valid())
				future.get();
		}
		catch (std::future_error& e) {
			std::cout << "future_error: " << e.code() << " - " << e.what() << "\n";
		}

	}
	chunkGenerationFutures.clear();

	unsigned int gpu_cameraFrustumDataBindingPoint = 6;
	unsigned int gpu_chunksVisibilityFromCullingDataBindingPoint = 7;
	ChunksVisiblityFromCulling chunksVisibilityFromCulling(totalNumChunks, gpu_cameraFrustumDataBindingPoint, gpu_chunksVisibilityFromCullingDataBindingPoint);

	// Todo 2 : Frustum Culling.
	// Todo 3 : Block types/ block palette.
	// Todo 4 : Binary Meshing.

	std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameEndTime = std::chrono::high_resolution_clock::now();
	float deltaTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		// Find best placement for this piece of code below VVV. Should it be at the end or at the begining or even somewhere in between?
		if (GetKeyPressedInThisFrame(KeyCode::KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, true);
		}

		UpdateKeyStates(window);

		float mouseSensitivity = 75.0f;
		float moveSpeed = 3.0f;

		if (GetKeyHeld(KeyCode::KEY_SPACE)) {
			moveSpeed = 30.0f;
		}
		else if (GetKeyReleasedInThisFrame(KeyCode::KEY_SPACE)) {
			moveSpeed = 3.0f;
		}


		if (GetKeyHeld(KeyCode::MOUSE_BUTTON_RIGHT)) {

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			cameraTransform.rotation.y -= mouseXDelta * mouseSensitivity * deltaTime;
			cameraTransform.rotation.x += mouseYDelta * mouseSensitivity * deltaTime;

			if (cameraTransform.rotation.x > 90.0f)
				cameraTransform.rotation.x = 90.0f;
			if (cameraTransform.rotation.x < -90.0f)
				cameraTransform.rotation.x = -90.0f;

			Vector3 cameraFront = Vector3{ 0.0f, 0.0f, 1.0f };

			//glm::quat rotQuat = glm::quat(glm::radians(cameraTransform.rotation));
			//Mat4x4 rotationMatrix = glm::mat4_cast(rotQuat);
			//cameraFront = glm::vec3(rotationMatrix * glm::vec4(cameraFront, 1.0f));

			glm::quat rotQuat = glm::quat(glm::radians(cameraTransform.rotation));
			cameraFront = normalize(rotQuat * glm::vec3(0, 0, 1));

			mainCamera.SetCameraDirectionVectors(Transform::worldUp, cameraFront);

			if (GetKeyHeld(KeyCode::KEY_W)) {
				cameraTransform.position += mainCamera.cameraPointingDirection * moveSpeed * deltaTime;
			}
			if (GetKeyHeld(KeyCode::KEY_A)) {
				cameraTransform.position -= mainCamera.cameraRight * moveSpeed * deltaTime;
			}
			if (GetKeyHeld(KeyCode::KEY_S)) {
				cameraTransform.position -= mainCamera.cameraPointingDirection * moveSpeed * deltaTime;
			}
			if (GetKeyHeld(KeyCode::KEY_D)) {
				cameraTransform.position += mainCamera.cameraRight * moveSpeed * deltaTime;
			}

			if (GetKeyHeld(KeyCode::KEY_Q)) {
				cameraTransform.position += cameraTransform.worldUp * moveSpeed * deltaTime;
			}
			if (GetKeyHeld(KeyCode::KEY_E)) {
				cameraTransform.position -= cameraTransform.worldUp * moveSpeed * deltaTime;
			}

			if (!CurrentlyGeneratingAChunk(chunkGenerationFutures)) {

				for (auto& future : chunkGenerationFutures)
				{
					try {
						if (future.valid())
							future.get();
					}
					catch (std::future_error& e) {
						std::cout << "future_error: " << e.code() << " - " << e.what() << "\n";
					}

				}
				chunkGenerationFutures.clear();

				std::vector<std::pair<Vector3Int, int>> newChunkLODLevels;
				CheckChunksForLODChanges(chunksLODLevel, cameraTransform.position, chunkSizeInVoxels, worldSizeInChunks, newChunkLODLevels);


				unsigned int numLOD0 = 0;
				unsigned int numLOD1 = 0;
				unsigned int numLOD2 = 0;
				unsigned int numLOD3 = 0;

				if (!newChunkLODLevels.empty()) {

					Vector3 currentCameraChunkPosition = Vector3(int(cameraTransform.position.x / (chunkSizeInVoxels.x)), 0.0, int(cameraTransform.position.z / (chunkSizeInVoxels.z)));
					currentCameraChunkPosition *= chunkSizeInVoxels;

					Vector3 halfChunkSize = Vector3(chunkSizeInVoxels / 2);

					auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
					auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();

					fnFractal->SetSource(fnSimplex);
					fnFractal->SetOctaveCount(5);

					for (unsigned int i = 0; i < newChunkLODLevels.size(); i++)
					{
						Vector3Int curChunkIndex = newChunkLODLevels[i].first;
						unsigned int flattenedChunkIndex = GetFlattenedChunkIndexForChunksVoxelsDataPoolMetadatas(worldSizeInChunks, curChunkIndex);

						int previousLODLevelOfCurChunk = chunksLODLevel[flattenedChunkIndex];


						unsigned int chunksCurrentTopFaceBucketIndex = chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas[flattenedChunkIndex].topFaceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex;
						voxelsDataPool.MakeBucketAFreeBucket(chunksCurrentTopFaceBucketIndex, previousLODLevelOfCurChunk);

						unsigned int chunksCurrentBottomFaceBucketIndex = chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas[flattenedChunkIndex].bottomFaceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex;
						voxelsDataPool.MakeBucketAFreeBucket(chunksCurrentBottomFaceBucketIndex, previousLODLevelOfCurChunk);

						unsigned int chunksCurrentFrontFaceBucketIndex = chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas[flattenedChunkIndex].frontFaceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex;
						voxelsDataPool.MakeBucketAFreeBucket(chunksCurrentFrontFaceBucketIndex, previousLODLevelOfCurChunk);

						unsigned int chunksCurrentBackFaceBucketIndex = chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas[flattenedChunkIndex].backFaceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex;
						voxelsDataPool.MakeBucketAFreeBucket(chunksCurrentBackFaceBucketIndex, previousLODLevelOfCurChunk);

						unsigned int chunksCurrentLeftFaceBucketIndex = chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas[flattenedChunkIndex].leftFaceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex;
						voxelsDataPool.MakeBucketAFreeBucket(chunksCurrentLeftFaceBucketIndex, previousLODLevelOfCurChunk);

						unsigned int chunksCurrentRightFaceBucketIndex = chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas[flattenedChunkIndex].rightFaceVoxelsDataPoolMetadata.voxelDataBucketOffsetIntoMegaArrayIndex;
						voxelsDataPool.MakeBucketAFreeBucket(chunksCurrentRightFaceBucketIndex, previousLODLevelOfCurChunk);
					}

					for (int i = 0; i < newChunkLODLevels.size(); i++)
					{
						chunkGenerationFutures.push_back(std::async(
							std::launch::async,
							DEBUG_GenerateChunkAndUploadTo_GPUAndAddToIndirectRenderCommandVectorOn_CPU,
							newChunkLODLevels[i].first,
							std::ref(chunksLODLevel),
							newChunkLODLevels[i].second,
							fnFractal,
							currentCameraChunkPosition,
							halfChunkSize,
							worldSizeInChunks,
							chunkSizeInVoxels,
							std::ref(voxelsDataPool),
							std::ref(chunksPerFaceIndirectDrawCommands),
							std::ref(chunksVoxelsDataPoolMetadatas.chunksVoxelsDataPoolMetadatas),
							std::ref(chunksVoxelsDataPoolMetadatas),
							cameraTransform.position));
					}
				}
			}


		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		// Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Render World Geometry
		//RenderMeshOnGPUWithDrawElementsIndirectCommandsWithComputeShader(
		//	voxelVertexAndFragmentWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDrawShaderProgram,
		//	voxelBackfaceCullingComputeShaderProgram,
		//	cameraTransform, mainCamera,
		//	modelTransform.GetTransformMatrix(),
		//	stickmanTextureIndex,
		//	worldSizeInChunks,
		//	commonChunkMeshOnGPU,
		//	chunksPerFaceIndirectDrawCommands,
		//	voxelsDataPool,
		//	chunksVoxelsDataPoolMetadatas
		//);

		RenderMeshOnGPUWithDrawElementsIndirectCommandsWithComputeShaderAndCullingComputeShader(
			voxelVertexAndFragmentWithCameraWithTexturesPerFaceSSBODataAsTriangleIndirectDrawShaderProgram,
			voxelBackfaceCullingComputeShaderProgram,
			cullingComputeShaderProgram, chunksVisibilityFromCulling,
			cameraTransform, mainCamera,
			modelTransform.GetTransformMatrix(),
			stickmanTextureIndex,
			worldSizeInChunks,
			commonChunkMeshOnGPU,
			chunksPerFaceIndirectDrawCommands,
			voxelsDataPool,
			chunksVoxelsDataPoolMetadatas
		);



		// Render UI
		glUseProgram(simpleVertexAndFragmentWithCameraWithTexturesShaderProgram.shaderProgramID);

		int viewLoc = glGetUniformLocation(simpleVertexAndFragmentWithCameraWithTexturesShaderProgram.shaderProgramID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(uiCameraTransform.GetTransformMatrix()));

		int projectionLoc = glGetUniformLocation(simpleVertexAndFragmentWithCameraWithTexturesShaderProgram.shaderProgramID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(uiCamera.GetProjectionMatrix()));

		Vector2 mousePosInUISpace = ConvertMousePosToScreenUIPos(mouseX, mouseY, WINDOW_WIDTH, WINDOW_HEIGHT);
		UpdateUITreeStates(UI_Rect::uiRects[rootUIRectIndex], mousePosInUISpace.x, mousePosInUISpace.y);
		HandleUIEvents(mouseXDelta, mouseYDelta);
		RenderUI(UI_Rect::uiRects[rootUIRectIndex], simpleVertexAndFragmentWithCameraWithTexturesShaderProgram, stickmanTextureIndex, simpleQuadMeshGPU);

		// Render Text
		glUseProgram(textRenderingVertexAndFragmentWithCameraWithTexturesShaderProgram.shaderProgramID);
		int viewLocInTextShader = glGetUniformLocation(textRenderingVertexAndFragmentWithCameraWithTexturesShaderProgram.shaderProgramID, "view");
		glUniformMatrix4fv(viewLocInTextShader, 1, GL_FALSE, glm::value_ptr(uiCameraTransform.GetTransformMatrix()));

		int projectionLocInTextShader = glGetUniformLocation(textRenderingVertexAndFragmentWithCameraWithTexturesShaderProgram.shaderProgramID, "projection");
		glUniformMatrix4fv(projectionLocInTextShader, 1, GL_FALSE, glm::value_ptr(uiCamera.GetProjectionMatrix()));

		RenderDeltaTimeInMS(textRenderingVertexAndFragmentWithCameraWithTexturesShaderProgram, rootUIRectIndex, simpleQuadMeshGPU, deltaTime);

		// Swap Buffers for display
		glfwSwapBuffers(window);
		glfwPollEvents();

		// Calculate frame delta time
		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsedTime = currentTime - lastFrameEndTime;
		deltaTime = elapsedTime.count();
		lastFrameEndTime = currentTime;

		// Reset Keyboard State for next frame
		ResetKeysReleased();
	}

	glfwTerminate();
	return 0;
}