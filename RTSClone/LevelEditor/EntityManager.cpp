#include "EntityManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include "LevelFileHandler.h"
#include "SelectionBox.h"
#include <assert.h>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

EntityManager::EntityManager()
	: m_entities(),
	m_selectedEntityID(Globals::INVALID_ENTITY_ID)
{}

bool EntityManager::isEntitySelected() const
{
	return m_selectedEntityID != Globals::INVALID_ENTITY_ID;
}

Entity* EntityManager::getSelectedEntity()
{
	if(m_selectedEntityID != Globals::INVALID_ENTITY_ID)
	{
		int selectedEntityID = m_selectedEntityID;
		auto selectedEntity = std::find_if(m_entities.begin(), m_entities.end(), [selectedEntityID](const auto& entity)
		{
			return entity.getID() == selectedEntityID;
		});
		
		assert(selectedEntity != m_entities.end());
		return &(*selectedEntity);
	}

	return nullptr;
}

const std::vector<Entity>& EntityManager::getEntities() const
{
	return m_entities;
}

void EntityManager::addEntity(eModelName modelName, const glm::vec3& position)
{
	auto entity = std::find_if(m_entities.cbegin(), m_entities.cend(), [&position](const auto& gameObject)
	{
		return gameObject.getPosition() == position;
	});
	if (entity == m_entities.cend())
	{
		m_entities.emplace_back(modelName, position);
	}
}

void EntityManager::removeAllSelectedEntities()
{
	for (auto entity = m_entities.begin(); entity != m_entities.end();)
	{
		if (entity->isSelected())
		{
			if (entity->getID() == m_selectedEntityID)
			{
				m_selectedEntityID = Globals::INVALID_ENTITY_ID;
			}

			entity = m_entities.erase(entity);
		}
		else
		{
			++entity;
		}
	}
}

const Entity* EntityManager::selectEntityAtPosition(const glm::vec3& position)
{
	const Entity* selectedEntity = nullptr;
	m_selectedEntityID = Globals::INVALID_ENTITY_ID;
	for (auto entity = m_entities.begin(); entity != m_entities.end(); ++entity)
	{
		if (entity->getAABB().contains(position))
		{
			entity->setSelected(true);
			m_selectedEntityID = entity->getID();
			selectedEntity = &(*entity);
		}
		else
		{
			entity->setSelected(false);
		}
	}

	return selectedEntity;
}

void EntityManager::selectEntities(const SelectionBox& selectionBox)
{
	int selectedEntityCount = 0;
	for (auto& entity : m_entities)
	{
		entity.setSelected(selectionBox.getAABB().contains(entity.getAABB()));
		if (entity.isSelected())
		{
			++selectedEntityCount;
			m_selectedEntityID = entity.getID();
		}	
	}

	if (selectedEntityCount > 1)
	{
		m_selectedEntityID = Globals::INVALID_ENTITY_ID;
	}	
}

void EntityManager::render(ShaderHandler& shaderHandler) const
{
	for (const auto& entity : m_entities)
	{
		entity.render(shaderHandler);
	}

	ModelManager::getInstance().getModel(eModelName::Terrain).render(shaderHandler, Globals::TERRAIN_POSITION);
}

#ifdef RENDER_AABB
void EntityManager::renderEntityAABB(ShaderHandler& shaderHandler)
{
	for (auto& entity : m_entities)
	{
		entity.renderAABB(shaderHandler);	
	}
}
#endif // RENDER_AABB

const std::ifstream& operator>>(std::ifstream& file, EntityManager& entityManager)
{
	auto data = [&entityManager](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string rawModelName;
		glm::vec3 position;
		stream >> rawModelName >> position.x >> position.y >> position.z;

		entityManager.m_entities.emplace_back(static_cast<eModelName>(std::stoi(rawModelName)), position);
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_SCENERY;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return file;
}

std::ostream& operator<<(std::ostream& ostream, const EntityManager& entityManager)
{
	ostream << Globals::TEXT_HEADER_SCENERY << "\n";
	for (const auto& entity : entityManager.m_entities)
	{
		ostream << static_cast<int>(entity.getModelName()) << " " <<
			entity.getPosition().x << " " << entity.getPosition().y << " " << entity.getPosition().z << "\n";
	}
	
	return ostream;
}