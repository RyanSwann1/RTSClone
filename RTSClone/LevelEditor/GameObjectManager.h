#pragma once

#include "glm/glm.hpp"
#include "NonMovable.h"
#include "Entity.h"
#include <vector>
#include <string>
#include <memory>

struct SelectionBox;
class ShaderHandler;
enum class eModelName;
class GameObjectManager : private NonCopyable, private NonMovable
{
public:
	GameObjectManager(std::string fileName = std::string());

	const std::vector<Entity>& getEntities() const;

	void addGameObject(eModelName modelName, const glm::vec3& position);
	void removeAllSelectedGameObjects();
	void selectGameObjectAtPosition(const glm::vec3& position);
	void selectCollidingGameObjects(const SelectionBox& selectionBox);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderGameObjectAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Entity> m_entities;
};