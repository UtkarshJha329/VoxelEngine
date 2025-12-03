#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in uint instancePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;


	// Top face (Y+)
vec3 verticesTopFace[4] = vec3[4](
	vec3(-0.5f,  0.5f,  0.5f),	// bottom left
	vec3( 0.5f,  0.5f,  0.5f),	// bottom right
	vec3( 0.5f,  0.5f, -0.5f),	// top right
	vec3(-0.5f,  0.5f, -0.5f)	// top left
);

	// Bottom face (Y-)
vec3 verticesBottomFace[4] = vec3[4](
	vec3(-0.5f, -0.5f, -0.5f),	// bottom left
	vec3( 0.5f, -0.5f, -0.5f),	// bottom right
	vec3( 0.5f, -0.5f,  0.5f),	// top right
	vec3(-0.5f, -0.5f,  0.5f)	// top left
);

	 // Left face (X-)
vec3 verticesLeftFace[4] = vec3[4](
	vec3(-0.5f, -0.5f, -0.5f),	// bottom left
	vec3(-0.5f, -0.5f,  0.5f),	// bottom right
	vec3(-0.5f,  0.5f,  0.5f),	// top right
	vec3(-0.5f,  0.5f, -0.5f)	// top left
);

	// Right face (X+)
vec3 verticesRightFace[4] = vec3[4](
	vec3(0.5f, -0.5f,  0.5f),	// bottom left
	vec3(0.5f, -0.5f, -0.5f),	// bottom right
	vec3(0.5f,  0.5f, -0.5f),	// top right
	vec3(0.5f,  0.5f,  0.5f)	// top left
);

	// Front face (Z+)
vec3 verticesFrontFace[4] = vec3[4](
	vec3(-0.5f, -0.5f,  0.5f),	// bottom left
	vec3( 0.5f, -0.5f,  0.5f),	// bottom right
	vec3( 0.5f,  0.5f,  0.5f),	// top right
	vec3(-0.5f,  0.5f,  0.5f)	// top left
);

	// Back face (Z-)
vec3 verticesBackFace[4] = vec3[4](
	vec3( 0.5f, -0.5f, -0.5f),	// bottom left
	vec3(-0.5f, -0.5f, -0.5f),	// bottom right
	vec3(-0.5f,  0.5f, -0.5f),	// top right
	vec3( 0.5f,  0.5f, -0.5f)	// top left
);

const uint topFaceID = 0;
const uint bottomFaceID = 1;
const uint leftFaceID = 2;
const uint rightFaceID = 3;
const uint frontFaceID = 4;
const uint backFaceID = 5;

vec3 GetCurrentVertexBasedOnFaceIndex(uint curFaceIndex) {

	if(curFaceIndex == topFaceID){
		return verticesTopFace[gl_VertexID];
	}
	else if(curFaceIndex == bottomFaceID){
		return verticesBottomFace[gl_VertexID];
	}
	else if(curFaceIndex == leftFaceID){
		return verticesLeftFace[gl_VertexID];
	}
	else if(curFaceIndex == rightFaceID){
		return verticesRightFace[gl_VertexID];
	}
	else if(curFaceIndex == frontFaceID){
		return verticesFrontFace[gl_VertexID];
	}
	else {
		return verticesBackFace[gl_VertexID];
	}

}

void main()
{
    uint currentInstancePosition = instancePosition;
    uint xPos = currentInstancePosition & 31;
    currentInstancePosition = currentInstancePosition >> 5;

    uint yPos = currentInstancePosition & 31;
    currentInstancePosition = currentInstancePosition >> 5;

    uint zPos = currentInstancePosition & 31;
    currentInstancePosition = currentInstancePosition >> 5;

    vec3 voxelPosition = vec3(float(xPos), float(yPos), float(zPos));

	uint curFace = currentInstancePosition & 7;

    vec3 vertexPosition = GetCurrentVertexBasedOnFaceIndex(curFace) + voxelPosition;



    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    texCoords = aTexCoords;
}