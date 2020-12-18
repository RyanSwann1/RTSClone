#pragma once

#include "glm/glm.hpp"

struct Mesh;
namespace CubeMeshGenerator
{
	void generateCubeMesh(Mesh& mesh, glm::vec3 position = glm::vec3(1.0f));
};