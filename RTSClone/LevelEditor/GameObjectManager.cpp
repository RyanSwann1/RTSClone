#include "GameObjectManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include <assert.h>

GameObjectManager::GameObjectManager()
	: m_gameObjects()
{}

const std::vector<GameObject>& GameObjectManager::getGameObjects() const
{
	return m_gameObjects;
}

void GameObjectManager::addGameObject(eModelName modelName, const glm::vec3& position)
{
	AABB newGameObjectAABB(position, ModelManager::getInstance().getModel(modelName));
	if(Globals::isWithinMapBounds(newGameObjectAABB))
	{
		auto gameObject = std::find_if(m_gameObjects.cbegin(), m_gameObjects.cend(), [&newGameObjectAABB](const auto& gameObject)
		{
			return gameObject.AABB.contains(newGameObjectAABB);
		});
		if (gameObject == m_gameObjects.cend())
		{
			m_gameObjects.emplace_back(modelName, position);
		}
	}
}

void GameObjectManager::removeGameObject(const glm::vec3& position)
{
	if (Globals::isPositionInMapBounds(position))
	{
		auto gameObject = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&position](const auto& gameObject)
		{
			return gameObject.AABB.contains(position);
		});
		if (gameObject != m_gameObjects.end())
		{
			m_gameObjects.erase(gameObject);
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
		gameObject.renderAABB(shaderHandler);
	}
}
#endif // RENDER_AABB