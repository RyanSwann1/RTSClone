#include "GameObjectManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include "LevelFileHandler.h"
#include "SelectionBox.h"
#include <assert.h>
#include <imgui/imgui.h>

namespace
{
	constexpr glm::vec3 TERRAIN_STARTING_POSITION = { 0.0f, Globals::GROUND_HEIGHT - 0.01f, 0.0f };
}

GameObjectManager::GameObjectManager(std::string fileName)
	: m_entities()
{
	if (!LevelFileHandler::loadLevelFromFile(fileName, m_entities))
	{
		m_entities.emplace_back(eModelName::Terrain, TERRAIN_STARTING_POSITION);
	}
}

const std::vector<Entity>& GameObjectManager::getEntities() const
{
	return m_entities;
}

void GameObjectManager::addGameObject(eModelName modelName, const glm::vec3& position)
{
	assert(Globals::isOnNodePosition(position));
	auto gameObject = std::find_if(m_entities.cbegin(), m_entities.cend(), [&position](const auto& gameObject)
	{
		return gameObject.getPosition() == position;
	});
	if (gameObject == m_entities.cend())
	{
		m_entities.emplace_back(modelName, position);
	}
}

void GameObjectManager::removeAllSelectedGameObjects()
{
	for (auto entity = m_entities.begin(); entity != m_entities.end();)
	{
		if (entity->isSelected())
		{
			entity = m_entities.erase(entity);
		}
		else
		{
			++entity;
		}
	}
}

void GameObjectManager::selectGameObjectAtPosition(const glm::vec3& position)
{
	for (auto entity = m_entities.begin(); entity != m_entities.end(); ++entity)
	{
		entity->setSelected(entity->getAABB().contains(position));
	}
}

void GameObjectManager::selectCollidingGameObjects(const SelectionBox& selectionBox)
{
	for (auto& entity : m_entities)
	{
		if (entity.getModelName() != eModelName::Terrain)
		{
			entity.setSelected(selectionBox.AABB.contains(entity.getAABB()));
		}
	}
}

void GameObjectManager::render(ShaderHandler& shaderHandler) const
{
	for (const auto& entity : m_entities)
	{
		entity.render(shaderHandler);
	}
}

#ifdef RENDER_AABB
void GameObjectManager::renderGameObjectAABB(ShaderHandler& shaderHandler)
{
	for (auto& gameObject : m_gameObjects)
	{
		if (gameObject.modelName != eModelName::Terrain)
		{
			gameObject.renderAABB(shaderHandler);
		}
	}
}
#endif // RENDER_AABB