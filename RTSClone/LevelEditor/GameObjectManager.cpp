#include "GameObjectManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include "LevelFileHandler.h"
#include <assert.h>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

//GameObject
GameObject::GameObject(Model& model)
	: position(),
	rotation(),
	aabb(),
	model(model)
{
	aabb.reset(position, model);
}

GameObject::GameObject(Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: position(startingPosition),
	rotation(startingRotation),
	aabb(),
	model(model)
{
	aabb.reset(position, model);
}

void GameObject::setPosition(const glm::vec3& _position)
{
	position = _position;
	aabb.update(position);
}

void GameObject::render(ShaderHandler& shaderHandler, bool highlight) const
{
	model.get().render(shaderHandler, position, rotation);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	aabb.render(shaderHandler);
}
#endif // RENDER_AABB

//GameObjectManager
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
		ostream << gameObject->model.get().modelName << " " <<
			gameObject->rotation.x << " " << gameObject->rotation.y << " " << gameObject->rotation.z << " " <<
			gameObject->rotation.x << " " << gameObject->rotation.y << " " << gameObject->rotation.z << "\n";
	}
	
	return ostream;
}