#pragma once

#include "glm/glm.hpp"
#include "AABB.h"
#include "ActiveStatus.h"
#include <functional>

struct Model;
class ShaderHandler;
struct SceneryGameObject
{
	SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation, 
		const glm::vec3& scale, float left, float right, float forward, float back);

	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

	std::reference_wrapper<const Model> model;
	AABB AABB;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

//struct Model;
//class ShaderHandler;
//class SceneryGameObject
//{
//public:
//	SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation, 
//		const glm::vec3& scale, float left, float right, float forward, float back);
//	SceneryGameObject(const SceneryGameObject&) = delete;
//	SceneryGameObject& operator=(const SceneryGameObject&) = delete;
//	SceneryGameObject(SceneryGameObject&&) = default;
//	SceneryGameObject& operator=(SceneryGameObject&&) = default;
//	~SceneryGameObject();
//
//	const glm::vec3& getScale() const;
//	const glm::vec3& getRotation() const;
//	const glm::vec3& getPosition() const;
//	const AABB& getAABB() const;
//	void render(ShaderHandler& shaderHandler) const;
//
//#ifdef RENDER_AABB
//	void renderAABB(ShaderHandler& shaderHandler);
//#endif // RENDER_AABB
//
//private:
//	std::reference_wrapper<const Model> m_model;
//	AABB m_AABB;
//	glm::vec3 m_position;
//	glm::vec3 m_rotation;
//	glm::vec3 m_scale;
//	ActiveStatus m_active;
//};