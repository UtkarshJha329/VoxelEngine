#version 460 core

// Out variables
out vec4 FragColor;

// In variables
in vec2 texCoords;

// Samplers
uniform sampler2D textureSampler;

void main()
{
//    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    FragColor = texture(textureSampler, texCoords);
} 