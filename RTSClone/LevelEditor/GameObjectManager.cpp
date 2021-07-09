#include "GameObjectManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include "LevelFileHandler.h"
#include <assert.h>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

GameObjectManager::GameObjectManager()
	: m_gameObjects()
{}

GameObject* GameObjectManager::getGameObject(const glm::vec3 & position)
{
	auto gameObject = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&position](const auto& gameObject)
	{
		return gameObject->getAABB().contains(position);
	});

	if (gameObject != m_gameObjects.end())
	{
		return &*(*gameObject);
	}

	return nullptr;
}

void GameObjectManager::addGameObject(const Model& model, const glm::vec3& position)
{
	auto gameObject = std::find_if(m_gameObjects.cbegin(), m_gameObjects.cend(), [&position](const auto& gameObject)
	{
		return gameObject->getPosition() == position;
	});
	if (gameObject == m_gameObjects.cend())
	{
		m_gameObjects.emplace_back(std::make_unique<GameObject>(model, position));
	}
}

void GameObjectManager::removeGameObject(const GameObject& removal)
{
	auto gameObject = std::find_if(m_gameObjects.cbegin(), m_gameObjects.cend(), [&removal](const auto& gameObject)
	{
		return &*gameObject == &removal;
	});

	assert(gameObject != m_gameObjects.cend());
	m_gameObjects.erase(gameObject);
}

void GameObjectManager::render(ShaderHandler& shaderHandler, const GameObject* selectedGameObject) const
{
	for (const auto& gameObject : m_gameObjects)
	{
		if (selectedGameObject && &*gameObject == &(*selectedGameObject))
		{
			gameObject->render(shaderHandler, true);
		}
		else
		{
			gameObject->render(shaderHandler);
		}
	}
}

#ifdef RENDER_AABB
void GameObjectManager::renderGameObjectAABB(ShaderHandler& shaderHandler)
{
	for (auto& gameObject : m_gameObjects)
	{
		gameObject->renderAABB(shaderHandler);	
	}
}
#endif // RENDER_AABB

const std::ifstream& operator>>(std::ifstream& file, GameObjectManager& gameObjectManager)
{
	auto data = [&gameObjectManager](const std::string& line)
	{
		std::stringstream stream{ line };
		std::string modelName;
		glm::vec3 rotation;
		glm::vec3 position;
		stream >> modelName >> rotation.x >> rotation.y >> rotation.z >> position.x >> position.y >> position.z;

		gameObjectManager.m_gameObjects.emplace_back(std::make_unique<GameObject>(ModelManager::getInstance().getModel(modelName), position, rotation));
	};

	auto conditional = [](const std::string& line)
	{
		return line == Globals::TEXT_HEADER_SCENERY;
	};

	LevelFileHandler::loadFromFile(file, data, conditional);

	return file;
}

std::ostream& operator<<(std::ostream& ostream, const GameObjectManager& gameObjectManager)
{
	ostream << Globals::TEXT_HEADER_SCENERY << "\n";
	for (const auto& gameObject : gameObjectManager.m_gameObjects)
	{
		ostream << gameObject->getModel().modelName << " " <<
			gameObject->getRotation().x << " " << gameObject->getRotation().y << " " << gameObject->getRotation().z << " " <<
			gameObject->getPosition().x << " " << gameObject->getPosition().y << " " << gameObject->getPosition().z << "\n";
	}
	
	return ostream;
}