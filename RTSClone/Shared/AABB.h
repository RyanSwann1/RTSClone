#pragma once

#include "glm/glm.hpp"
#include <vector>

#ifdef RENDER_AABB
#include "Mesh.h"
#endif // RENDER_AABB

//Min/Max

struct Model;
class ShaderHandler;
#ifdef GAME
class Unit;
#endif // GAME
class AABB
{
public:
	AABB();
	AABB(const glm::vec3& position, const glm::vec3& size);
	AABB(const glm::vec3& position, const Model& model);
#ifdef GAME
	AABB(const std::vector<Unit*>& selectedUnits);
#endif // GAME
	
	glm::vec3 getMax() const;
	glm::vec3 getMin() const;
	glm::vec3 getCenterPosition() const;
	float getLeft() const;
	float getRight() const;
	float getTop() const;
	float getBottom() const;
	float getForward() const;
	float getBack() const;

	bool contains(const glm::vec3& position) const;
	bool contains(const AABB& other) const;
	
	void update(const glm::vec3& position);
	void update(const glm::vec3& position, const glm::vec3& size);
	void reset(const glm::vec3& position, const Model& model);
	void reset(const glm::vec3& position, const glm::vec3& size);
	void reset();

#ifdef RENDER_AABB
	void render(ShaderHandler& shaderHandler);
	Mesh m_mesh;
#endif // RENDER_AABB

private:
	float m_left;
	float m_right;
	float m_top;
	float m_bottom;
	float m_forward;
	float m_back;
};