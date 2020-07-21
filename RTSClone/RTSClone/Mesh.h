#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "glm/glm.hpp"
#include <vector>

struct Vertex
{
	Vertex()
		: position(),
		textCoords()
	{}

	bool operator==(const Vertex& other) const {
		return position == other.position && textCoords == other.textCoords;
	}

	glm::vec3 position;
	glm::vec2 textCoords;
};

struct Mesh : private NonMovable, private NonCopyable
{
	Mesh();
	~Mesh();

	void bind() const;
	void attachToVAO();
	void render() const;

	unsigned int m_vaoID;
	unsigned int m_vboID;
	unsigned int m_indicesID;
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
};