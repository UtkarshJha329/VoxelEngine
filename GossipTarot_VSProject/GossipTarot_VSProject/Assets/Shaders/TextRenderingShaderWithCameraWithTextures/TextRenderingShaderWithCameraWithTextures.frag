#version 460 core

// Out variables
out vec4 FragColor;

// In variables
in vec2 texCoords;

// Samplers
uniform sampler2D textureSampler;

void main()
{
    vec2 verticallyFlippedTexCoord = vec2(texCoords.x, texCoords.y);
    FragColor = vec4(1.0, 0.5, 0.1, texture(textureSampler, verticallyFlippedTexCoord).r);
} 