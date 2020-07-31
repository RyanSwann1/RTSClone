#pragma once

#include "glm/glm.hpp"

#ifdef RENDER_AABB
#include "Mesh.h"
#endif // RENDER_AABB

struct Model;
class ShaderHandler;
struct AABB
{
	AABB();
	AABB(const glm::vec3& position, const Model& model);
	AABB(const glm::vec3& position, const glm::vec3& size);
	AABB(const glm::vec3& position, float distance);

	bool contains(const glm::vec3& position) const;
	bool contains(const AABB& other) const;

	void resetFromCentre(const glm::vec3& position, const glm::vec3& size);
	void reset(const glm::vec3& position, const glm::vec3& size);
	void reset(const glm::vec3& position, float distance);
	void reset();

#ifdef RENDER_AABB
	void render(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	float m_left;
	float m_right;
	float m_top;
	float m_bottom;
	float m_forward;
	float m_back;

#ifdef RENDER_AABB
	Mesh m_mesh;
#endif // RENDER_AABB
};