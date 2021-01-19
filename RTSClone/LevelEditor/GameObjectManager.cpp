#include "GameObjectManager.h"
#include "ModelManager.h"
#include "Globals.h"
#include "LevelFileHandler.h"
#include "SelectionBox.h"
#include <assert.h>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

GameObjectManager::GameObjectManager()
	: m_gameObjects(),
	m_selectedGameObjectID(Globals::INVALID_ENTITY_ID)
{}

bool GameObjectManager::isGameObjectSelected() const
{
	return m_selectedGameObjectID != Globals::INVALID_ENTITY_ID;
}

GameObject* GameObjectManager::getSelectedGameObject()
{
	if(m_selectedGameObjectID != Globals::INVALID_ENTITY_ID)
	{
		auto selectedGameObject = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [this](const auto& gameObject)
		{
			return this->m_selectedGameObjectID == m_selectedGameObjectID;
		});
		
		assert(selectedGameObject != m_gameObjects.end());
		return &(*selectedGameObject);
	}

	return nullptr;
}

const std::vector<GameObject>& GameObjectManager::getGameObjects() const
{
	return m_gameObjects;
}

void GameObjectManager::addGameObject(const Model& model, const glm::vec3& position)
{
	auto gameObject = std::find_if(m_gameObjects.cbegin(), m_gameObjects.cend(), [&position](const auto& gameObject)
	{
		return gameObject.getPosition() == position;
	});
	if (gameObject == m_gameObjects.cend())
	{
		m_gameObjects.emplace_back(model, position);
	}
}

void GameObjectManager::removeAllSelectedEntities()
{
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end();)
	{
		if (gameObject->isSelected())
		{
			if (gameObject->getID() == m_selectedGameObjectID)
			{
				m_selectedGameObjectID = Globals::INVALID_ENTITY_ID;
			}

			gameObject = m_gameObjects.erase(gameObject);
		}
		else
		{
			++gameObject;
		}
	}
}

const GameObject* GameObjectManager::selectGameObjectAtPosition(const glm::vec3& position)
{
	const GameObject* selectedGameObject = nullptr;
	m_selectedGameObjectID = Globals::INVALID_ENTITY_ID;
	for (auto gameObject = m_gameObjects.begin(); gameObject != m_gameObjects.end(); ++gameObject)
	{
		if (gameObject->getAABB().contains(position))
		{
			gameObject->setSelected(true);
			m_selectedGameObjectID = gameObject->getID();
			selectedGameObject = &(*gameObject);
		}
		else
		{
			gameObject->setSelected(false);
		}
	}

	return selectedGameObject;
}

void GameObjectManager::selectGameObjects(const SelectionBox& selectionBox)
{
	int selectedGameObjectCount = 0;
	for (auto& gameObject : m_gameObjects)
	{
		gameObject.setSelected(selectionBox.getAABB().contains(gameObject.getAABB()));
		if (gameObject.isSelected())
		{
			++selectedGameObjectCount;
			m_selectedGameObjectID = gameObject.getID();
		}	
	}

	if (selectedGameObjectCount > 1)
	{
		m_selectedGameObjectID = Globals::INVALID_ENTITY_ID;
	}	
}

void GameObjectManager::render(ShaderHandler& shaderHandler) const
{
	for (const auto& gameObject : m_gameObjects)
	{
		gameObject.render(shaderHandler);
	}

	ModelManager::getInstance().getModel(TERRAIN_MODEL_NAME).render(shaderHandler, Globals::TERRAIN_POSITION);
}

#ifdef RENDER_AABB
void GameObjectManager::renderEntityAABB(ShaderHandler& shaderHandler)
{
	for (auto& gameObject : m_gameObjects)
	{
		gameObject.renderAABB(shaderHandler);	
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

		gameObjectManager.m_gameObjects.emplace_back(ModelManager::getInstance().getModel(modelName), position, rotation);
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
		ostream << gameObject.getModel().modelName << " " <<
			gameObject.getRotation().x << " " << gameObject.getRotation().y << " " << gameObject.getRotation().z << " " <<
			gameObject.getPosition().x << " " << gameObject.getPosition().y << " " << gameObject.getPosition().z << "\n";
	}
	
	return ostream;
}