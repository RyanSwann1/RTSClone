#pragma once

#include "glm/glm.hpp"
#include "NonMovable.h"
#include "Entity.h"
#include <vector>
#include <memory>
#include <ostream>

struct Model;
class SelectionBox;
class ShaderHandler;
class EntityManager : private NonCopyable, private NonMovable
{
public:
	EntityManager();

	bool isEntitySelected() const;
	Entity* getSelectedEntity();
	const std::vector<std::unique_ptr<Entity>>& getEntities() const;
	
	void addEntity(const Model& model, const glm::vec3& position);
	void removeAllSelectedEntities();
	const Entity* selectEntityAtPosition(const glm::vec3& position);
	void selectEntities(const SelectionBox& selectionBox);
	void render(ShaderHandler& shaderHandler) const;

	friend const std::ifstream& operator>>(std::ifstream& file, EntityManager& entityManager);
	friend std::ostream& operator<<(std::ostream& ostream, const EntityManager& entityManager);

#ifdef RENDER_AABB
	void renderEntityAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<std::unique_ptr<Entity>> m_entities;
	int m_selectedEntityID;
};