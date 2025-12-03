#pragma once

#include <vector>

class VertexLayout {

public:

	int numVertexAttribs = -1;
	std::vector<int> vertexAttribsPositions;
	std::vector<int> vertexAttribNumComponents;
	std::vector<size_t> vertexAttributeStride;
	std::vector<void*> pointerToFirstAttributeInVertex;
};

class MeshOnCPU {

public:

	std::vector<float> vertices;
	std::vector<int> indices;

	VertexLayout* vertexLayout = &MeshOnCPU::defaultVertexLayout;

	inline static VertexLayout defaultVertexLayout = {
		2,
		{0, 1},
		{3, 2},
		{5 * sizeof(float), 5 * sizeof(float)},
		{(void*)0, (void*)(3 * sizeof(float))}
	};

};