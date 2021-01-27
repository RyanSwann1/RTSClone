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
	Y,
	Z
};

struct Model;
class ShaderHandler;
class TranslateObject : private NonCopyable, private NonMovable
{
public:
	TranslateObject();

	eAxisCollision getCurrentAxisSelected() const;
	bool isSelected() const;
	const glm::vec3& getPosition() const;
	
	void select(const glm::vec3& position);
	void deselect();
	void setPosition(const glm::vec3& position);
	void render(ShaderHandler& shaderHandler, bool active) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler, bool active);
#endif // RENDER_AABB

private:
	const Model& m_model;
	eAxisCollision m_currentAxisSelected;
	bool m_selected;
	glm::vec3 m_position;
	AABB m_xAABB;
	AABB m_yAABB;
	AABB m_zAABB;
};