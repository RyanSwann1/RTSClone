#include "SelectionBox.h"
#include "Globals.h"
#include "glad.h"
#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Unit.h"
#include <assert.h>
#include <array>

namespace
{
    constexpr float MAX_RAY_DISTANCE = 500.0f;

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

    glm::vec2 getNormalizedDeviceCoords(const sf::Window& window, const glm::ivec2& mousePosition)
    {
        float x = (2.0f * mousePosition.x) / window.getSize().x - 1.0f;
        float y = 1.0f - (2.0f * mousePosition.y) / window.getSize().y;

        return glm::vec2(x, y);
    }

    glm::vec4 toEyeCoords(const glm::vec4& clipSpaceCoords, const glm::mat4& projection)
    {
        glm::vec4 rayEye = glm::inverse(projection) * clipSpaceCoords;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        return rayEye;
    }

    glm::vec3 toWorldCoords(const glm::mat4& view, const glm::vec4& eyeCoords)
    {   
        glm::vec4 position = glm::vec4(glm::inverse(view) * eyeCoords);
        glm::vec3 rayWorld = { position.x, position.y, position.z };
        return glm::normalize(rayWorld);
    }

    glm::vec3 calculateMouseRay(const glm::mat4& projection, const glm::mat4& view, const Camera& camera, const sf::Window& window, 
        const glm::ivec2& mousePosition)
    {
        glm::vec2 normalizedMouseCoords = getNormalizedDeviceCoords(window, mousePosition);
        glm::vec4 clipCoords = glm::vec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0f, 1.0f);
        glm::vec4 eyeCoords = toEyeCoords(clipCoords, projection);
        glm::vec3 worldRay = toWorldCoords(view, eyeCoords);

        return worldRay;
    }

    glm::vec3 getMouseToGroundPosition(const glm::mat4& projection, const glm::mat4& view, const Camera& camera, const sf::Window& window)
    {
        glm::ivec2 mousePosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
        glm::vec3 rayPositionFromMouse = calculateMouseRay(projection, view, camera, window, mousePosition);

        for (float i = 0; i <= MAX_RAY_DISTANCE; i += 1.0f)
        {
            glm::vec3 pos = rayPositionFromMouse * i + camera.position;
            if (pos.y <= 0)
            {
                return { pos.x, Globals::GROUND_HEIGHT, pos.z };
            }
        }

        assert(false);
    }
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

void SelectionBox::update(const glm::mat4& projection, const glm::mat4& view, const Camera& camera, const sf::Window& window,
    Unit& unit)
{
    if (m_active)
    {
        m_mouseToGroundPosition = getMouseToGroundPosition(projection, view, camera, window);
        m_selectionBox.reset(m_startingPositionWorldPosition, m_mouseToGroundPosition - m_startingPositionWorldPosition);

        unit.setSelected(m_selectionBox.contains(unit.getAABB()));
    }
}

void SelectionBox::handleInputEvents(const sf::Event& currentSFMLEvent, const sf::Window& window, const glm::mat4& projection, 
    const glm::mat4& view, const Camera& camera, Unit& unit)
{
    if (currentSFMLEvent.type == sf::Event::MouseButtonPressed)
    {
        if (currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
        {
            m_startingPositionWorldPosition = getMouseToGroundPosition(projection, view, camera, window);
            m_startingPositionScreenPosition = { sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y };
            m_active = true;
        }
        else if (currentSFMLEvent.mouseButton.button == sf::Mouse::Right && unit.isSelected())
        {
            unit.moveTo(getMouseToGroundPosition(projection, view, camera, window));
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