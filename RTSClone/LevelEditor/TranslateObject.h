#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include "AABB.h"
#include <glm/glm.hpp>

enum class eAxisCollision
{
	None = 0,
	X,
	Z
};

class ShaderHandler;
class TranslateObject : private NonCopyable, private NonMovable
{
public:
	TranslateObject();

	eAxisCollision getCurrentAxisSelected() const;
	bool isSelected() const;
	const glm::vec3& getCenterPosition() const;
	
	void setSelected(bool selected, const glm::vec3& position);
	void deselect();
	void setActive(bool active);
	void setPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	eAxisCollision m_currentAxisSelected;
	bool m_selected;
	bool m_active;
	glm::vec3 m_centerPosition;
	glm::vec3 m_xPosition;
	AABB m_xAABB;
	glm::vec3 m_zPosition;
	AABB m_zAABB;
};