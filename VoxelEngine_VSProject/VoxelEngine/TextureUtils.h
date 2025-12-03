#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "Texture.h"

int TextureFromFile(std::string& path, bool flipTexture = false)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(flipTexture);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }


    std::string texturePath = path;
    int textureIndex = static_cast<int>(Texture::textures.size());
    Texture newTexture = Texture{ textureIndex, textureID, texturePath};

    Texture::texturePathToTextureIndex[texturePath] = textureIndex;
    Texture::textures.push_back(newTexture);

    return textureIndex;
}

int TextureGlyph(const std::string& fontPath, const std::string& characterName, const int& glyphWidth, const int& glyphHeight, unsigned char* glyphTextureData)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        glyphWidth,
        glyphHeight,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        glyphTextureData
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    std::string texturePath = fontPath + "/" + characterName;
    int textureIndex = static_cast<int>(Texture::textures.size());
    Texture newTexture = Texture{ textureIndex, textureID, texturePath };

    Texture::texturePathToTextureIndex[texturePath] = textureIndex;
    Texture::textures.push_back(newTexture);

    return textureIndex;
}