#pragma once

#include <vector>
#include <string>

#include "MeshUtils.h"

class Model {

public:

	std::vector<MeshOnCPU> meshesOnCPU;

	std::vector<MeshOnGPU> meshOnGPU;

	std::string modelFilePath;

};