#version 460 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 worldSizeInChunks;
uniform vec3 cameraWorldVoxelPosition;

layout(binding = 2, std430) readonly buffer VoxelFaceAndPositionData {
    uint voxelFaceAndPositionData[];
};

out vec2 texCoords;
out vec4 chunkDebugColour;


	// Top face (Y+)
vec3 verticesTopFace[3] = vec3[3](
	vec3(-0.5f,  0.5f,  0.5f),	// bottom left
	vec3( 1.5f,  0.5f,  0.5f),	// bottom right
	vec3(-0.5f,  0.5f, -1.5f)	// top left
);

	// Bottom face (Y-)
vec3 verticesBottomFace[3] = vec3[3](
	vec3(-0.5f, -0.5f, -0.5f),	// bottom left
	vec3( 1.5f, -0.5f, -0.5f),	// bottom right
	vec3(-0.5f, -0.5f,  1.5f)	// top left
);

	 // Left face (X-)
vec3 verticesLeftFace[3] = vec3[3](
	vec3(-0.5f, -0.5f, -0.5f),	// bottom left
	vec3(-0.5f, -0.5f,  1.5f),	// bottom right
	vec3(-0.5f,  1.5f, -0.5f)	// top left
);

	// Right face (X+)
vec3 verticesRightFace[3] = vec3[3](
	vec3(0.5f, -0.5f,  0.5f),	// bottom left
	vec3(0.5f, -0.5f, -1.5f),	// bottom right
	vec3(0.5f,  1.5f,  0.5f)	// top left
);

	// Front face (Z+)
vec3 verticesFrontFace[3] = vec3[3](
	vec3(-0.5f, -0.5f,  0.5f),	// bottom left
	vec3( 1.5f, -0.5f,  0.5f),	// bottom right
	vec3(-0.5f,  1.5f,  0.5f)	// top left
);

	// Back face (Z-)
vec3 verticesBackFace[3] = vec3[3](
	vec3( 0.5f, -0.5f, -0.5f),	// bottom left
	vec3(-1.5f, -0.5f, -0.5f),	// bottom right
	vec3( 0.5f,  1.5f, -0.5f)	// top left
);

vec2 arbitaryTexCoords[3] = vec2[3](
	vec2(0.0f, 1.0f),			// bottom left
	vec2(2.0f, 1.0f),			// bottom right
	vec2(0.0f, -1.0f)			// top left
);

const uint topFaceID = 0;
const uint bottomFaceID = 1;
const uint leftFaceID = 2;
const uint rightFaceID = 3;
const uint frontFaceID = 4;
const uint backFaceID = 5;


uint GetCurrentVertexIDWithoutBaseVertex(){
	return gl_VertexID - gl_BaseVertex;
}

uint GetCurrentTriangleVertexID(){
	return GetCurrentVertexIDWithoutBaseVertex() & 3;
}

vec3 GetCurrentVertexBasedOnFaceIndex(uint curFaceIndex) {

	uint triangleVertexID = GetCurrentTriangleVertexID();

	if(curFaceIndex == topFaceID){
		return verticesTopFace[triangleVertexID];
	}
	else if(curFaceIndex == bottomFaceID){
		return verticesBottomFace[triangleVertexID];
	}
	else if(curFaceIndex == leftFaceID){
		return verticesLeftFace[triangleVertexID];
	}
	else if(curFaceIndex == rightFaceID){
		return verticesRightFace[triangleVertexID];
	}
	else if(curFaceIndex == frontFaceID){
		return verticesFrontFace[triangleVertexID];
	}
	else {
		return verticesBackFace[triangleVertexID];
	}
}

// Change to have per face textures and finally per voxel type textures based on LUT.
vec2 GetCurrentTexCoordBasedOnVertexIDAndCurFace(uint curFaceIndex) {

	uint triangleVertexID = GetCurrentTriangleVertexID();
	return arbitaryTexCoords[triangleVertexID];
}

