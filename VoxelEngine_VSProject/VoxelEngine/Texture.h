#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "glad/glad.h"

class Texture {

public:

    int textureIndex = -1;
    unsigned int textureID;
    std::string texturePath = "";

    inline static std::vector<Texture> textures;
    inline static std::unordered_map<std::string, int> texturePathToTextureIndex;
};