#pragma once

#ifdef RENDER_PATHING
#include "glm/glm.hpp"
#include <vector>
struct Mesh;
class ShaderHandler;
namespace RenderPathMesh
{
	void generate(const std::vector<glm::vec3>& path, Mesh& mesh);
	void render(ShaderHandler& shaderHandler, const std::vector<glm::vec3>& pathToPosition, Mesh& renderPathMesh);
}
#endif // RENDER_PATHING