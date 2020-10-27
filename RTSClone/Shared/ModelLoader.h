#pragma once

#include <string>
#include <vector>

struct Mesh;
namespace ModelLoader
{
	bool loadModel(const std::string& fileName, std::vector<Mesh>& meshes);
}