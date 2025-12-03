#pragma once

#include "GLMIncludes.h"

#include "Texture.h"

class Viewport {

public:
	Vector2 dimensions;
	Texture viewportTexture;

	float GetAspectRatio() {
		return dimensions.x / dimensions.y;
	}
};