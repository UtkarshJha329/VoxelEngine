#pragma once

const float QuadVertices[] = {
	 0.5f,  0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	-0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f
};


const float QuadVerticesWithTexCoords[] = {
	// positions			// texture coords
	-0.5f,  0.5f, 0.0f,		0.0f, 0.0f,    // top left 
	-0.5f, -0.5f, 0.0f,		0.0f, 1.0f,   // bottom left
	 0.5f, -0.5f, 0.0f,		1.0f, 1.0f,   // bottom right
	 0.5f,  0.5f, 0.0f,		1.0f, 0.0f,   // top right
};

const unsigned int QuadIndices[] = {
	0, 1, 2,
	2, 3, 0
};


const float CubeVerticesWithTexCoords[] = {
	// Front face (Z+)
	-0.5f, -0.5f,  0.5f,	0.0f, 1.0f,   // bottom left
	 0.5f, -0.5f,  0.5f,	1.0f, 1.0f,   // bottom right
	 0.5f,  0.5f,  0.5f,	1.0f, 0.0f,   // top right
	-0.5f,  0.5f,  0.5f,	0.0f, 0.0f,   // top left

	// Back face (Z-)
	 0.5f, -0.5f, -0.5f,	0.0f, 1.0f,   // bottom left
	-0.5f, -0.5f, -0.5f,	1.0f, 1.0f,   // bottom right
	-0.5f,  0.5f, -0.5f,	1.0f, 0.0f,   // top right
	 0.5f,  0.5f, -0.5f,	0.0f, 0.0f,   // top left

	 // Left face (X-)
	-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,   // bottom left
	-0.5f, -0.5f,  0.5f,	1.0f, 1.0f,   // bottom right
	-0.5f,  0.5f,  0.5f,	1.0f, 0.0f,   // top right
	-0.5f,  0.5f, -0.5f,	0.0f, 0.0f,   // top left

	// Right face (X+)
	0.5f, -0.5f,  0.5f,		0.0f, 1.0f,   // bottom left
	0.5f, -0.5f, -0.5f,		1.0f, 1.0f,   // bottom right
	0.5f,  0.5f, -0.5f,		1.0f, 0.0f,   // top right
	0.5f,  0.5f,  0.5f,		0.0f, 0.0f,   // top left

	// Top face (Y+)
	-0.5f,  0.5f,  0.5f,	0.0f, 1.0f,   // bottom left
	 0.5f,  0.5f,  0.5f,	1.0f, 1.0f,   // bottom right
	 0.5f,  0.5f, -0.5f,	1.0f, 0.0f,   // top right
	-0.5f,  0.5f, -0.5f,	0.0f, 0.0f,   // top left

	// Bottom face (Y-)
	-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,   // bottom left
	 0.5f, -0.5f, -0.5f,	1.0f, 1.0f,   // bottom right
	 0.5f, -0.5f,  0.5f,	1.0f, 0.0f,   // top right
	-0.5f, -0.5f,  0.5f,	0.0f, 0.0f	// top left
};

const unsigned int CubeIndices[] = {
	// Front
	0, 1, 2,
	2, 3, 0,

	// Back
	4, 5, 6,
	6, 7, 4,

	// Left
	8, 9, 10,
	10, 11, 8,

	// Right
	12, 13, 14,
	14, 15, 12,

	// Top
	16, 17, 18,
	18, 19, 16,

	// Bottom
	20, 21, 22,
	22, 23, 20
};

const unsigned int CubeFrontFaceIndices[] = {
	0, 1, 2,
	2, 3, 0,
};

const unsigned int CubeBackFaceIndices[] = {
	4, 5, 6,
	6, 7, 4,
};

const unsigned int CubeLeftFaceIndices[] = {
	8, 9, 10,
	10, 11, 8,
};

const unsigned int CubeRightFaceIndices[] = {
	12, 13, 14,
	14, 15, 12,
};

const unsigned int CubeTopFaceIndices[] = {
	16, 17, 18,
	18, 19, 16,
};

const unsigned int CubeBottomFaceIndices[] = {
	20, 21, 22,
	22, 23, 20
};