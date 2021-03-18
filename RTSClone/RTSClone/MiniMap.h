#pragma once

#include "Sprite.h"

class ShaderHandler;
class MiniMap
{
public:
	MiniMap(glm::ivec2 position, glm::ivec2 size);
	MiniMap(const MiniMap&) = delete;
	MiniMap& operator=(const MiniMap&) = delete;
	MiniMap(MiniMap&&) = delete;
	MiniMap& operator=(MiniMap&&) = delete;

	void render(ShaderHandler& shaderHandler, glm::uvec2 windowSize) const;

private:
	const Sprite m_background;
	const glm::ivec2 m_position;
	const glm::ivec2 m_size;
};