#pragma once

#include "AABB.h"
#include <vector>
#include <memory>
#include <ostream>
#include <functional>

class ShaderHandler;
struct Model;
struct GameObject
{
	GameObject(Model& model);
	GameObject(Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation = glm::vec3(0.0f));
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = default;
	GameObject& operator=(GameObject&&) = default;

	void setPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler, bool highlight = false) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	glm::vec3 position;
	glm::vec3 rotation;
	AABB aabb;
	std::reference_wrapper<Model> model;
};

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
	
	void addGameObject(Model& model, const glm::vec3& position);
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