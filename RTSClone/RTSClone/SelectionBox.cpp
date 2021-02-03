#include "SelectionBox.h"
#include "glad.h"
#include "Globals.h"
#include "Camera.h"

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

    const float MINIMUM_SIZE = 1.5f;
};

SelectionBox::SelectionBox()
    : m_AABB(),
    m_enabled(false),
    m_screenStartingPosition(),
    m_worldStartingPosition(),
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
    return m_enabled && isMinimumSize();
}

bool SelectionBox::isMinimumSize() const
{
    return (m_AABB.getRight() - m_AABB.getLeft() >= MINIMUM_SIZE) || 
        (m_AABB.getForward() - m_AABB.getBack() >= MINIMUM_SIZE);
}

void SelectionBox::setStartingPosition(const sf::Window& window, const glm::vec3& position)
{
    m_worldStartingPosition = position;
    m_screenStartingPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
    m_enabled = true;
}

void SelectionBox::update(const Camera& camera, const sf::Window& window)
{
    if (m_enabled)
    {
        glm::vec3 position = camera.getRayToGroundPlaneIntersection(window);

        glm::vec3 startingPosition = {
        glm::min(m_worldStartingPosition.x, position.x),
        m_worldStartingPosition.y,
        glm::min(m_worldStartingPosition.z, position.z) };

        glm::vec3 size = {
            std::abs(position.x - m_worldStartingPosition.x),
            m_worldStartingPosition.y,
            std::abs(position.z - m_worldStartingPosition.z)
        };

        m_AABB.reset(startingPosition, size);
    }
}

void SelectionBox::reset()
{
    m_enabled = false;
    m_AABB.reset();
}

void SelectionBox::render(const sf::Window& window) const
{
    if (isActive())
    {
        glm::vec2 endingPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
        std::array<glm::ivec2, 6> quadCoords = getSelectionBoxQuadCoords(m_screenStartingPosition,
            endingPosition - m_screenStartingPosition);

        glBindVertexArray(m_vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
        glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::ivec2), quadCoords.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(glm::ivec2), (const void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}