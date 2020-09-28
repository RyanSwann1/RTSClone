#include "PlayableAreaDisplay.h"
#include "Globals.h"
#include "glad.h"
#include "ShaderHandler.h"
#include "glm/glm.hpp"
#include <array>

namespace
{
    constexpr glm::vec3 GROUND_COLOR = { 1.0f, 1.0f, 0.5f };

    std::array<glm::vec3, 6> getQuadCoords(const glm::ivec2& mapSize)
    {
        glm::vec2 size = { static_cast<float>(mapSize.x), static_cast<float>(mapSize.y) };
        return
        {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(Globals::NODE_SIZE * size.x, 0.0f, 0.0f),
            glm::vec3(Globals::NODE_SIZE * size.x, 0.0f, Globals::NODE_SIZE * size.y),
            glm::vec3(Globals::NODE_SIZE * size.x, 0.0f, Globals::NODE_SIZE * size.y),
            glm::vec3(0.0f, 0.0f, Globals::NODE_SIZE * size.y),
            glm::vec3(0.0f, 0.0f, 0.0f)
        };
    };
}

PlayableAreaDisplay::PlayableAreaDisplay(const glm::ivec2& size)
    : m_vaoID(Globals::INVALID_OPENGL_ID),
    m_vboID(Globals::INVALID_OPENGL_ID)
{
    glGenVertexArrays(1, &m_vaoID);
    glGenBuffers(1, &m_vboID);

    setSize(size);
}

PlayableAreaDisplay::~PlayableAreaDisplay()
{
    glDeleteVertexArrays(1, &m_vaoID);
    glDeleteBuffers(1, &m_vboID);
}

void PlayableAreaDisplay::setSize(const glm::ivec2& size)
{
    glBindVertexArray(m_vaoID);

    std::array<glm::vec3, 6> quadCoords = getQuadCoords(size);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::vec3), quadCoords.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
}

void PlayableAreaDisplay::render(ShaderHandler& shaderHandler) const
{
    shaderHandler.setUniformVec3(eShaderType::Debug, "uColor", GROUND_COLOR);
    shaderHandler.setUniform1f(eShaderType::Debug, "uOpacity", 0.1f);
    glBindVertexArray(m_vaoID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}