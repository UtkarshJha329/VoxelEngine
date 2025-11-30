#version 460 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 worldSizeInChunks;
uniform vec3 chunkSizeInVoxels;
uniform vec3 cameraWorldVoxelPosition;

layout(binding = 2, std430) readonly buffer VoxelFaceAndPositionData {
    uint voxelFaceAndPositionData[];
};

out vec2 texCoords;
out vec4 chunkDebugColour;


	// Top face (Y+)
//vec3 verticesTopFace[3] = vec3[3](
//	vec3(-0.5f,  0.5f,  0.5f),	// bottom left
//	vec3( 1.5f,  0.5f,  0.5f),	// bottom right
//	vec3(-0.5f,  0.5f, -1.5f)	// top left
//);
vec3 verticesTopFace[3] = vec3[3](
    vec3(0.0f,   1.0f,  1.0f),   // bottom left
    vec3(2.0f,   1.0f,  1.0f),   // bottom right
    vec3(0.0f,   1.0f, -1.0f)    // top left
);
	// Bottom face (Y-)
//vec3 verticesBottomFace[3] = vec3[3](
//	vec3(-0.5f, -0.5f, -0.5f),	// bottom left
//	vec3( 1.5f, -0.5f, -0.5f),	// bottom right
//	vec3(-0.5f, -0.5f,  1.5f)	// top left
//);
vec3 verticesBottomFace[3] = vec3[3](
    vec3(0.0f,  0.0f,  0.0f),    // bottom left
    vec3(2.0f,  0.0f,  0.0f),    // bottom right
    vec3(0.0f,  0.0f,  2.0f)     // top left
);
	 // Left face (X-)
//vec3 verticesLeftFace[3] = vec3[3](
//	vec3(-0.5f, -0.5f, -0.5f),	// bottom left
//	vec3(-0.5f, -0.5f,  1.5f),	// bottom right
//	vec3(-0.5f,  1.5f, -0.5f)	// top left
//);
vec3 verticesLeftFace[3] = vec3[3](
    vec3(0.0f,  0.0f,  0.0f),    // bottom left
    vec3(0.0f,  0.0f,  2.0f),    // bottom right
    vec3(0.0f,  2.0f,  0.0f)     // top left
);

	// Right face (X+)
//vec3 verticesRightFace[3] = vec3[3](
//	vec3(0.5f, -0.5f,  0.5f),	// bottom left
//	vec3(0.5f, -0.5f, -1.5f),	// bottom right
//	vec3(0.5f,  1.5f,  0.5f)	// top left
//);
vec3 verticesRightFace[3] = vec3[3](
    vec3(1.0f,  0.0f,  1.0f),    // bottom left
    vec3(1.0f,  0.0f, -1.0f),    // bottom right
    vec3(1.0f,  2.0f,  1.0f)     // top left
);

	// Front face (Z+)
//vec3 verticesFrontFace[3] = vec3[3](
//	vec3(-0.5f, -0.5f,  0.5f),	// bottom left
//	vec3( 1.5f, -0.5f,  0.5f),	// bottom right
//	vec3(-0.5f,  1.5f,  0.5f)	// top left
//);
//
vec3 verticesFrontFace[3] = vec3[3](
    vec3(0.0f,  0.0f,  1.0f),    // bottom left
    vec3(2.0f,  0.0f,  1.0f),    // bottom right
    vec3(0.0f,  2.0f,  1.0f)     // top left
);

	// Back face (Z-)
//vec3 verticesBackFace[3] = vec3[3](
//	vec3( 0.5f, -0.5f, -0.5f),	// bottom left
//	vec3(-1.5f, -0.5f, -0.5f),	// bottom right
//	vec3( 0.5f,  1.5f, -0.5f)	// top left
//);

