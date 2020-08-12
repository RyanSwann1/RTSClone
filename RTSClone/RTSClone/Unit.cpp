#include "Unit.h"
#include "ShaderHandler.h"
#include "Model.h"
#include "PathFinding.h"
#include "ModelManager.h"

namespace
{
	constexpr float MOVEMENT_SPEED = 7.5f;

#ifdef RENDER_PATHING
	constexpr glm::vec3 PATH_COLOUR = { 1.0f, 0.27f, 0.0f };
	constexpr float PATH_OPACITY = 0.25f;

	void generateRenderPath(const std::vector<glm::vec3>& path, Mesh& mesh)
	{
		constexpr std::array<glm::vec3, 4> CUBE_FACE_TOP =
		{
			glm::vec3(0.0f, 0.0f, Globals::NODE_SIZE),
			glm::vec3(Globals::NODE_SIZE, 0.0f, Globals::NODE_SIZE),
			glm::vec3(Globals::NODE_SIZE, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f)
		};

		mesh.m_indices.clear();
		mesh.m_vertices.clear();

		int elementCount = 0;
		for (const auto& pathNode : path)
		{
			for (const auto& i : CUBE_FACE_TOP)
			{
				glm::vec3 position = { pathNode + i };
				position.x -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
				position.z -= static_cast<float>(Globals::NODE_SIZE) / 2.0f;
				mesh.m_vertices.emplace_back(position);
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

Unit::Unit(const glm::vec3& startingPosition, const Model& model, Map& map, eEntityType entityType)
	: Entity(startingPosition, model, entityType),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition()
{}

Unit::Unit(const glm::vec3 & startingPosition, const glm::vec3 & destinationPosition, const Model & model, Map & map, eEntityType entityType)
	: Entity(startingPosition, model, entityType),
	m_currentState(eUnitState::Idle),
	m_front(),
	m_pathToPosition()
{
	moveTo(destinationPosition, map);
}

eUnitState Unit::getCurrentState() const
{
	return m_currentState;
}

void Unit::moveTo(const glm::vec3& destinationPosition, const Map& map, const std::vector<Unit>& units,
	const GetAllAdjacentPositions& getAdjacentPositions)
{
	glm::vec3 closestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		closestDestination = m_pathToPosition.back();
	}

	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, getAdjacentPositions);
	//PathFinding::getInstance().convertPathToWaypoints(m_pathToPosition, *this, units, map);
	if (!m_pathToPosition.empty())
	{
		m_currentState = eUnitState::Moving;
	}
	else
	{
		if (closestDestination != m_position)
		{
			m_pathToPosition.push_back(closestDestination);
			m_currentState = eUnitState::Moving;
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
	}
}

void Unit::moveTo(const glm::vec3& destinationPosition, const Map& map)
{
	glm::vec3 previousClosestDestination = m_position;
	if (!m_pathToPosition.empty())
	{
		previousClosestDestination = m_pathToPosition.back();
	}

	m_pathToPosition.clear();
	PathFinding::getInstance().getPathToPosition(*this, destinationPosition, m_pathToPosition, 
		[&](const glm::ivec2& position) { return getAllAdjacentPositions(position, map); });
	if (!m_pathToPosition.empty())
	{
		m_currentState = eUnitState::Moving;
	}
	else
	{
		if (previousClosestDestination != m_position)
		{
			m_pathToPosition.push_back(previousClosestDestination);
			m_currentState = eUnitState::Moving;
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
	}
}

void Unit::update(float deltaTime, const ModelManager& modelManager)
{
	switch (m_currentState)
	{
	case eUnitState::Moving:
		if (!m_pathToPosition.empty())
		{
			glm::vec3 newPosition = Globals::moveTowards(m_position, m_pathToPosition.back(), MOVEMENT_SPEED * deltaTime);
			m_front = glm::normalize(glm::vec3(newPosition - m_position));
			m_position = newPosition;
			m_AABB.reset(m_position, modelManager.getModel(m_modelName));
			if (m_position == m_pathToPosition.back())
			{
				m_pathToPosition.pop_back();

				if (m_pathToPosition.empty())
				{
					m_currentState = eUnitState::Idle;
				}
			}
		}
		else
		{
			m_currentState = eUnitState::Idle;
		}
		break;
	}
}

#ifdef RENDER_PATHING
void Unit::renderPathMesh(ShaderHandler& shaderHandler)
{
	if (!m_pathToPosition.empty())
	{	
		shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", PATH_COLOUR);
		shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", PATH_OPACITY);
		generateRenderPath(m_pathToPosition, m_renderPathMesh);
		m_renderPathMesh.render(shaderHandler, m_selected);
	}
}
#endif // RENDER_PATHING