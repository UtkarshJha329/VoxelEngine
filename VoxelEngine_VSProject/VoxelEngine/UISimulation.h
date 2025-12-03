#pragma once

#include <iostream>
#include <stack> // Using stack for event queue instead of Queue because I want children's events to be cleared up first as traversal for state checking and changing happens from root to child.

#include "Input.h"
#include "UIRect.h"

struct UIEventsData {
	int uiRectId;
	UI_RectState state;
};

std::stack<UIEventsData> uiEvents;

// Adds a copy! Not the one provided!
void AddUIRectAsChildToUIRect(UI_Rect& child, const int& parentIndex) {

	child.parentIndex = parentIndex;

	int childIndex = UI_Rect::uiRects.size();
	UI_Rect::uiRects.push_back(child);
	UI_Rect::uiRects[parentIndex].children.push_back(childIndex);
}

bool PointLiesInsideUIRect(const UI_Rect& uiRect, const Vector2& point) {

	//std::cout << "Checking point, (" << point.x << ", " << point.y << ")" << " in rec := " << uiRect.index 
	//			<< " Bounds : (minX : " << uiRect.worldStartPos.x << ", maxX : " << uiRect.worldEndPos.x 
	//			<< ", minY : " << uiRect.worldStartPos.y << ", maxY : " << uiRect.worldEndPos.y << ")" << std::endl;

	float rectMinX = glm::min(uiRect.worldStartPos.x, uiRect.worldEndPos.x);
	float rectMaxX = glm::max(uiRect.worldStartPos.x, uiRect.worldEndPos.x);
	float rectMinY = glm::min(uiRect.worldStartPos.y, uiRect.worldEndPos.y);
	float rectMaxY = glm::max(uiRect.worldStartPos.y, uiRect.worldEndPos.y);

	return point.x >= rectMinX &&
		point.x <= rectMaxX &&
		point.y >= rectMinY &&
		point.y <= rectMaxY;

}

