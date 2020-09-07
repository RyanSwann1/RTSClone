#include "GameObjectManager.h"
#include <assert.h>

GameObjectManager::GameObjectManager()
	: m_gameObjects()
{}

void GameObjectManager::addGameObject(eModelName modelName, const glm::vec3& position)
{
	if (m_gameObjects.find(position) == m_gameObjects.cend())
	{
		m_gameObjects.emplace(std::piecewise_construct,
			std::forward_as_tuple(position),
			std::forward_as_tuple(modelName, position));
	}
}

void GameObjectManager::render(ShaderHandler& shaderHandler) const
{
	for (const auto& gameObject : m_gameObjects)
	{
		gameObject.second.render(shaderHandler);
	}
}