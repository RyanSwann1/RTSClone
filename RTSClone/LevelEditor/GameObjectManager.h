#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "NonMovable.h"
#include "GameObject.h"
#include <vector>
#include <unordered_map>

class ShaderHandler;
enum class eModelName;
class GameObjectManager : private NonCopyable, private NonMovable
{
public:
	GameObjectManager();

	void addGameObject(eModelName modelName, const glm::vec3& position);

	void render(ShaderHandler& shaderHandler) const;

private:
	std::unordered_map<glm::vec3, GameObject> m_gameObjects;
};