#include "SelectionBox.h"
#include "Globals.h"
#include "glad.h"
#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Unit.h"
#include "Headquarters.h"
#include <assert.h>
#include <array>

namespace
{
    std::array<glm::ivec2, 6> getQuadCoords(const glm::ivec2& startingPosition, const glm::ivec2& size)
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
};

SelectionBox::SelectionBox()
    : m_selectionBox(),
    m_active(false),
    m_mouseToGroundPosition(),
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

void SelectionBox::update(const Camera& camera, const sf::Window& window, Unit& unit, Headquarters& building)
{
    if (m_active)
    {
        m_mouseToGroundPosition = camera.getMouseToGroundPosition(window); 
        m_selectionBox.reset(m_startingPositionWorldPosition, m_mouseToGroundPosition - m_startingPositionWorldPosition);

        building.setSelected(m_selectionBox.contains(building.getAABB()));
        unit.setSelected(m_selectionBox.contains(unit.getAABB()));
    }
}

void SelectionBox::handleInputEvents(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Unit& unit, const Map& map)
{
    if (currentSFMLEvent.type == sf::Event::MouseButtonPressed)
    {
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_startingPositionWorldPosition = camera.getMouseToGroundPosition(window);
            m_startingPositionScreenPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
            m_active = true;
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right && unit.isSelected())
        {
            unit.moveTo(camera.getMouseToGroundPosition(window), map);
        }
    }
    else if (currentSFMLEvent.type == sf::Event::MouseButtonReleased)
    {
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_active = false;
            m_selectionBox.reset();
        }
    }
}

void SelectionBox::render(const sf::Window& window) const
{
    if (m_active)
    {
        glm::vec2 endingPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
        std::array<glm::ivec2, 6> quadCoords = getQuadCoords(m_startingPositionScreenPosition, endingPosition - m_startingPositionScreenPosition);
        
        glBindVertexArray(m_vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
        glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::ivec2), quadCoords.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(glm::ivec2), (const void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}