vec3 verticesBackFace[3] = vec3[3](
    vec3(1.0f,  0.0f,  0.0f),    // bottom left
    vec3(-1.0f, 0.0f,  0.0f),    // bottom right
    vec3(1.0f,  2.0f,  0.0f)     // top left
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

// A simple float hash function used internally
float hash1D(float n) {
    // Fract returns the fractional part of a float (e.g., fract(3.14) = 0.14)
    return fract(sin(n) * 43758.5453123);
}

// Function to generate a consistent colour (vec3) based on 3D integer index (i, j, k)
// index_i, index_j, index_k should be the integer coordinates of the chunk/voxel.
vec3 generateConsistentColor(float i, float j, float k) {
    
    // 1. Combine the indices using large prime numbers (vec3 p)
    // The prime numbers (73856.093, 19349.663, 83492.791) are scaled down 
    // to work better with floating-point precision in GLSL.
    vec3 p = vec3(738.56093, 193.49663, 834.92791);
    
    // 2. Generate three distinct seeds by adding an offset and using XOR-like mixing (dot product)
    // The fractional part of the result is used to make it float-friendly.
    // Each channel (R, G, B) gets a different seed offset (0.0, 1.0, 2.0)
    
    // Seed for Red channel
    float seedR = dot(vec3(i, j, k), p);
    
    // Seed for Green channel
    float seedG = dot(vec3(i, j, k) + 1.0, p);
    
    // Seed for Blue channel
    float seedB = dot(vec3(i, j, k) + 2.0, p);
    
    // 3. Hash each seed to get a random float [0.0, 1.0]
    float r = hash1D(seedR);
    float g = hash1D(seedG);
    float b = hash1D(seedB);
    
    // Return the final consistent colour
    return vec3(r, g, b);
}

void main()
{
//	ivec3 numVoxelsInChunk = ivec3(32, 32, 32);
	uint maxChunkLocalCoord = 127;
	uint chunkPackedCoordShiftBy = 7;
	uint maxLODLevel = 3;

	uint maxVoxelLocalCoord = 63;
	uint voxelCoordBitShiftBy = 6;

	uint curVertexDataID = GetCurrentVertexIDWithoutBaseVertex() >> 2;		// Because GenerateCommonChunkMeshOnGPU in VoxelFunctions.h does bit shift to the left by 2 bits for adding triangle vertex ID of [0, 1, 2];
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
	uint chunkXPos = (packedChunkCoords & maxChunkLocalCoord) * int(chunkSizeInVoxels.x);
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	uint chunkYPos = (packedChunkCoords & maxChunkLocalCoord) * int(chunkSizeInVoxels.y);
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	uint chunkZPos = (packedChunkCoords & maxChunkLocalCoord) * int(chunkSizeInVoxels.z);
	packedChunkCoords = packedChunkCoords >> chunkPackedCoordShiftBy;

	ivec3 chunkPosition = ivec3(chunkXPos, chunkYPos, chunkZPos);

	uint chunkLODLevel = packedChunkCoords & 7;

	if (chunkLODLevel == 0){
		chunkDebugColour = vec4(1.0);
	} else if (chunkLODLevel == 4) {
		chunkDebugColour = vec4(1.0, 0.0, 1.0, 1.0);

	} else if (chunkLODLevel == 3) {
		chunkDebugColour = vec4(0.0, 0.0, 1.0, 1.0);

	} else if (chunkLODLevel == 2) {
		chunkDebugColour = vec4(0.0, 1.0, 0.0, 1.0);

	} else if (chunkLODLevel == 1) {
		chunkDebugColour = vec4(1.0, 0.0, 0.0, 1.0);
	}

	float scaleFromLOD = pow(2, chunkLODLevel);
	float xScale = scaleFromLOD;
	float yScale = scaleFromLOD;
	float zScale = scaleFromLOD;


	uint curFace = currentInstancePosition & 7;
    vec3 vertexPosition = chunkPosition + (voxelLocalPosition) + ((GetCurrentVertexBasedOnFaceIndex(curFace)) * vec3(xScale, yScale, zScale));

    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    texCoords = GetCurrentTexCoordBasedOnVertexIDAndCurFace(curFace);

	vec3 maxPosition = worldSizeInChunks * chunkSizeInVoxels;

//	chunkDebugColour = vec4(generateConsistentColor(chunkXPos, chunkYPos, chunkZPos), 1.0);

//	if (chunkXPos > chunkYPos){
//		if (chunkXPos > chunkZPos){
//			chunkDebugColour = vec4(chunkXPos / maxPosition.x, 0.0, 0.0, 1.0);
//		} else if(chunkXPos < chunkZPos){
//			chunkDebugColour = vec4(0.0, 0.0, chunkZPos / maxPosition.z, 1.0);			
//		} else{
//			chunkDebugColour = vec4(0.0, chunkYPos / maxPosition.y, chunkZPos / maxPosition.z, 1.0);			
//		}
//	}else{
//		if (chunkYPos > chunkZPos){
//			chunkDebugColour = vec4(0.0, chunkYPos / maxPosition.y, 0.0, 1.0);			
//		} else if(chunkYPos < chunkZPos){
//			chunkDebugColour = vec4(0.0, 0.0, chunkZPos / maxPosition.z, 1.0);			
//		} else{
//			chunkDebugColour = vec4(chunkXPos / maxPosition.x, chunkYPos / maxPosition.y, 0.0, 1.0);			
//		}
//	}

}