#pragma once

#include <vector>

#include "GLMIncludes.h"
#include "Colour.h"

enum AnchorPosition {

	TopLeft,
	TopMiddle,
	TopRight,

	MiddleLeft,
	MiddleMiddle,
	MiddleRight,

	BottomLeft,
	BottomMiddle,
	BottomRight
};

enum UI_RectState {

	OnHoverEnter,
	OnHovering,
	OnHoverExit,
	OnNotHovering,

	OnClick,
	OnClickHeld,
	OnClickRelease
};

struct UI_Rect {

	int index = -1;

	Vector3 start;
	Vector3 end;

	AnchorPosition anchorPosition = TopLeft;

	UI_RectState uiRectState = UI_RectState::OnNotHovering;

	int parentIndex;

	Vector3 worldStartPos;
	Vector3 worldEndPos;

	//Colour normalColour;
	//Colour mouseHoverColour;

	std::vector<unsigned int> children;

	inline static std::vector<UI_Rect> uiRects;
};

float GetUIRectWidth(const UI_Rect& uiRect) {
	return abs(uiRect.end.x - uiRect.start.x);
}

float GetUIRectHeight(const UI_Rect& uiRect) {
	return abs(uiRect.end.y - uiRect.start.y);
}

Vector3 GetUIRectCentrePos(const UI_Rect& uiRect) {
	//return uiRect.worldStartPos + Vector3{ GetUIRectWidth(uiRect) * 0.5f, GetUIRectHeight(uiRect) * 0.5f, 0.0f };
	return (uiRect.worldStartPos + uiRect.worldEndPos) * 0.5f;
}