#pragma once

#include "glm/glm.hpp"
#include "NonMovable.h"
#include "Entity.h"
#include <vector>
#include <memory>
#include <ostream>

class SelectionBox;
class ShaderHandler;
enum class eModelName;
class EntityManager : private NonCopyable, private NonMovable
{
public:
	EntityManager();

	Entity* getSelectedEntity();
	const std::vector<Entity>& getEntities() const;
	
	void addEntity(eModelName modelName, const glm::vec3& position);
	void removeAllSelectedEntities();
	bool selectEntityAtPosition(const glm::vec3& position);
	void selectEntities(const SelectionBox& selectionBox);
	void render(ShaderHandler& shaderHandler) const;

	friend const std::ifstream& operator>>(std::ifstream& file, EntityManager& entityManager);
	friend std::ostream& operator<<(std::ostream& ostream, const EntityManager& entityManager);

#ifdef RENDER_AABB
	void renderEntityAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Entity> m_entities;
	int m_selectedEntityID;
};