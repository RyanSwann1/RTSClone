#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "PathFinding.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;

	glm::vec3 moveTowards(const glm::vec3& currentPosition, const glm::vec3& targetPosition, float maxDistanceDelta)
	{
		float magnitude = glm::distance(targetPosition, currentPosition);
		if (magnitude <= maxDistanceDelta || magnitude == 0.0f)
		{
			return targetPosition;
		}

		return currentPosition + glm::vec3(targetPosition - currentPosition) / magnitude * maxDistanceDelta;
	}

#ifdef RENDER_PATHING
	constexpr glm::vec3 PATH_COLOUR = { 1.0f, 0.27f, 0.0f };

	void generateRenderPath(const std::vector<glm::vec3>& path, Mesh& mesh)
	{
		constexpr int CUBE_FACE_INDICIE_COUNT = 4;
		constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
		{
			0, 1, 2,
			2, 3, 0
		};

		constexpr std::array<glm::vec3, 4> CUBE_FACE_TOP =
		{
			glm::vec3(0, Globals::NODE_SIZE, Globals::NODE_SIZE),
			glm::vec3(Globals::NODE_SIZE, Globals::NODE_SIZE, Globals::NODE_SIZE),
			glm::vec3(Globals::NODE_SIZE, Globals::NODE_SIZE, 0),
			glm::vec3(0, Globals::NODE_SIZE, 0)
		};

		mesh.m_indices.clear();
		mesh.m_vertices.clear();

		int elementCount = 0;
		for (const auto& pathNode : path)
		{
			for (const auto& i : CUBE_FACE_TOP)
			{
				mesh.m_vertices.emplace_back(pathNode + i);
			}

			for (unsigned int i : CUBE_FACE_INDICIES)
			{
				mesh.m_indices.push_back(i + elementCount);
			}

			elementCount += CUBE_FACE_INDICIE_COUNT;
		}

		mesh.attachToVAO();
	};

#endif // RENDER_PATHING
}

Unit::Unit(const glm::vec3& startingPosition)
	: m_position(startingPosition), 
	m_front(),
	m_AABB(startingPosition, 5.0f),
	m_selected(false),
	m_pathToPosition()
{}

const AABB& Unit::getAABB() const
{
	return m_AABB;
}

bool Unit::isSelected() const
{
	return m_selected;
}

void Unit::setSelected(bool selected)
{
	m_selected = selected;
}

void Unit::moveTo(const glm::vec3& destinationPosition)
{
	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(m_position, destinationPosition, m_pathToPosition);
}

void Unit::update(float deltaTime)
{
	if (!m_pathToPosition.empty())
	{
#ifdef RENDER_PATHING
		generateRenderPath(m_pathToPosition, m_renderPathMesh);
#endif // RENDER_PATHING

		glm::vec3 newPosition = moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_front = glm::normalize(glm::vec3(newPosition - m_position));
		m_position = newPosition;
		if (m_position == m_pathToPosition.back())
		{
			m_pathToPosition.pop_back();
		}
	}
}

void Unit::render(ShaderHandler& shaderHandler, const Model& renderModel) const
{
	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, m_position);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
	renderModel.render(shaderHandler, m_selected);
}

#ifdef RENDER_PATHING
void Unit::renderPathMesh(ShaderHandler& shaderHandler)
{
	if (!m_pathToPosition.empty())
	{
		shaderHandler.setUniformVec3(eShaderType::Ground, "uColor", PATH_COLOUR);
		m_renderPathMesh.render(shaderHandler, m_selected);
	}
}
#endif // RENDER_PATHING