#pragma once

#include <string>

struct Model;
namespace ModelLoader
{
	bool loadModel(const std::string& filePath, Model& model);
}