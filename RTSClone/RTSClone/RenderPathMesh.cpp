#include "RenderPathMesh.h"
#ifdef RENDER_PATHING
#include "ShaderHandler.h"
#include "Mesh.h"
#include "Globals.h"
#include <array>

namespace
{
	const int CUBE_FACE_INDICIE_COUNT = 4;
	const std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};
	const glm::vec3 PATH_COLOUR = { 1.0f, 0.27f, 0.0f };
	const float PATH_OPACITY = 0.25f;
}

void RenderPathMesh::generate(const std::vector<glm::vec3>& path, Mesh& mesh)
{
	const std::array<glm::vec3, 4> CUBE_FACE_TOP =
	{
		glm::vec3(0.0f, 0.0f, Globals::NODE_SIZE),
		glm::vec3(Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE),
		glm::vec3(Globals::NODE_SIZE, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f)
	};

	mesh.indices.clear();
	mesh.vertices.clear();

	int elementCount = 0;
	for (const auto& pathNode : path)
	{
		for (const auto& i : CUBE_FACE_TOP)
		{
			glm::vec3 position = { pathNode + i };
			position.x -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
			position.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
			mesh.vertices.emplace_back(position);
		}

		for (unsigned int i : CUBE_FACE_INDICIES)
		{
			mesh.indices.push_back(i + elementCount);
		}

		elementCount += CUBE_FACE_INDICIE_COUNT;
	}

	mesh.attachToVAO();
}

void RenderPathMesh::render(ShaderHandler& shaderHandler, const std::vector<glm::vec3>& pathToPosition, Mesh& renderPathMesh)
{
	if (!pathToPosition.empty())
	{
		shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", PATH_COLOUR);
		shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", PATH_OPACITY);
		RenderPathMesh::generate(pathToPosition, renderPathMesh);
		renderPathMesh.renderDebugMesh(shaderHandler);
	}
}
#endif // RENDER_PATHING