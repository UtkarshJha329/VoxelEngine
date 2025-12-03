#version 460 core
layout (location = 0) in uint voxelVertexData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;


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

vec3 GetCurrentVertexBasedOnFaceIndex(uint curFaceIndex) {

	int triangleVertexID = gl_VertexID % 3;

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

	int triangleVertexID = gl_VertexID % 3;
	return arbitaryTexCoords[triangleVertexID];
}

void main()
{
	uint maxVoxelLocalCoord = 31;
	uint voxelCoordBitShiftBy = 5;

    uint currentInstancePosition = voxelVertexData;

    uint xPos = currentInstancePosition & maxVoxelLocalCoord;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;

    uint yPos = currentInstancePosition & maxVoxelLocalCoord;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;

    uint zPos = currentInstancePosition & maxVoxelLocalCoord;
    currentInstancePosition = currentInstancePosition >> voxelCoordBitShiftBy;

    vec3 voxelLocalPosition = vec3(float(xPos), float(yPos), float(zPos));

	uint curFace = currentInstancePosition & 7;
    vec3 vertexPosition = GetCurrentVertexBasedOnFaceIndex(curFace) + voxelLocalPosition;



    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    texCoords = GetCurrentTexCoordBasedOnVertexIDAndCurFace(curFace);
}