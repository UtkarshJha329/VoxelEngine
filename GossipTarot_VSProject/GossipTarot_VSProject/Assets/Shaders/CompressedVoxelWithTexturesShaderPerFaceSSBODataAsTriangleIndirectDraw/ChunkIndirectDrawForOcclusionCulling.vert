#version 460 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 worldSizeInChunks;
uniform vec3 cameraWorldVoxelPosition;

layout (binding = 2, std430) readonly buffer VoxelFaceAndPositionData {
    uint voxelFaceAndPositionData[];
};


layout (binding = 7, std430) buffer ChunksVisibilityData {
	int chunksVisibilityData[];
};


out float distFromCamera;
//out vec2 texCoords;
out flat uint chunkFlattenedIndex;


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
	uint maxChunkLocalCoord = 127;
	uint chunkPackedCoordShiftBy = 7;

	uint voxelCoordBitShiftBy = 5;

	uint curVertexDataID = GetCurrentVertexIDWithoutBaseVertex() >> 2;		// Because GenerateCommonChunkMeshOnGPU in VoxelFunctions.h does bit shift to the left by 2 bits for adding triangle vertex ID of [0, 1, 2];
	curVertexDataID += gl_BaseVertex;
    uint currentInstancePosition = voxelFaceAndPositionData[curVertexDataID];


    ivec3 voxelLocalPosition = ivec3(16, 16, 16);

	uint packedChunkCoords = gl_BaseInstance;
	uint chunkXIndex = (packedChunkCoords & maxChunkLocalCoord);
	uint chunkXPos =  chunkXIndex * numVoxelsInChunk.x;
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	uint chunkYIndex = (packedChunkCoords & maxChunkLocalCoord);
	uint chunkYPos = chunkYIndex * numVoxelsInChunk.y;
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	uint chunkZIndex = (packedChunkCoords & maxChunkLocalCoord);
	uint chunkZPos = chunkZIndex * numVoxelsInChunk.z;
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	ivec3 chunkPosition = ivec3(chunkXPos, chunkYPos, chunkZPos);


	float xScale = numVoxelsInChunk.x;
	float yScale = numVoxelsInChunk.y;
	float zScale = numVoxelsInChunk.z;

	currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;
	currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;
	uint curFace = currentInstancePosition & 7;
    vec3 vertexPosition = chunkPosition + (voxelLocalPosition) + (GetCurrentVertexBasedOnFaceIndex(curFace) * vec3(xScale, yScale, zScale));

    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
//    texCoords = GetCurrentTexCoordBasedOnVertexIDAndCurFace(curFace);

	chunkFlattenedIndex = uint(chunkYIndex * worldSizeInChunks.x * worldSizeInChunks.z + chunkZIndex * worldSizeInChunks.x + chunkXIndex);

	distFromCamera = length(vertexPosition - cameraWorldVoxelPosition);

	if (distFromCamera <= 32.0){
		chunksVisibilityData[chunkFlattenedIndex] = 1;
//        atomicAdd(chunksVisibilityData[chunkFlattenedIndex], 1);

	}
}
