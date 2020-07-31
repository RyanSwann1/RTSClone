#include "Faction.h"
#include "Globals.h"
#include "glad.h"
#include "Unit.h"
#include "Headquarters.h"
#include "Camera.h"
#include "Map.h"
#include "ModelManager.h"
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
Faction::Faction(const ModelManager& modelManager, Map& map)
    : m_selectionBox(),
    m_HQ({ 37.5f, Globals::GROUND_HEIGHT, 37.5f }, modelManager.getModel(eModelName::HQ), map),
    m_unit({ 20.0f, Globals::GROUND_HEIGHT, 20.0f }, modelManager.getModel(eModelName::Unit), map),
    m_harvesters()
{}

void Faction::handleInput(const sf::Event& currentSFMLEvent, const sf::Window& window, const Camera& camera, Map& map, 
    const ModelManager& modelManager, const Entity& mineral)
{
    switch (currentSFMLEvent.type)
    {
    case sf::Event::MouseButtonPressed:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            m_selectionBox.startingPositionWorldPosition = mouseToGroundPosition;
            m_selectionBox.startingPositionScreenPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
            m_selectionBox.active = true;

            m_HQ.setSelected(m_HQ.getAABB().contains(mouseToGroundPosition));
            m_unit.setSelected(m_unit.getAABB().contains(mouseToGroundPosition));
            for (auto& harvester : m_harvesters)
            {
                harvester.setSelected(harvester.getAABB().contains(mouseToGroundPosition));
            }
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right)
        {
            glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            if (m_unit.isSelected())
            {
                m_unit.moveTo(mouseToGroundPosition, map);
            }
            else if (m_HQ.isSelected())
            {
                m_HQ.setWaypointPosition(mouseToGroundPosition);
            }

            for (auto& harvester : m_harvesters)
            {
                if (harvester.isSelected())
                {
                    harvester.moveTo(mouseToGroundPosition, map, mineral);
                }
            }
        }
        break;
    case sf::Event::MouseButtonReleased:
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_selectionBox.active = false;
            m_selectionBox.AABB.reset();
        }
        break;
    case sf::Event::MouseMoved:
        if (m_selectionBox.active)
        {
            m_selectionBox.mouseToGroundPosition = camera.getMouseToGroundPosition(window);
            m_selectionBox.AABB.reset(m_selectionBox.startingPositionWorldPosition,
                m_selectionBox.mouseToGroundPosition - m_selectionBox.startingPositionWorldPosition);

            m_unit.setSelected(m_selectionBox.AABB.contains(m_unit.getAABB()));

            for (auto& harvester : m_harvesters)
            {
                harvester.setSelected(m_selectionBox.AABB.contains(harvester.getAABB()));
            }
        }
        break;
    case sf::Event::KeyPressed:
        switch (currentSFMLEvent.key.code)
        {
        case sf::Keyboard::Num1:
            if (m_HQ.isSelected())
            {
                spawnUnit(m_HQ.getUnitSpawnPosition(), modelManager.getModel(eModelName::Harvester), map);
            }
            break;
        }
        break;
    }
}

void Faction::update(float deltaTime, const ModelManager& modelManager)
{
    m_unit.update(deltaTime, modelManager);
    for (auto& harvester : m_harvesters)
    {
        harvester.update(deltaTime, modelManager);
    }
}

void Faction::render(ShaderHandler& shaderHandler, const ModelManager& modelManager) const
{
    m_HQ.render(shaderHandler, modelManager.getModel(m_HQ.getModelName()), modelManager.getModel(eModelName::Waypoint));
    m_unit.render(shaderHandler, modelManager.getModel(m_unit.getModelName()));
    for (auto& harvester : m_harvesters)
    {
        harvester.render(shaderHandler, modelManager.getModel(harvester.getModelName()));
    }
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
    for (auto& harvester : m_harvesters)
    {
        harvester.renderPathMesh(shaderHandler);
    }
}
#endif // RENDER_PATHING

#ifdef RENDER_AABB
void Faction::renderAABB(ShaderHandler& shaderHandler)
{
    m_unit.renderAABB(shaderHandler);
    for (auto& harvester : m_harvesters)
    {
        harvester.renderAABB(shaderHandler);
    }
    m_HQ.renderAABB(shaderHandler);
}
#endif // RENDER_AABB

void Faction::spawnUnit(const glm::vec3& spawnPosition, const Model& unitModel, Map& map)
{
    if (m_HQ.getWaypointPosition() != m_HQ.getPosition())
    {
        m_harvesters.emplace_back(spawnPosition, m_HQ.getWaypointPosition(), unitModel, map);
    }
    else
    {
        m_harvesters.emplace_back(spawnPosition, unitModel, map);
    }
}