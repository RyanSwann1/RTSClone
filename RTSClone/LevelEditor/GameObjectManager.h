#pragma once

#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameObject.h"
#include <vector>
#include <string>
#include <memory>

class ShaderHandler;
enum class eModelName;
class GameObjectManager : private NonCopyable, private NonMovable
{
public:
	static std::unique_ptr<GameObjectManager> create(std::string levelName = std::string());
	
	const std::vector<GameObject>& getGameObjects() const;

	void addGameObject(eModelName modelName, const glm::vec3& position);
	void removeGameObject(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderGameObjectAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	GameObjectManager();
	GameObjectManager(std::vector<GameObject>&& gameObjectsFromFile);
	std::vector<GameObject> m_gameObjects;
};