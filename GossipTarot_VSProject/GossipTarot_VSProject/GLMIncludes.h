#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>


typedef glm::vec2 Vector2;
typedef glm::ivec2 Vector2Int;
typedef glm::vec3 Vector3;
typedef glm::ivec3 Vector3Int;
typedef glm::vec4 Vector4;
typedef glm::mat4 Mat4x4;

#ifdef _DEBUG

#include <iostream>

void PrintVector3(const Vector3& vector) {
	std::cout << vector.x << ", " << vector.z << ", " << vector.y << std::endl;
}

#endif