#pragma once

#include "glm/glm.hpp"
#include "GameObject.h"
#include <vector>
#include <memory>
#include <ostream>

struct Model;
class ShaderHandler;
class GameObjectManager
{
public:
	GameObjectManager();
	GameObjectManager(const GameObjectManager&) = delete;
	GameObjectManager& operator=(const GameObjectManager&) = delete;
	GameObjectManager(GameObjectManager&&) = delete;
	GameObjectManager& operator=(GameObjectManager&&) = delete;

	GameObject* getGameObject(const glm::vec3& position);
	
	void addGameObject(const Model& model, const glm::vec3& position);
	void removeGameObject(const GameObject& removal);
	void render(ShaderHandler& shaderHandler, const GameObject* selectedGameObject) const;

	friend const std::ifstream& operator>>(std::ifstream& file, GameObjectManager& entityManager);
	friend std::ostream& operator<<(std::ostream& ostream, const GameObjectManager& entityManager);

#ifdef RENDER_AABB
	void renderGameObjectAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<std::unique_ptr<GameObject>> m_gameObjects;
};