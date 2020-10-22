#include "AABB.h"
#include "Model.h"
#ifdef GAME
#include "Unit.h"
#endif // GAME
#include <limits>

#ifdef RENDER_AABB
#include "ShaderHandler.h"
#include "Globals.h"
#include <array>
namespace
{
	constexpr glm::vec3 COLOR = { 1.0f, 0.0f, 0.0f };
	constexpr float OPACITY = 0.2f;
	void generateMesh(float left, float right, float back, float forward, Mesh& mesh)
	{
		mesh.m_vertices.clear();
		mesh.m_indices.clear();

		const std::array<glm::vec3, 4> CUBE_FACE_TOP =
		{
			glm::vec3(left, Globals::GROUND_HEIGHT, back),
			glm::vec3(right, Globals::GROUND_HEIGHT, back),
			glm::vec3(right, Globals::GROUND_HEIGHT, forward),
			glm::vec3(left, Globals::GROUND_HEIGHT, forward)
		};

		for (const auto& i : CUBE_FACE_TOP)
		{
			mesh.m_vertices.emplace_back(i);
		}

		for (unsigned int i : Globals::CUBE_FACE_INDICIES)
		{
			mesh.m_indices.push_back(i);
		}

		mesh.attachToVAO();
	}
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
	generateMesh(m_left, m_right, m_back, m_forward, m_mesh);

	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", COLOR);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", OPACITY);
	m_mesh.render(shaderHandler, false);
}
#endif // RENDER_AABB