void SetUIRectState(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {

	if (PointLiesInsideUIRect(uiRect, { mouseX, mouseY })) {

		if (uiRect.uiRectState == UI_RectState::OnNotHovering)
		{
			//std::cout << uiRect.index << " entered hover." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHoverEnter;
		}
		else if (uiRect.uiRectState == UI_RectState::OnHoverEnter) {

			//std::cout << uiRect.index << " hovering." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHovering;
		}
	}
	else {
		if (uiRect.uiRectState == UI_RectState::OnHoverEnter || uiRect.uiRectState == UI_RectState::OnHovering) {

			//std::cout << uiRect.index << " hover exit." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHoverExit;
		}
		else {

			//std::cout << uiRect.index << " not hovering." << std::endl;
			uiRect.uiRectState = UI_RectState::OnNotHovering;
		}
	}

	uiEvents.push({ uiRect.index, uiRect.uiRectState });

}

bool AddDeltaToUIRectLocalPosition(const int& uiRectIndex, const float& deltaX, const float& deltaY) {

	UI_Rect& uiRect = UI_Rect::uiRects[uiRectIndex];
	UI_Rect& parentUIRect = UI_Rect::uiRects[uiRect.parentIndex];

	Vector3 delta = Vector3{ deltaX, -deltaY, 0.0f };

	Vector3 modifiedWorldStart = uiRect.worldStartPos + delta;
	Vector3 modifiedWorldEnd = uiRect.worldEndPos + delta;

	bool modifiedStartAndEndLieInside = PointLiesInsideUIRect(parentUIRect, modifiedWorldStart) && PointLiesInsideUIRect(parentUIRect, modifiedWorldEnd);

	if (modifiedStartAndEndLieInside) {

		uiRect.worldStartPos += delta;
		uiRect.worldEndPos += delta;

		uiRect.start += delta;
		uiRect.end += delta;

		return true;
	}

	return false;
}

void TransformUIPositionsToParentSpaceBasedOnAnchor(Vector3& start, Vector3& end, const UI_Rect& uiRect, const UI_Rect& parentUIRect) {

	float widthParent = GetUIRectWidth(parentUIRect);
	float heightParent = GetUIRectHeight(parentUIRect);

	if (uiRect.anchorPosition == AnchorPosition::TopLeft) {

		start = { uiRect.start.x - (widthParent / 2.0f), uiRect.start.y + (heightParent / 2.0f), uiRect.start.z };
		end = { uiRect.end.x - (widthParent / 2.0f), uiRect.end.y + (heightParent / 2.0f), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::TopMiddle) {

		start = { (uiRect.start.x), uiRect.start.y + (heightParent / 2.0f) , uiRect.start.z };
		end = { (uiRect.end.x), uiRect.end.y + (heightParent / 2.0f), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::TopRight) {

		start = { uiRect.start.x + (widthParent / 2.0f), uiRect.start.y + (heightParent / 2.0f), uiRect.start.z };
		end = { uiRect.end.x + (widthParent / 2.0f), uiRect.end.y + (heightParent / 2.0f), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleLeft) {

		start = { uiRect.start.x - (widthParent / 2.0f), uiRect.start.y, uiRect.start.z };
		end = { uiRect.end.x - (widthParent / 2.0f), uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleMiddle) {

		start = { (uiRect.start.x), (uiRect.start.y), uiRect.start.z };
		end = { (uiRect.end.x), (uiRect.end.y), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleRight) {

		start = { uiRect.start.x + (widthParent / 2.0f), (uiRect.start.y), uiRect.start.z };
		end = { uiRect.end.x + (widthParent / 2.0f), (uiRect.end.y), uiRect.end.z };

	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomLeft) {

		start = { uiRect.start.x - (widthParent / 2.0f), uiRect.start.y - (heightParent / 2.0f), uiRect.start.z };
		end = { uiRect.end.x - (widthParent / 2.0f), uiRect.end.y - (heightParent / 2.0f), uiRect.end.z };

	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomMiddle) {

		start = { (uiRect.start.x), uiRect.start.y - (heightParent / 2.0f) , uiRect.start.z };
		end = { (uiRect.end.x), uiRect.end.y - (heightParent / 2.0f), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomRight) {

		start = { uiRect.start.x + (widthParent / 2.0f), uiRect.start.y - (heightParent / 2.0f), uiRect.start.z };
		end = { uiRect.end.x + (widthParent / 2.0f), uiRect.end.y - (heightParent / 2.0f), uiRect.end.z };
	}
}

void SetUIRectStatesToNotHovering(UI_Rect& rootUiRect) {

	rootUiRect.uiRectState = UI_RectState::OnNotHovering;

	for (int i = 0; i < rootUiRect.children.size(); i++)
	{
		SetUIRectStatesToNotHovering(UI_Rect::uiRects[rootUiRect.children[i]]);
	}

}

void UpdateUIRectStates(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {
	SetUIRectState(uiRect, mouseX, mouseY);
	if (uiRect.uiRectState == UI_RectState::OnHovering) {
		//uiRect.colour = uiRect.mouseHoverColour;
		//std::cout << "Hovering : START : over uiRect with index := " << uiRect.index << std::endl;
	}
	else if (uiRect.uiRectState == UI_RectState::OnHoverExit) {
		//uiRect.colour = uiRect.normalColour;
		//std::cout << "Hovering : END : over uiRect with index := " << uiRect.index << std::endl;
	}
}

void UpdateUITreeStates(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {

	UpdateUIRectStates(uiRect, mouseX, mouseY);

	if (uiRect.uiRectState != UI_RectState::OnNotHovering)
	{
		for (int i = 0; i < uiRect.children.size(); i++)
		{
			UpdateUITreeStates(UI_Rect::uiRects[uiRect.children[i]], mouseX, mouseY);
		}
	}
	else
	{
		for (int i = 0; i < uiRect.children.size(); i++)
		{
			SetUIRectStatesToNotHovering(UI_Rect::uiRects[uiRect.children[i]]);
		}
	}
}

void MakeHoveringUIRectsFollowCursorMovement(const int& childIndex, const float& mouseDeltaX, const float& mouseDeltaY) {

	if (childIndex != 0) {
		AddDeltaToUIRectLocalPosition(childIndex, mouseDeltaX, mouseDeltaY);
	}
}

int curSelectedUIRectIndex = -1;

void HandleUIEvents(const float& mouseDeltaX, const float& mouseDeltaY) {

	while (!uiEvents.empty()) {
		if (uiEvents.top().state == UI_RectState::OnHovering && GetKeyHeld(KeyCode::MOUSE_BUTTON_LEFT)) {

			if (curSelectedUIRectIndex == -1 || UI_Rect::uiRects[uiEvents.top().uiRectId].index == curSelectedUIRectIndex) {
				curSelectedUIRectIndex = uiEvents.top().uiRectId;
				//std::cout << "Selecting uiRect index : " << curSelectedUIRectIndex << std::endl;
				MakeHoveringUIRectsFollowCursorMovement(uiEvents.top().uiRectId, mouseDeltaX, mouseDeltaY);
			}
		}

		uiEvents.pop();
	}

	if (GetKeyReleasedInThisFrame(KeyCode::MOUSE_BUTTON_LEFT)) {
		curSelectedUIRectIndex = -1;
	}
}

Vector2 ConvertMousePosToScreenUIPos(const float& mouseX, const float& mouseY, const float& screen_width, const float& screen_height) {

	float halfWidth = screen_width / 2.0f;
	float halfHeight = screen_height / 2.0f;

	return { mouseX - halfWidth, halfHeight - mouseY };
}