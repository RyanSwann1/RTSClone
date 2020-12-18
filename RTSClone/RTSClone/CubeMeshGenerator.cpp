#include "CubeMeshGenerator.h"
#include "Mesh.h"
#include <array>

namespace
{
	constexpr unsigned int CUBE_FACE_INDICIE_COUNT = 4;
	constexpr float CUBE_FACE_SIZE = 10.0f;
	constexpr std::array<unsigned int, 6> CUBE_FACE_INDICIES =
	{
		0, 1, 2,
		2, 3, 0
	};
	enum class eCubeFace
	{
		Left,
		Right,
		Front,
		Back,
		Top,
		Bottom
	};

	constexpr std::array<glm::vec3, 4> CUBE_FACE_FRONT =
	{
		glm::vec3(0, 0, CUBE_FACE_SIZE),
		glm::vec3(CUBE_FACE_SIZE, 0, CUBE_FACE_SIZE),
		glm::vec3(CUBE_FACE_SIZE, CUBE_FACE_SIZE, CUBE_FACE_SIZE),
		glm::vec3(0, CUBE_FACE_SIZE, CUBE_FACE_SIZE)
	};
	constexpr std::array<glm::vec3, 4> CUBE_FACE_BACK =
	{
		glm::vec3(CUBE_FACE_SIZE, 0, 0),
		glm::vec3(0, 0, 0),
		glm::vec3(0, CUBE_FACE_SIZE, 0),
		glm::vec3(CUBE_FACE_SIZE, CUBE_FACE_SIZE, 0)
	};
	constexpr std::array<glm::vec3, 4> CUBE_FACE_LEFT =
	{
		glm::vec3(0, 0, 0),
		glm::vec3(0, 0, CUBE_FACE_SIZE),
		glm::vec3(0, CUBE_FACE_SIZE, CUBE_FACE_SIZE),
		glm::vec3(0, CUBE_FACE_SIZE, 0)
	};
	constexpr std::array<glm::vec3, 4> CUBE_FACE_RIGHT =
	{
		glm::vec3(CUBE_FACE_SIZE, 0, CUBE_FACE_SIZE),
		glm::vec3(CUBE_FACE_SIZE, 0, 0),
		glm::vec3(CUBE_FACE_SIZE, CUBE_FACE_SIZE, 0),
		glm::vec3(CUBE_FACE_SIZE, CUBE_FACE_SIZE, CUBE_FACE_SIZE)
	};
	constexpr std::array<glm::vec3, 4> CUBE_FACE_TOP =
	{
		glm::vec3(0, CUBE_FACE_SIZE, CUBE_FACE_SIZE),
		glm::vec3(CUBE_FACE_SIZE, CUBE_FACE_SIZE, CUBE_FACE_SIZE),
		glm::vec3(CUBE_FACE_SIZE, CUBE_FACE_SIZE, 0),
		glm::vec3(0, CUBE_FACE_SIZE, 0)
	};
	constexpr std::array<glm::vec3, 4> CUBE_FACE_BOTTOM =
	{
		glm::vec3(0, 0, 0),
		glm::vec3(CUBE_FACE_SIZE, 0, 0),
		glm::vec3(CUBE_FACE_SIZE, 0, CUBE_FACE_SIZE),
		glm::vec3(0, 0, CUBE_FACE_SIZE)
	};
}

void generateCubeFace(eCubeFace cubeFace, Mesh& mesh, unsigned int& elementBufferIndex, const glm::vec3& position);

void CubeMeshGenerator::generateCubeMesh(Mesh& mesh, glm::vec3 position)
{
	unsigned int elementBufferIndex = 0;
	generateCubeFace(eCubeFace::Left, mesh, elementBufferIndex, position);
	generateCubeFace(eCubeFace::Right, mesh, elementBufferIndex, position);
	generateCubeFace(eCubeFace::Top, mesh, elementBufferIndex, position);
	generateCubeFace(eCubeFace::Bottom, mesh, elementBufferIndex, position);
	generateCubeFace(eCubeFace::Back, mesh, elementBufferIndex, position);
	generateCubeFace(eCubeFace::Front, mesh, elementBufferIndex, position);

	mesh.attachToVAO();
}

void generateCubeFace(eCubeFace cubeFace, Mesh& mesh, unsigned int& elementBufferIndex, const glm::vec3& position)
{
	glm::vec3 centeredPosition =
	{ position.x - CUBE_FACE_SIZE / 2.0f, position.y - CUBE_FACE_SIZE / 2.0f, position.z - CUBE_FACE_SIZE / 2.0f };

	switch (cubeFace)
	{
	case eCubeFace::Left:
	{
		for (const auto& i : CUBE_FACE_LEFT)
		{
			mesh.vertices.emplace_back(centeredPosition + i);
		}
	}
	break;
	case eCubeFace::Right:
	{
		for (const auto& i : CUBE_FACE_RIGHT)
		{
			mesh.vertices.emplace_back(centeredPosition + i);
		}
	}
	break;
	case eCubeFace::Top:
	{
		for (const auto& i : CUBE_FACE_TOP)
		{
			mesh.vertices.emplace_back(centeredPosition + i);
		}
	}
	break;
	case eCubeFace::Bottom:
	{
		for (const auto& i : CUBE_FACE_BOTTOM)
		{
			mesh.vertices.emplace_back(centeredPosition + i);
		}
	}
	break;
	case eCubeFace::Front:
	{
		for (const auto& i : CUBE_FACE_FRONT)
		{
			mesh.vertices.emplace_back(centeredPosition + i);
		}
	}
	break;
	case eCubeFace::Back:
	{
		for (const auto& i : CUBE_FACE_BACK)
		{
			mesh.vertices.emplace_back(centeredPosition + i);
		}
	}
	break;
	}

	for (unsigned int i : CUBE_FACE_INDICIES)
	{
		mesh.indices.emplace_back(i + elementBufferIndex);
	}

	elementBufferIndex += CUBE_FACE_INDICIE_COUNT;
}