#pragma once

#include <vector>
#include <string>

#include "glad/glad.h"

#include "FileHandling.h"

enum class SHADER_TYPE {
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	COMPUTE_SHADER
};

class Shader {

public:
	SHADER_TYPE shaderType;
	std::string shaderFilePath = "";
	//const char* shaderText;

	int shaderID;

public:

	int CreateShader() {

		ContentsOfFile shaderContentsOfFile = ReturnContentsOfFile(shaderFilePath);
		if (shaderContentsOfFile.parsedState != 1) {
			if (shaderType == SHADER_TYPE::VERTEX_SHADER) {
				std::cout << "Failed to read vertex shader file." << std::endl;
			}
			else if (shaderType == SHADER_TYPE::FRAGMENT_SHADER) {
				std::cout << "Failed to read fragment shader file." << std::endl;
			}
			else if (shaderType == SHADER_TYPE::COMPUTE_SHADER) {
				std::cout << "Failed to read compute shader file." << std::endl;
			}
			return -1;
		}

		const char* curShaderText = shaderContentsOfFile.contents.c_str();

		if (shaderType == SHADER_TYPE::VERTEX_SHADER) {
			shaderID = glCreateShader(GL_VERTEX_SHADER);
		}
		else if (shaderType == SHADER_TYPE::FRAGMENT_SHADER) {
			shaderID = glCreateShader(GL_FRAGMENT_SHADER);
		}
		else if (shaderType == SHADER_TYPE::COMPUTE_SHADER) {
			shaderID = glCreateShader(GL_COMPUTE_SHADER);
		}

		glShaderSource(shaderID, 1, &curShaderText, NULL);
		glCompileShader(shaderID);

		int success;
		char infoLog[1024];
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
			if (shaderType == SHADER_TYPE::VERTEX_SHADER) {
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			else if (shaderType == SHADER_TYPE::FRAGMENT_SHADER) {
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			else if (shaderType == SHADER_TYPE::COMPUTE_SHADER) {
				std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
		};

		return 1;
	}
};

class ShaderProgram {

public:

	int shaderProgramID;

public:

	void CreateShaderProgram(std::vector<Shader> shaders) {

		shaderProgramID = glCreateProgram();

		glUseProgram(shaderProgramID);

		for (int i = 0; i < shaders.size(); i++)
		{
			glAttachShader(shaderProgramID, shaders[i].shaderID);
		}

		glLinkProgram(shaderProgramID);

		int success;
		char infoLog[1024];
		glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderProgramID, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		for (int i = 0; i < shaders.size(); i++)
		{
			glDeleteShader(shaders[i].shaderID);
		}
	}
};