#include "MiniMap.h"

namespace
{
	const glm::vec3 BACKGROUND_COLOR = { 0.0f, 0.0f, 0.0f };
}

MiniMap::MiniMap(glm::ivec2 position, glm::ivec2 size)
	: m_background(),
	m_position(position),
	m_size(size)
{}

void MiniMap::render(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const
{
	m_background.render(m_position, m_size, BACKGROUND_COLOR, shaderHandler, windowSize);
}
