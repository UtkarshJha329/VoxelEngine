#pragma once

#include "glad/glad.h"

#include "MeshUtils.h"

struct CommonChunkBoundingMesh {

public:

    const float cubeVertices[24] = {
        // position (x, y, z)

        0.0f,  0.0f,  0.0f,   // 0
        32.0f, 0.0f,  0.0f,   // 1
        32.0f, 32.0f, 0.0f,   // 2
        0.0f,  32.0f, 0.0f,   // 3

        0.0f,  0.0f,  32.0f,  // 4
        32.0f, 0.0f,  32.0f,  // 5
        32.0f, 32.0f, 32.0f,  // 6
        0.0f,  32.0f, 32.0f   // 7
    };

    const unsigned int cubeIndices[36] = {
        // front face (z = 0)
        0, 1, 2,
        2, 3, 0,

        // right face (x = 32)
        1, 5, 6,
        6, 2, 1,

        // back face (z = 32)
        5, 4, 7,
        7, 6, 5,

        // left face (x = 0)
        4, 0, 3,
        3, 7, 4,

        // top face (y = 32)
        3, 2, 6,
        6, 7, 3,

        // bottom face (y = 0)
        4, 5, 1,
        1, 0, 4
    };

    MeshOnCPU boundingCubeMeshOnCPU;
    MeshOnGPU boundingCubeMeshOnGPU;

public:

    void CreateBoundingBoxMeshOnCPU() {

        for (int i = 0; i < 20; i++)
        {
            //simpleQuadMeshCPU.vertices.push_back(vertices[i]);
            boundingCubeMeshOnCPU.vertices.push_back(cubeVertices[i]);
        };

        for (int i = 0; i < 6; i++)
        {
            boundingCubeMeshOnCPU.indices.push_back(cubeIndices[i]);
        }
    }

    void CreateBoundingBoxMeshOnGPU() {
        CreateMeshOnGPU(boundingCubeMeshOnCPU, boundingCubeMeshOnGPU);
    }

};