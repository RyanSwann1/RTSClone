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
class EntityManager : private NonCopyable, private NonMovable
{
public:
	EntityManager(std::string fileName = std::string());

	Entity* getSelectedEntity();
	const std::vector<Entity>& getEntities() const;
	
	void addEntity(eModelName modelName, const glm::vec3& position);
	void removeAllSelectedEntities();
	bool selectEntityAtPosition(const glm::vec3& position);
	void selectEntities(const SelectionBox& selectionBox);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderEntityAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Entity> m_entities;
	int m_selectedEntityID;
};