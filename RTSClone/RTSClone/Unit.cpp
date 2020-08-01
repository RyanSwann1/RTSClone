#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "PathFinding.h"
#include "ModelManager.h"

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
	constexpr float PATH_OPACITY = 0.25f;

	void generateRenderPath(const std::vector<glm::vec3>& path, Mesh& mesh)
	{
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

			for (unsigned int i : Globals::CUBE_FACE_INDICIES)
			{
				mesh.m_indices.push_back(i + elementCount);
			}

			elementCount += Globals::CUBE_FACE_INDICIE_COUNT;
		}

		mesh.attachToVAO();
	};

#endif // RENDER_PATHING
}

Unit::Unit(const glm::vec3& startingPosition, const Model& model, Map& map)
	: Entity(startingPosition, model, eEntityType::Unit, map),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition()
{}

Unit::Unit(const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Model & model, Map & map)
	: Entity(startingPosition, model, eEntityType::Unit, map),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition()
{
	moveTo(destinationPosition, map);
}

void Unit::moveTo(const glm::vec3& destinationPosition, const Map& map)
{
	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(m_position, destinationPosition, m_pathToPosition, map);
}

void Unit::update(float deltaTime, const ModelManager& modelManager)
{
	if (!m_pathToPosition.empty())
	{
		m_currentState = eUnitState::Moving;
#ifdef RENDER_PATHING
		generateRenderPath(m_pathToPosition, m_renderPathMesh);
#endif // RENDER_PATHING

		glm::vec3 newPosition = moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
		m_front = glm::normalize(glm::vec3(newPosition - m_position));
		m_position = newPosition;
		m_AABB.resetFromCentre(m_position, modelManager.getModel(m_modelName).sizeFromCentre);
		if (m_position == m_pathToPosition.back())
		{
			m_pathToPosition.pop_back();
			
			if (m_pathToPosition.empty())
			{
				m_currentState = eUnitState::Idle;
			}
		}
	}
}

#ifdef RENDER_PATHING
void Unit::renderPathMesh(ShaderHandler& shaderHandler)
{
	if (!m_pathToPosition.empty())
	{
		shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", PATH_COLOUR);
		shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", PATH_OPACITY);
		m_renderPathMesh.render(shaderHandler, m_selected);
	}
}
#endif // RENDER_PATHING