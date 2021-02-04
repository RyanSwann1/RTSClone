#include "ShaderHandler.h"
#include "glad.h"
#include "Globals.h"
#include "glm/gtc/type_ptr.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
	const std::string SHADER_DIRECTORY = "../Shared/Shaders/";
	const int INVALID_UNIFORM_LOCATION = -1;

	bool parseShaderFromFile(const std::string& filePath, std::string& shaderSource)
	{
		std::ifstream stream(filePath);
		if (!stream.is_open())
		{
			return false;
		}
		std::string line;
		std::stringstream stringStream;

		while (getline(stream, line))
		{
			stringStream << line << "\n";
		}

		shaderSource = stringStream.str();
		stream.close();
		stringStream.clear();

		return true;
	}

	bool createShaderProgram(unsigned int shaderID, const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
	{
		std::string vertexShaderSource;
		bool vertexShaderLoaded = parseShaderFromFile(SHADER_DIRECTORY + vertexShaderFilePath, vertexShaderSource);
		if (!vertexShaderLoaded)
		{
			std::cout << "Couldn't load " << vertexShaderFilePath << "\n";
			return false;
		}
		std::string fragmentShaderSource;
		bool fragmentShaderLoaded = parseShaderFromFile(SHADER_DIRECTORY + fragmentShaderFilePath, fragmentShaderSource);
		if (!fragmentShaderLoaded)
		{
			std::cout << "Couldn't load: " << fragmentShaderFilePath << "\n";
			return false;
		}

		//Create Vertex Shader
		unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		const char* vertexSrc = vertexShaderSource.c_str();
		glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
		glCompileShader(vertexShaderID);

		int vertexShaderResult = 0;
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &vertexShaderResult);
		if (vertexShaderResult == GL_FALSE)
		{
			int messageLength = 0;
			glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &messageLength);
			char* errorMessage = static_cast<char*>(alloca(messageLength * sizeof(char)));
			glGetShaderInfoLog(vertexShaderID, messageLength, &messageLength, errorMessage);
			std::cout << "Failed to compile: " << errorMessage << "\n";

			return false;
		}

		//Create Fragment Shader
		unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		const char* fragSrc = fragmentShaderSource.c_str();
		glShaderSource(fragmentShaderID, 1, &fragSrc, nullptr);
		glCompileShader(fragmentShaderID);

		int fragmentShaderResult = 0;
		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &fragmentShaderResult);
		if (fragmentShaderResult == GL_FALSE)
		{
			int messageLength = 0;
			glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &messageLength);
			char* errorMessage = static_cast<char*>(alloca(messageLength * sizeof(char)));
			glGetShaderInfoLog(fragmentShaderID, messageLength, &messageLength, errorMessage);
			std::cout << "Failed to compile: " << errorMessage << "\n";

			return false;
		}

		glAttachShader(shaderID, vertexShaderID);
		glAttachShader(shaderID, fragmentShaderID);
		glLinkProgram(shaderID);
		glValidateProgram(shaderID);

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);

		return true;
	}
}

//ShaderHandler
ShaderHandler::ShaderHandler()
	: m_currentShaderType(eShaderType::Default)
{}

std::unique_ptr<ShaderHandler> ShaderHandler::create()
{
	std::unique_ptr<ShaderHandler> shaderHandler = std::unique_ptr<ShaderHandler>(new ShaderHandler());
	size_t shaderLoadedCounter = 0;
	for (const auto& shader : shaderHandler->m_shaders)
	{
		switch (shader.getType())
		{
		case eShaderType::Default:
			if (createShaderProgram(shader.getID(), "VertexShader.glsl", "FragmentShader.glsl"))
			{
				++shaderLoadedCounter;
			}
			break;
		case eShaderType::Widjet:
			if (createShaderProgram(shader.getID(), "WidgetVertexShader.glsl", "WidgetFragmentShader.glsl"))
			{
				++shaderLoadedCounter;
			}
			break;
		case eShaderType::Debug:
			if (createShaderProgram(shader.getID(), "DebugVertexShader.glsl", "DebugFragmentShader.glsl"))
			{
				++shaderLoadedCounter;
			}
			break;
		default:
			assert(false);
		}
	}

	assert(shaderLoadedCounter == shaderHandler->m_shaders.size());
	return (shaderLoadedCounter == shaderHandler->m_shaders.size() ? std::move(shaderHandler) : std::unique_ptr<ShaderHandler>());
}

eShaderType ShaderHandler::getActiveShaderType() const
{
	return m_currentShaderType;
}

void ShaderHandler::setUniformMat4f(eShaderType shaderType, const std::string& uniformName, const glm::mat4& matrix)
{
	assert(shaderType == m_currentShaderType);
	int uniformLocation = m_shaders[static_cast<int>(shaderType)].getUniformLocation(uniformName);
	assert(uniformLocation != INVALID_UNIFORM_LOCATION);
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderHandler::setUniformVec3(eShaderType shaderType, const std::string& uniformName, const glm::vec3& v)
{
	assert(shaderType == m_currentShaderType);
	int uniformLocation = m_shaders[static_cast<int>(shaderType)].getUniformLocation(uniformName);
	assert(uniformLocation != INVALID_UNIFORM_LOCATION);
	glUniform3fv(uniformLocation, 1, &v[0]);
}

void ShaderHandler::setUniform1i(eShaderType shaderType, const std::string& uniformName, int value)
{
	assert(shaderType == m_currentShaderType);
	int uniformLocation = m_shaders[static_cast<int>(shaderType)].getUniformLocation(uniformName);
	assert(uniformLocation != INVALID_UNIFORM_LOCATION);
	glUniform1i(uniformLocation, value);
}

void ShaderHandler::setUniform1f(eShaderType shaderType, const std::string& uniformName, float value)
{
	assert(shaderType == m_currentShaderType);
	int uniformLocation = m_shaders[static_cast<int>(shaderType)].getUniformLocation(uniformName);
	assert(uniformLocation != INVALID_UNIFORM_LOCATION);
	glUniform1f(uniformLocation, value);
}

void ShaderHandler::switchToShader(eShaderType shaderType)
{
	//assert(shaderType != m_currentShaderType);
	m_currentShaderType = shaderType;
	glUseProgram(m_shaders[static_cast<int>(shaderType)].getID());
}

//Shader
ShaderHandler::Shader::Shader(eShaderType shaderType)
	: m_itemID(glCreateProgram()),
	m_type(shaderType),
	m_uniformLocations()
{}

ShaderHandler::Shader::~Shader()		
{
	assert(m_itemID != Globals::INVALID_OPENGL_ID);
	glDeleteProgram(m_itemID);
}

unsigned int ShaderHandler::Shader::getID() const
{
	return m_itemID;
}

eShaderType ShaderHandler::Shader::getType() const
{
	return m_type;
}

int ShaderHandler::Shader::getUniformLocation(const std::string& uniformName)
{
	if (m_uniformLocations.find(uniformName) != m_uniformLocations.cend())
	{
		return m_uniformLocations[uniformName];
	}
	else
	{
		int location = glGetUniformLocation(m_itemID, uniformName.c_str());
		if (location == -1)
		{
			std::cout << "Failed to find uniform: " << uniformName << "\n";
		}
		else
		{
			m_uniformLocations[uniformName] = location;
		}

		return location;
	}
}