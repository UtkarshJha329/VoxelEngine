#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in uint instancePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texCoords;

void main()
{
    uint currentInstancePosition = instancePosition;
    uint xPos = currentInstancePosition & 31;
    currentInstancePosition = currentInstancePosition >> 5;
    
    uint yPos = currentInstancePosition & 31;
    currentInstancePosition = currentInstancePosition >> 5;

    uint zPos = currentInstancePosition & 31;

    vec3 voxelPosition = vec3(float(xPos), float(yPos), float(zPos));

    vec3 vertexPosition = aPos + voxelPosition;

    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
    texCoords = aTexCoords;
}