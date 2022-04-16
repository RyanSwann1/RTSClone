#include "Scene/SceneryGameObject.h"
#include "Events/GameMessenger.h"
#include "Events/GameMessages.h"
#include "Graphics/ModelManager.h"
#include "Core/Globals.h"
#include "Graphics/Model.h"

SceneryGameObject::SceneryGameObject(const Model& model, const glm::vec3& position, const glm::vec3& rotation,
	const glm::vec3& scale, float left, float right, float forward, float back)
	: model(model),
	AABB(left, right, forward, back),
	position(position),
	rotation(rotation),
	scale(scale)
{}

void SceneryGameObject::render(ShaderHandler& shaderHandler) const
{
	model.get().render(shaderHandler, *this);
}

#ifdef RENDER_AABB
void SceneryGameObject::renderAABB(ShaderHandler& shaderHandler)
{
	AABB AABB(m_position, m_model);
	AABB.render(shaderHandler);
}
#endif // RENDER_AABB