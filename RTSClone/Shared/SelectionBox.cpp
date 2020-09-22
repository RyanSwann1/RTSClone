#include "SelectionBox.h"
#include "glad.h"
#include "Globals.h"

namespace
{
    std::array<glm::ivec2, 6> getSelectionBoxQuadCoords(const glm::ivec2& startingPosition, const glm::ivec2& size)
    {
        return
        {
            glm::ivec2(glm::min(startingPosition.x, startingPosition.x + size.x), glm::max(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::max(startingPosition.x, startingPosition.x + size.x), glm::max(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::max(startingPosition.x, startingPosition.x + size.x), glm::min(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::max(startingPosition.x, startingPosition.x + size.x), glm::min(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::min(startingPosition.x, startingPosition.x + size.x), glm::min(startingPosition.y, startingPosition.y + size.y)),
            glm::ivec2(glm::min(startingPosition.x, startingPosition.x + size.x), glm::max(startingPosition.y, startingPosition.y + size.y))
        };
    };

    constexpr float MINIMUM_SIZE = 1.5f;
};

SelectionBox::SelectionBox()
    : m_AABB(),
    m_active(false),
    m_startingPositionScreenPosition(),
    m_startingPositionWorldPosition(),
    m_vaoID(Globals::INVALID_OPENGL_ID),
    m_vboID(Globals::INVALID_OPENGL_ID)
{
    glGenVertexArrays(1, &m_vaoID);
    glGenBuffers(1, &m_vboID);
}

SelectionBox::~SelectionBox()
{
    assert(m_vaoID != Globals::INVALID_OPENGL_ID);
    glDeleteVertexArrays(1, &m_vaoID);

    assert(m_vboID != Globals::INVALID_OPENGL_ID);
    glDeleteBuffers(1, &m_vboID);
}

const AABB& SelectionBox::getAABB() const
{
    return m_AABB;
}

bool SelectionBox::isActive() const
{
    return m_active;
}

bool SelectionBox::isMinimumSize() const
{
    return (m_AABB.getRight() - m_AABB.getLeft() >= MINIMUM_SIZE) || 
        (m_AABB.getForward() - m_AABB.getBack() >= MINIMUM_SIZE);
}

void SelectionBox::setStartingPosition(const sf::Window& window, const glm::vec3& mouseToGroundPosition)
{
    m_startingPositionWorldPosition = mouseToGroundPosition;
    m_startingPositionScreenPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
    m_active = true;
}

void SelectionBox::setSize(const glm::vec3& mouseToGroundPosition)
{
    m_AABB.reset(m_startingPositionWorldPosition, mouseToGroundPosition - m_startingPositionWorldPosition);
}

void SelectionBox::reset()
{
    m_active = false;
    m_AABB.reset();
}

void SelectionBox::render(const sf::Window& window) const
{
    if (m_active && isMinimumSize())
    {
        glm::vec2 endingPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
        std::array<glm::ivec2, 6> quadCoords = getSelectionBoxQuadCoords(m_startingPositionScreenPosition,
            endingPosition - m_startingPositionScreenPosition);

        glBindVertexArray(m_vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
        glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::ivec2), quadCoords.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(glm::ivec2), (const void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}