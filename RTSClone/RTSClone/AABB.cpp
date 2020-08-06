#include "AABB.h"
#include "Model.h"
#include "Unit.h"
#include <limits>

#ifdef RENDER_AABB
#include "ShaderHandler.h"
#include "Globals.h"
#include <array>
namespace
{
	constexpr glm::vec3 COLOR = { 1.0f, 0.0f, 0.0f };
	constexpr float OPACITY = 0.4f;
	constexpr float HEIGHT = 0.0f;
	void generateMesh(const glm::vec3& position, const glm::vec3& size, Mesh& mesh)
	{
		mesh.m_vertices.clear();
		mesh.m_indices.clear();

		const std::array<glm::vec3, 4> CUBE_FACE_TOP =
		{
			glm::vec3(position.x - size.x, HEIGHT, position.z + size.z),
			glm::vec3(position.x + size.x, HEIGHT, position.z + size.z),
			glm::vec3(position.x + size.x, HEIGHT, position.z - size.z),
			glm::vec3(position.x - size.x, HEIGHT, position.z - size.z)
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

AABB::AABB(const glm::vec3 & position, const Model & model)
	: m_left(position.x - model.sizeFromCentre.x),
	m_right(position.x + model.sizeFromCentre.x),
	m_top(position.y + model.sizeFromCentre.y),
	m_bottom(position.y - model.sizeFromCentre.y),
	m_forward(position.z + model.sizeFromCentre.z),
	m_back(position.z - model.sizeFromCentre.z)
{}

AABB::AABB(const glm::vec3& position, const glm::vec3& size)
	: m_left(glm::min(position.x, position.x + size.x)),
	m_right(glm::max(position.x, position.x + size.x)),
	m_top(glm::max(position.y, position.y + size.y)),
	m_bottom(glm::min(position.y, position.y + size.y)),
	m_forward(glm::max(position.z, position.z + size.z)),
	m_back(glm::min(position.z, position.z + size.z))
{}

AABB::AABB(const glm::vec3& position, float distance)
	: m_left(position.x - distance),
	m_right(position.x + distance),
	m_top(position.y + distance),
	m_bottom(position.y - distance),
	m_forward(position.z + distance),
	m_back(position.z - distance)
{ }

AABB::AABB(const std::vector<const Unit*>& selectedUnits)
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
	}

	for (const auto& selectedUnit : selectedUnits)
	{
		if (selectedUnit->getPosition().x > m_right)
		{
			m_right = selectedUnit->getPosition().x;
		}
	}

	for (const auto& selectedUnit : selectedUnits)
	{
		if (selectedUnit->getPosition().z > m_forward)
		{
			m_forward = selectedUnit->getPosition().z;
		}
	}

	for (const auto& selectedUnit : selectedUnits)
	{
		if (selectedUnit->getPosition().z < m_back)
		{
			m_back = selectedUnit->getPosition().z;
		}
	}
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
	return m_left <= other.m_right &&
		m_right >= other.m_left &&
		m_top >= other.m_bottom &&
		m_bottom <= other.m_top &&
		m_forward >= other.m_back &&
		m_back <= other.m_forward;
}

void AABB::resetFromCentre(const glm::vec3& position, const glm::vec3& size)
{
	m_left = position.x - size.x;
	m_right = position.x + size.x;
	m_top = position.y + size.y;
	m_bottom = position.y - size.y;
	m_forward = position.z + size.z;
	m_back = position.z - size.z;
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

void AABB::reset(const glm::vec3& position, float distance)
{
	m_left = position.x - distance;
	m_right = position.x + distance;
	m_top = position.y + distance;
	m_bottom = position.y - distance;
	m_forward = position.z + distance;
	m_back = position.z - distance;
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
	glm::vec3 centrePosition((m_right - (m_right - m_left) / 2.0f), (m_top - (m_top - m_bottom) / 2.0f), (m_forward - (m_forward - m_back) / 2.0f));
	glm::vec3 distanceFromCentre(m_right - centrePosition.x, m_top - centrePosition.y, m_forward - centrePosition.z);

	generateMesh(centrePosition, distanceFromCentre, m_mesh);

	shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", COLOR);
	shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", OPACITY);
	m_mesh.render(shaderHandler, false);
}
#endif // RENDER_AABB