#include "Core/AABB.h"
#include "Graphics/Model.h"
#ifdef GAME
#include "Entities/Unit.h"
#endif // GAME
#include <limits>

#include "Graphics/RenderPrimitiveMesh.h"
#include "Core/Globals.h"
#include <array>
#ifdef RENDER_AABB
namespace
{
	constexpr float OPACITY = 0.2f;
	constexpr glm::vec3 COLOR = { 0.f, 0.5f, 0.f };
}
#endif // RENDER_AABB

AABB::AABB()
	: m_left(0.0f),
	m_right(0.0f),
	m_top(0.0f),
	m_bottom(0.0f),
	m_forward(0.0f),
	m_back(0.0f)
{}

AABB::AABB(const glm::vec3& position, const glm::vec3& size)
	: m_left(0.0f),
	m_right(0.0f),
	m_top(0.0f),
	m_bottom(0.0f),
	m_forward(0.0f),
	m_back(0.0f)
{
	reset(position, size);
}

AABB::AABB(const glm::vec3& position, const Model& model)
	: m_left(0.0f),
	m_right(0.0f),
	m_top(0.0f),
	m_bottom(0.0f),
	m_forward(0.0f),
	m_back(0.0f)
{
	reset(position, model);
}

#ifdef GAME
AABB::AABB(const std::vector<Unit*>& selectedUnits)
	: m_left(std::numeric_limits<float>::max()),
	m_right(0.0f),
	m_top(1.0f),
	m_bottom(-1.0f),
	m_forward(0.0f),
	m_back(std::numeric_limits<float>::max())
{
	assert(std::find(selectedUnits.cbegin(), selectedUnits.cend(), nullptr) == selectedUnits.cend());

	for (const auto& selectedUnit : selectedUnits)
	{
		if (selectedUnit->getPosition().x < m_left)
		{
			m_left = selectedUnit->getPosition().x;
		}

		if (selectedUnit->getPosition().x > m_right)
		{
			m_right = selectedUnit->getPosition().x;
		}

		if (selectedUnit->getPosition().z > m_forward)
		{
			m_forward = selectedUnit->getPosition().z;
		}

		if (selectedUnit->getPosition().z < m_back)
		{
			m_back = selectedUnit->getPosition().z;
		}
	}
}
#endif // GAME

AABB::AABB(float left, float right, float forward, float back)
	: m_left(left),
	m_right(right),
	m_top(0.f),
	m_bottom(0.f),
	m_forward(forward),
	m_back(back)
{}

glm::vec3 AABB::getMax() const
{
	return { m_right, m_top, m_forward };
}

glm::vec3 AABB::getMin() const
{
	return { m_left, m_bottom, m_back };
}

glm::vec3 AABB::getCenterPosition() const
{
	float width = getRight() - getLeft();
	float depth = getForward() - getBack();

	return glm::vec3(m_left + (width / 2.0f), Globals::GROUND_HEIGHT, m_back + (depth / 2.0f));
}

glm::vec3 AABB::getSize() const
{
	return { m_right - m_left, m_top - m_bottom, m_forward - m_back };
}

float AABB::getLeft() const
{
	return m_left;
}
float AABB::getRight() const
{
	return m_right;
}
float AABB::getTop() const
{
	return m_top;
}
float AABB::getBottom() const
{
	return m_bottom;
}
float AABB::getForward() const
{
	return m_forward;
}
float AABB::getBack() const
{
	return m_back;
}

bool AABB::contains(const glm::vec3& position) const
{
	return position.x >= m_left &&
		position.x <= m_right &&
		position.y >= m_bottom &&
		position.y <= m_top &&
		position.z >= m_back &&
		position.z <= m_forward;
}

bool AABB::contains(const AABB& other) const
{
	return m_left < other.m_right &&
		m_right > other.m_left &&
		m_top > other.m_bottom &&
		m_bottom < other.m_top &&
		m_forward > other.m_back &&
		m_back < other.m_forward;
}

#ifdef LEVEL_EDITOR
void AABB::move(const glm::vec3& currentPosition, const glm::vec3& position)
{
	m_right = position.x + (m_right - currentPosition.x);
	m_left = position.x + (m_left - currentPosition.x);
	m_forward = position.z + (m_forward - currentPosition.z);
	m_back = position.z + (m_back - currentPosition.z);
}

void AABB::adjustRight(float size)
{
	m_right += size;
}

void AABB::adjustLeft(float size)
{
	m_left += size;
}

void AABB::adjustForward(float size)
{
	m_forward += size;
}

void AABB::adjustBack(float size)
{
	m_back += size;
}
#endif // LEVEL_EDITOR

void AABB::update(const glm::vec3& position)
{
	float x = (m_right - m_left) / 2.0f;
	float y = (m_top - m_bottom) / 2.0f;
	float z = (m_forward - m_back) / 2.0f;

	m_left = position.x - x;
	m_right = position.x + x;
	m_top = position.y + y;
	m_bottom = position.y - y;
	m_forward = position.z + z;
	m_back = position.z - z;
}

void AABB::update(const glm::vec3& position, const glm::vec3& size)
{
	reset(position, size);
}

void AABB::reset(const glm::vec3& position, const Model& model)
{
	m_left = position.x - model.AABBSizeFromCenter.x;
	m_right = position.x + model.AABBSizeFromCenter.x;
	m_top = position.y + model.AABBSizeFromCenter.y;
	m_bottom = position.y - model.AABBSizeFromCenter.y;
	m_forward = position.z + model.AABBSizeFromCenter.z;
	m_back = position.z - model.AABBSizeFromCenter.z;
}

void AABB::reset(const glm::vec3& position, const glm::vec3& size)
{
	m_left = glm::min(position.x, position.x + size.x);
	m_right = glm::max(position.x, position.x + size.x);
	m_top = glm::max(position.y, position.y + size.y);
	m_bottom = glm::min(position.y, position.y + size.y);
	m_forward = glm::max(position.z, position.z + size.z);
	m_back = glm::min(position.z, position.z + size.z);
}

void AABB::reset()
{
	m_left = 0.0f;
	m_right = 0.0f;
	m_top = 0.0f;
	m_bottom = 0.0f;
	m_forward = 0.0f;
	m_back = 0.0f;
}

#ifdef RENDER_AABB
void AABB::render(ShaderHandler& shaderHandler) 
{
	RenderPrimitiveMesh::generate(*this);
	RenderPrimitiveMesh::render(shaderHandler, *this, COLOR, OPACITY);
}
#endif // RENDER_AABB