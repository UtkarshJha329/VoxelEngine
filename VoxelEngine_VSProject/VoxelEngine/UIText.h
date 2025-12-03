#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "TextureUtils.h"

#include "GLMIncludes.h"
#include "UIRect.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct Character {

public:

    unsigned int textureIndex;
    int characterUIRectIndex;
    Vector2Int bearing;    // Offset from baseline to (left,top) of glyph
    unsigned int advance;    // Offset horizontally to advance to next glyph
};

const static inline int numCharactersReadFromZero = 128;
static inline std::vector<Character> charactersPerCharacter;

int InitializeCharacterRects(const std::string& fontPath, int rootUIIndex) {

    charactersPerCharacter.resize(numCharactersReadFromZero);

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -2;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph of character [ " << (char)c << "]" << std::endl;
            return -3;
        }

        // generate texture
        std::string characterName(1, static_cast<char>(c));
        int textureIndex = TextureGlyph(fontPath, characterName, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);

        //VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
        int characterUIRectIndex = UI_Rect::uiRects.size();  // Must be worked upon to be pararalised.
        //AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        float glyphWidth = (float)face->glyph->bitmap.width;
        float glyphHeight = (float)face->glyph->bitmap.rows;
        Vector3 characterRectStart = { -glyphWidth * 0.5f, -glyphHeight * 0.5f, 0.0f };
        Vector3 characterRectEnd = { glyphWidth * 0.5f, glyphHeight * 0.5f, 0.0f };

        UI_Rect characterRect = { characterUIRectIndex, characterRectStart, characterRectEnd, MiddleMiddle };
        //AddUIRectAsChildToUIRect(characterRect, rootUIIndex);
        UI_Rect::uiRects.push_back(characterRect);

        //std::cout << static_cast<int>(c) << ". Character " << (char)c << "'s ui rect width is := " << face->glyph->bitmap.width << std::endl;

        // now store character for later use
        Character character = {
            textureIndex,
            characterUIRectIndex,
            {static_cast<int>(face->glyph->bitmap_left), static_cast<int>(face->glyph->bitmap_top)},
            static_cast<unsigned int>(face->glyph->advance.x)
        };


        charactersPerCharacter[c] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return 1;
}

void RenderTextUICharacter(const Vector3& position, const AnchorPosition& anchorPosition, const int& characterUIRectIndex, const UI_Rect& parentUIRect, const ShaderProgram& shaderProgramForRendering, const int& textureIndex, const MeshOnGPU& meshOnGPU) {

    UI_Rect uiRect = UI_Rect::uiRects[characterUIRectIndex];
    uiRect.anchorPosition = anchorPosition;

    Vector3 start = uiRect.start;
    Vector3 end = uiRect.end;

    TransformUIPositionsToParentSpaceBasedOnAnchor(start, end, uiRect, parentUIRect);

    uiRect.worldStartPos = GetUIRectCentrePos(parentUIRect) + start;
    uiRect.worldEndPos = GetUIRectCentrePos(parentUIRect) + end;

    Mat4x4 uiRectTransformMatrix = MatrixOfUIRectToRenderWithPositionOffset(uiRect, position);
    RenderQuad(shaderProgramForRendering, uiRectTransformMatrix, textureIndex, meshOnGPU);
}

void RenderText(const ShaderProgram& shaderProgramForRendering, const AnchorPosition& anchorPositionOfText, const MeshOnGPU& meshToRenderWith, const std::string text, const Vector3& textPosition, const int& rootUIIndex)
{
    int x = textPosition.x;
    int y = textPosition.y;

    // iterate through all characters
    for (int i = 0; i < text.length(); i++)
    {
        Character currentCharacter = charactersPerCharacter[static_cast<int>(text[i])];

        Vector2Int currentCharacterSize = { GetUIRectWidth(UI_Rect::uiRects[currentCharacter.characterUIRectIndex]), GetUIRectHeight(UI_Rect::uiRects[currentCharacter.characterUIRectIndex]) };

        float verticalOffsetOfLetterFromOriginOfGivenPos = (currentCharacterSize.y * 0.5f) - (currentCharacterSize.y - currentCharacter.bearing.y);
        float yPos = y + verticalOffsetOfLetterFromOriginOfGivenPos;

        Vector3 positionOffsetOfCharacterRect = { x + currentCharacter.bearing.x, yPos, textPosition.z };

        RenderTextUICharacter(positionOffsetOfCharacterRect, anchorPositionOfText, currentCharacter.characterUIRectIndex, UI_Rect::uiRects[rootUIIndex], shaderProgramForRendering, currentCharacter.textureIndex, meshToRenderWith);

        x += (currentCharacter.advance >> 6); // bitshift by 6 to get value in pixels (2^6 = 64)
    }
}