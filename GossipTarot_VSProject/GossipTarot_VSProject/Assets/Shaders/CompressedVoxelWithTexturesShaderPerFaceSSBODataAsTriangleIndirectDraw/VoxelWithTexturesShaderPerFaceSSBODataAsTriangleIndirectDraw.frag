#version 460 core

// Out variables
out vec4 FragColor;

// In variables
in vec2 texCoords;
in vec4 chunkDebugColour;

// Samplers
uniform sampler2D textureSampler;

void main()
{
    if(texCoords.x > 1 || texCoords.y > 1 || texCoords.x < 0 || texCoords.y < 0){
        discard;
    }

//    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    FragColor = texture(textureSampler, texCoords) * chunkDebugColour;
} 