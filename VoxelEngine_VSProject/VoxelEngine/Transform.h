#pragma once

#include "GLMIncludes.h"

class Transform {

public:

	Vector3 position;
	Vector3 rotation;
	Vector3 scale;

private:

	Mat4x4 translationMatrix;
	Mat4x4 rotationMatrix;
	Mat4x4 scaleMatrix;

	Mat4x4 transformMatrix;

public:

	inline static Vector3 worldForward = Vector3{ 0.0f, 0.0f, 1.0f };
	inline static Vector3 worldUp = Vector3{ 0.0f, 1.0f, 0.0f };
	inline static Vector3 worldRight = Vector3{ 1.0f, 0.0f, 0.0f };

public:

	Mat4x4& GetTransformMatrix() {

		//translationMatrix = Mat4x4(1.0f);
		//translationMatrix = glm::translate(translationMatrix, position);

		//glm::quat rotQuat = glm::rotate(rotQuat, rotation.x, Vector3(1.0f, 0.0f, 0.0f));
		//rotQuat = glm::rotate(rotQuat, rotation.y, Vector3(0.0f, 1.0f, 0.0f));
		//rotQuat = glm::rotate(rotQuat, rotation.z, Vector3(0.0f, 0.0f, 1.0f));
		//rotationMatrix = glm::mat4_cast(rotQuat);

		//scaleMatrix = Mat4x4(1.0f);
		//scaleMatrix = glm::scale(scaleMatrix, scale);

		Mat4x4 tempMatrix = Mat4x4(1.0f);
		//Vector3 flippedZPos = { position.x, position.y, position.z };
		//tempMatrix = glm::translate(tempMatrix, flippedZPos);
		tempMatrix = glm::translate(tempMatrix, position);

		//glm::quat rotQuat = glm::identity<glm::quat>();
		//rotQuat = glm::rotate(rotQuat, glm::radians(rotation.x), Vector3(1.0f, 0.0f, 0.0f));
		//rotQuat = glm::rotate(rotQuat, glm::radians(rotation.y), Vector3(0.0f, 1.0f, 0.0f));
		//rotQuat = glm::rotate(rotQuat, glm::radians(rotation.z), Vector3(0.0f, 0.0f, 1.0f));
		//rotationMatrix = glm::mat4_cast(rotQuat);

		glm::quat rotQuat = glm::quat(glm::radians(rotation));
		rotationMatrix = glm::mat4_cast(rotQuat);


		//rotationMatrix = Mat4x4(1.0f);
		//rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.x), Vector3(1.0f, 0.0f, 0.0f));
		//rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), Vector3(0.0f, 1.0f, 0.0f));
		//rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), Vector3(0.0f, 0.0f, 1.0f));

		tempMatrix *= rotationMatrix;

		tempMatrix = glm::scale(tempMatrix, scale);

		transformMatrix = tempMatrix;

		return transformMatrix;
	}
};