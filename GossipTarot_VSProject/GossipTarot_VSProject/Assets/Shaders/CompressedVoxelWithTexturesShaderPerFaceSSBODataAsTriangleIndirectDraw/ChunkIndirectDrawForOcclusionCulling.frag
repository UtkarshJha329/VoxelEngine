#version 460 core

layout(early_fragment_tests) in;

// Out variables
out vec4 FragColor;

// In variables
// in float distFromCamera;
//in vec2 texCoords;
in flat uint chunkFlattenedIndex;

layout (binding = 7, std430) buffer ChunksVisibilityData {
	int chunksVisibilityData[];
};

void main()
{

//    if(texCoords.x > 1 || texCoords.y > 1 || texCoords.x < 0 || texCoords.y < 0){
//        discard;
//    }
//
    chunksVisibilityData[chunkFlattenedIndex] = 1;
//    atomicAdd(chunksVisibilityData[chunkFlattenedIndex.x], 10);

	FragColor = vec4(0, 1, 0, 1);

} 