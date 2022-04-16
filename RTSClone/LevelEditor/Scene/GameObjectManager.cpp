#include "Scene/GameObjectManager.h"
#include "Graphics/ModelManager.h"
#include "Core/Globals.h"
#include "Core/LevelFileHandler.h"
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
		return gameObject->aabb.contains(position);
	});

	if (gameObject != m_gameObjects.end())
	{
		return &*(*gameObject);
	}

	return nullptr;
}

void GameObjectManager::addGameObject(Model& model, const glm::vec3& position)
{
	auto gameObject = std::find_if(m_gameObjects.cbegin(), m_gameObjects.cend(), [&position](const auto& gameObject)
	{
		return gameObject->position == position;
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
		bool highlight = selectedGameObject && gameObject.get() == &(*selectedGameObject);
		gameObject->render(shaderHandler, highlight);
	}
}

#ifdef RENDER_AABB
void GameObjectManager::renderGameObjectAABB(ShaderHandler& shaderHandler)
{
	for (const auto& gameObject : m_gameObjects)
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
		glm::vec3 rotation(0.f);
		glm::vec3 position(0.f);
		glm::vec3 scale(0.f);		
		float left = 0.f, right = 0.f, forward = 0.f, back = 0.f;
		bool useLocalScale = false;

		stream >> 
			modelName >> 
			rotation.x >> rotation.y >> rotation.z >>
			position.x >> position.y >> position.z >>
			scale.x >> scale.y >> scale.z >>
			left >> right >> forward >> back >> 
			useLocalScale;

		gameObjectManager.m_gameObjects.emplace_back(
			std::make_unique<GameObject>(
			ModelManager::getInstance().getModel(modelName), position, rotation, scale, left, right, forward, back, useLocalScale));
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
		const GameObject& go = *gameObject.get();
		ostream << 
			go.model.get().modelName << " " <<
			go.rotation.x << " " << go.rotation.y << " " << go.rotation.z << " " <<
			go.position.x << " " << go.position.y << " " << go.position.z << " " <<
			go.scale.x << " " << go.scale.y << " " << go.scale.z << " " <<
			go.aabb.getLeft() << " " << go.aabb.getRight() << " " << go.aabb.getForward() << " " << go.aabb.getBack() << " " <<
			go.useLocalScale << "\n";
	}
	
	return ostream;
}