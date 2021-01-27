#pragma once

#include "glm/glm.hpp"
#include "NonMovable.h"
#include "GameObject.h"
#include <vector>
#include <memory>
#include <ostream>

struct Model;
class ShaderHandler;
class GameObjectManager : private NonCopyable, private NonMovable
{
public:
	GameObjectManager();

	GameObject* getGameObject(const glm::vec3& position);
	const std::vector<GameObject>& getGameObjects() const;
	
	void addGameObject(const Model& model, const glm::vec3& position);
	void removeGameObject(const GameObject& removal);
	void render(ShaderHandler& shaderHandler) const;

	friend const std::ifstream& operator>>(std::ifstream& file, GameObjectManager& entityManager);
	friend std::ostream& operator<<(std::ostream& ostream, const GameObjectManager& entityManager);

#ifdef RENDER_AABB
	void renderGameObjectAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<GameObject> m_gameObjects;
};