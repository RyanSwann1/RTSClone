#include "GameObjectManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include "LevelFileHandler.h"
#include "SelectionBox.h"
#include <assert.h>

namespace
{
	constexpr glm::vec3 TERRAIN_STARTING_POSITION = { 0.0f, Globals::GROUND_HEIGHT - 0.01f, 0.0f };
}

GameObjectManager::GameObjectManager(std::string fileName)
	: m_gameObjects()
{
	if (!LevelFileHandler::loadLevelFromFile(fileName, m_gameObjects))
	{
		m_gameObjects.emplace_back(eModelName::Terrain, TERRAIN_STARTING_POSITION);
	}
}

const std::vector<GameObject>& GameObjectManager::getGameObjects() const
{
	return m_gameObjects;
}

void GameObjectManager::addGameObject(eModelName modelName, const glm::vec3& position)
{
	assert(Globals::isOnNodePosition(position));
	auto gameObject = std::find_if(m_gameObjects.cbegin(), m_gameObjects.cend(), [&position](const auto& gameObject)
	{
		return gameObject.position == position;
	});
	if (gameObject == m_gameObjects.cend())
	{
		m_gameObjects.emplace_back(modelName, position);
	}
}

void GameObjectManager::removeGameObject(const glm::vec3& position)
{
	if (Globals::isPositionInMapBounds(position))
	{
		for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
		{
			if (gameObject->modelName != eModelName::Terrain && 
				gameObject->AABB.contains(position))
			{
				gameObject = m_gameObjects.erase(gameObject);
			}
			else
			{
				++gameObject;
			}
		}
	}
}

void GameObjectManager::removeAllSelectedGameObjects()
{
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
	{
		if (gameObject->selected)
		{
			gameObject = m_gameObjects.erase(gameObject);
		}
		else
		{
			++gameObject;
		}
	}
}

void GameObjectManager::selectGameObjectAtPosition(const glm::vec3& position)
{
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end(); ++gameObject)
	{
		if (gameObject->AABB.contains(position))
		{
			gameObject->selected = true;
		}
		else
		{
			gameObject->selected = false;
		}
	}
}

void GameObjectManager::selectCollidingGameObjects(const SelectionBox& selectionBox)
{
	for (auto& gameObject : m_gameObjects)
	{
		if (gameObject.modelName != eModelName::Terrain)
		{
			gameObject.selected = gameObject.AABB.contains(selectionBox.AABB);
		}
	}
}

void GameObjectManager::render(ShaderHandler& shaderHandler) const
{
	for (const auto& gameObject : m_gameObjects)
	{
		gameObject.render(shaderHandler);
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