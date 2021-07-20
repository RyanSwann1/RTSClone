#include "RenderPrimitiveMesh.h"
#include "ShaderHandler.h"
#include "Mesh.h"
#include "Globals.h"
#include "AABB.h"
#include <array>

namespace
{
	constexpr int CUBE_FACE_INDICIE_COUNT = 4;
	constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};
	constexpr std::array<glm::vec3, 4> CUBE_FACE_TOP =
	{
		glm::vec3(0.0f, 0.0f, Globals::NODE_SIZE),
		glm::vec3(Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE),
		glm::vec3(Globals::NODE_SIZE, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f)
	};
	constexpr glm::vec3 PATH_COLOUR = { 1.0f, 0.27f, 0.0f };
	constexpr float PATH_OPACITY = 0.25f;
}

void RenderPrimitiveMesh::generate(const std::vector<glm::vec3>& path, Mesh& mesh)
{
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

void RenderPrimitiveMesh::generate(AABB& aabb)
{
	aabb.mesh.vertices.clear();
	aabb.mesh.indices.clear();

	const std::array<glm::vec3, 4> CUBE_FACE_TOP =
	{
		glm::vec3(aabb.getLeft(), Globals::GROUND_HEIGHT, aabb.getBack()),
		glm::vec3(aabb.getRight(), Globals::GROUND_HEIGHT, aabb.getBack()),
		glm::vec3(aabb.getRight(), Globals::GROUND_HEIGHT, aabb.getForward()),
		glm::vec3(aabb.getLeft(), Globals::GROUND_HEIGHT, aabb.getForward())
	};

	for (const auto& i : CUBE_FACE_TOP)
	{
		aabb.mesh.vertices.emplace_back(i);
	}

	for (unsigned int i : CUBE_FACE_INDICIES)
	{
		aabb.mesh.indices.push_back(i);
	}

	aabb.mesh.attachToVAO();
}

void RenderPrimitiveMesh::render(ShaderHandler& shaderHandler, const std::vector<glm::vec3>& pathToPosition, Mesh& renderPathMesh)
{
	if (!pathToPosition.empty())
	{
		shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", PATH_COLOUR);
		shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", PATH_OPACITY);
		RenderPrimitiveMesh::generate(pathToPosition, renderPathMesh);
		renderPathMesh.renderDebugMesh(shaderHandler);
	}
}

void RenderPrimitiveMesh::render(ShaderHandler& shaderHandler, AABB& aabb, const glm::vec3& colour, float opacity)
{
	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", colour);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", opacity);
	aabb.mesh.renderDebugMesh(shaderHandler);
}

void RenderPrimitiveMesh::render(ShaderHandler& shaderHandler, const glm::vec3& position, Mesh& mesh)
{
	mesh.indices.clear();
	mesh.vertices.clear();

	for (const auto& i : CUBE_FACE_TOP)
	{
		glm::vec3 pos = { position + i };
		pos.x -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
		pos.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
		mesh.vertices.emplace_back(position);
	}

	for (unsigned int i : CUBE_FACE_INDICIES)
	{
		mesh.indices.push_back(i);
	}

	mesh.attachToVAO();
	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", PATH_COLOUR);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", 1.0f);
	mesh.renderDebugMesh(shaderHandler);
}