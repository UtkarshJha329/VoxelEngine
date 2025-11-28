#version 460 core

layout(early_fragment_tests) in;

// Out variables
//out vec4 FragColor;

// In variables
in float distFromCamera;
in vec2 texCoords;
in flat uint chunkFlattenedIndex;

layout (binding = 7, std430) buffer ChunksVisibilityData {
	uint chunksVisibilityData[];
};

void main()
{

//    chunksVisibilityData[chunkFlattenedIndex] = 1;
//
//    if(texCoords.x > 1 || texCoords.y > 1 || texCoords.x < 0 || texCoords.y < 0){
//        discard;
//    }

    chunksVisibilityData[chunkFlattenedIndex] = 1;
//    FragColor = vec4(vec3(distFromCamera / 1000), 0.0);
//  FragColor = vec4(0.0);
} 