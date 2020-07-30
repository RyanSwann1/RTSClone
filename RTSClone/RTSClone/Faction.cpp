#include "Faction.h"
#include "Globals.h"
#include "glad.h"
#include "Unit.h"
#include "Headquarters.h"
#include "Camera.h"
#include "Map.h"
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

//SelectionBox
SelectionBox::SelectionBox()
    : AABB(),
    active(false),
    mouseToGroundPosition(),
    startingPositionScreenPosition(),
    startingPositionWorldPosition(),
    vaoID(Globals::INVALID_OPENGL_ID),
    vboID(Globals::INVALID_OPENGL_ID)
{
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboID);
}

SelectionBox::~SelectionBox()
{
    assert(vaoID != Globals::INVALID_OPENGL_ID);
    glDeleteVertexArrays(1, &vaoID);

    assert(vboID != Globals::INVALID_OPENGL_ID);
    glDeleteBuffers(1, &vboID);
}

//Faction
Faction::Faction(const Model& headquartersModel, const Model& unitModel, Map& map)
    : m_selectionBox(),
    m_HQ({ 37.5f, Globals::GROUND_HEIGHT, 37.5f }, headquartersModel),
    m_unit({ 20.0f, Globals::GROUND_HEIGHT, 20.0f }, unitModel)
{
    map.addEntityAABB(m_HQ.getAABB());
}

void Faction::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, const Map& map)
{
    if (currentSFMLEvent.type == sf::Event::MouseButtonPressed)
    {
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_selectionBox.startingPositionWorldPosition = camera.getMouseToGroundPosition(window);
            m_selectionBox.startingPositionScreenPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
            m_selectionBox.active = true;
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right && m_unit.isSelected())
        {
            m_unit.moveTo(camera.getMouseToGroundPosition(window), map);
        }
    }
    else if (currentSFMLEvent.type == sf::Event::MouseButtonReleased)
    {
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_selectionBox.active = false;
            m_selectionBox.AABB.reset();
        }
    }

    m_HQ.handleInput(currentSFMLEvent, camera, window);
}

void Faction::update(const Camera& camera, const sf::Window& window)
{
    if (m_selectionBox.active)
    {
        m_selectionBox.mouseToGroundPosition = camera.getMouseToGroundPosition(window);
        m_selectionBox.AABB.reset(m_selectionBox.startingPositionWorldPosition, 
            m_selectionBox.mouseToGroundPosition - m_selectionBox.startingPositionWorldPosition);

        m_HQ.setSelected(m_selectionBox.AABB.contains(m_HQ.getAABB()));
        m_unit.setSelected(m_selectionBox.AABB.contains(m_unit.getAABB()));
    }
}

void Faction::update(float deltaTime)
{
    m_unit.update(deltaTime);
}

void Faction::render(ShaderHandler& shaderHandler, const Model& hqModel, const Model& unitModel, const Model& waypointModel) const
{
    m_HQ.render(shaderHandler, hqModel, waypointModel);
    m_unit.render(shaderHandler, unitModel);
}

void Faction::renderSelectionBox(const sf::Window& window) const
{
    if (m_selectionBox.active)
    {
        glm::vec2 endingPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
        std::array<glm::ivec2, 6> quadCoords = getQuadCoords(m_selectionBox.startingPositionScreenPosition, 
            endingPosition - m_selectionBox.startingPositionScreenPosition);
        
        glBindVertexArray(m_selectionBox.vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, m_selectionBox.vboID);
        glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::ivec2), quadCoords.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(glm::ivec2), (const void*)0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

#ifdef RENDER_PATHING
void Faction::renderPathing(ShaderHandler& shaderHandler)
{
    m_unit.renderPathMesh(shaderHandler);
}
#endif // RENDER_PATHING

#ifdef RENDER_AABB
void Faction::renderAABB(ShaderHandler& shaderHandler)
{
    m_unit.renderAABB(shaderHandler);
    m_HQ.renderAABB(shaderHandler);
}
#endif // RENDER_AABB
