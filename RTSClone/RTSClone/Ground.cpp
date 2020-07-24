#include "Ground.h"
#include "Globals.h"
#include "glad.h"
#include <array>

namespace
{
    std::array<glm::vec3, 6> getQuadCoords()
    {
        return
        {
            glm::vec3(0, 0, 0),
            glm::vec3(750, 0, 0),
            glm::vec3(750, 0, 750),
            glm::vec3(750, 0, 750),
            glm::vec3(0, 0, 750),
            glm::vec3(0, 0, 0)
        };
    };
}

Ground::Ground()
    : m_vaoID(Globals::INVALID_OPENGL_ID),
    m_vboID(Globals::INVALID_OPENGL_ID)
{
    glGenVertexArrays(1, &m_vaoID);
    glGenBuffers(1, &m_vboID);

    glBindVertexArray(m_vaoID);

    std::array<glm::vec3, 6> quadCoords = getQuadCoords();
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
    glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::vec3), quadCoords.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const void*)0);
}

Ground::~Ground()
{
    glDeleteVertexArrays(1, &m_vaoID);
    glDeleteBuffers(1, &m_vboID);
}

void Ground::render() const
{
    glBindVertexArray(m_vaoID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
