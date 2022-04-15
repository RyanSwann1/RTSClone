#include "GameObject.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "Model.h"
#include "ModelManager.h"

GameObject::GameObject(Model& model)
	: position(0.f),
	rotation(0.f),
	scale(model.scale),
	aabb(position, model),
	model(model),
	useLocalScale(false)
{}

GameObject::GameObject(Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: position(startingPosition),
	rotation(startingRotation),
	scale(model.scale),
	aabb(position, model),
	model(model),
	useLocalScale(false)
{}

GameObject::GameObject(Model& model, const glm::vec3& startingPosition, const glm::vec3& rotation, const glm::vec3& scale, 
	float left, float right, float forward, float back, bool useLocalScale)
	: position(startingPosition),
	rotation(rotation),
	scale(scale),
	aabb(left, right, forward, back),
	model(model),
	useLocalScale(useLocalScale)
{}

void GameObject::setPosition(const glm::vec3& _position)
{
	position = _position;
	aabb.update(position);
}

void GameObject::move(const glm::vec3& _position)
{
	glm::vec3 oldPosition = position;
	position = _position;
	aabb.move(oldPosition, position);
}

void GameObject::rotate(float y)
{
	rotation.y += y;
	if (glm::abs(y) >= 360.0f)
	{
		rotation = { rotation.x, 0.0f, rotation.z };
	}
}

void GameObject::render(ShaderHandler& shaderHandler, bool highlight) const
{
	model.get().render(shaderHandler, *this, highlight);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	aabb.render(shaderHandler);
}
#endif // RENDER_AABB