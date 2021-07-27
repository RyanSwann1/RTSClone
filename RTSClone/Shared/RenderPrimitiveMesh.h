#pragma once

#include "glm/glm.hpp"
#include <array>
#include <vector>

struct Mesh;
class AABB;
class ShaderHandler;
namespace RenderPrimitiveMesh
{
	void generate(const std::vector<glm::vec3>& path, Mesh& mesh);
	void render(ShaderHandler& shaderHandler, const glm::vec3& position, Mesh& mesh);
	void render(ShaderHandler& shaderHandler, const std::vector<glm::vec3>& pathToPosition, Mesh& renderPathMesh);
#ifdef RENDER_AABB
	void generate(AABB& aabb);
	void render(ShaderHandler& shaderHandler, AABB& aabb, const glm::vec3& colour, float opacity);
#endif // RENDER_AABB
}