void main()
{
	ivec3 numVoxelsInChunk = ivec3(32, 32, 32);
	uint maxChunkLocalCoord = 63;
	uint chunkPackedCoordShiftBy = 6;

	uint maxVoxelLocalCoord = 31;
	uint voxelCoordBitShiftBy = 5;

	uint curVertexDataID = GetCurrentVertexIDWithoutBaseVertex() >> 2;
	curVertexDataID += gl_BaseVertex;
    uint currentInstancePosition = voxelFaceAndPositionData[curVertexDataID];

    uint xPos = currentInstancePosition & maxVoxelLocalCoord;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;

    uint yPos = currentInstancePosition & maxVoxelLocalCoord;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;

    uint zPos = currentInstancePosition & maxVoxelLocalCoord;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;

    ivec3 voxelLocalPosition = ivec3(xPos, yPos, zPos);

	uint packedChunkCoords = gl_BaseInstance;
	uint chunkXPos = (packedChunkCoords & maxChunkLocalCoord) * numVoxelsInChunk.x;
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	uint chunkYPos = (packedChunkCoords & maxChunkLocalCoord) * numVoxelsInChunk.y;
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	uint chunkZPos = (packedChunkCoords & maxChunkLocalCoord) * numVoxelsInChunk.z;
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	ivec3 chunkPosition = ivec3(chunkXPos, chunkYPos, chunkZPos);



//	vec3 currentCameraChunkPosition = vec3(int(cameraWorldVoxelPosition.x / (numVoxelsInChunk.x)), 0.0, int(cameraWorldVoxelPosition.z / (numVoxelsInChunk.z)));
//	currentCameraChunkPosition *= numVoxelsInChunk;

	vec3 worldCentrePosition = (worldSizeInChunks * numVoxelsInChunk) * 0.5;
	vec3 currentCameraChunkPosition = vec3(worldCentrePosition);

	vec3 curChunkCentrePosition = chunkPosition + (numVoxelsInChunk * 0.5);

	float xDist = (currentCameraChunkPosition.x - curChunkCentrePosition.x);
	float zDist = (currentCameraChunkPosition.z - curChunkCentrePosition.z);

	float xDistSign = sign(xDist);
	float zDistSign = sign(zDist);
	
	xDist = abs(xDist);
	zDist = abs(zDist);

//	float xScale = 1.0 * xDistSign;
//	float zScale = 1.0 * zDistSign;

	float xScale = 1.0;
	float yScale = 1.0;
	float zScale = 1.0;


	if (xDist <= 64.0 && zDist <= 64.0){
		chunkDebugColour = vec4(1.0);
//	} else if ((xDist > 256.0 && xDist <= 512.0 ) || (zDist > 256.0 && zDist <= 512.0)) {
	} else if ((xDist > 256.0) || (zDist > 256.0)) {
		chunkDebugColour = vec4(0.0, 0.0, 1.0, 1.0);

		float scaleMultiplier = 8.0;
		xScale *= scaleMultiplier;
		yScale *= scaleMultiplier;
		zScale *= scaleMultiplier;

	} else if ((xDist > 128.0 && xDist <= 256.0 ) || (zDist > 128.0 && zDist <= 256.0)) {
		chunkDebugColour = vec4(0.0, 1.0, 0.0, 1.0);

		float scaleMultiplier = 4.0;
		xScale *= scaleMultiplier;
		yScale *= scaleMultiplier;
		zScale *= scaleMultiplier;

	} else if ((xDist > 64.0 && xDist <= 128.0) || (zDist > 64.0 && zDist <= 128.0)) {
		chunkDebugColour = vec4(1.0, 0.0, 0.0, 1.0);

		float scaleMultiplier = 2.0;
		xScale *= scaleMultiplier;
		yScale *= scaleMultiplier;
		zScale *= scaleMultiplier;
	}

//	xScale = 1.0;
//	zScale = 1.0;


	uint curFace = currentInstancePosition & 7;
    vec3 vertexPosition = chunkPosition + (voxelLocalPosition) + (GetCurrentVertexBasedOnFaceIndex(curFace) * vec3(xScale, yScale, zScale));
//	vec3 vertexPosition = chunkPosition + (voxelLocalPosition) + (GetCurrentVertexBasedOnFaceIndex(curFace) * vec3(xScale, 1.0, zScale));





    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    texCoords = GetCurrentTexCoordBasedOnVertexIDAndCurFace(curFace);



}
