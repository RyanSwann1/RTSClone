#include "UI/EntitySelectorBox.h"
#include "glad/glad.h"
#include "Core/Globals.h"
#include "Core/Camera.h"
#include "Graphics/ShaderHandler.h"

namespace
{
    const float MINIMUM_SIZE = 1.5f;
    const glm::vec3 COLOR{ 0.2f, 0.8f, 0.2f };
    const float OPACITY = 0.3f;

    std::array<glm::vec2, 6> getSelectionBoxQuadCoords(glm::vec2 position, glm::vec2 size)
    {
        return
        {
            glm::vec2(glm::min(position.x, position.x + size.x), glm::min(position.y, position.y + size.y)),
            glm::vec2(glm::max(position.x, position.x + size.x), glm::min(position.y, position.y + size.y)),
            glm::vec2(glm::max(position.x, position.x + size.x), glm::max(position.y, position.y + size.y)),
            glm::vec2(glm::max(position.x, position.x + size.x), glm::max(position.y, position.y + size.y)),
            glm::vec2(glm::min(position.x, position.x + size.x), glm::max(position.y, position.y + size.y)),
            glm::vec2(glm::min(position.x, position.x + size.x), glm::min(position.y, position.y + size.y))
        };
    };

    glm::vec2 convertMousePositionToNDC(const sf::Window& window)
    {
        return
        {
            2.0f * sf::Mouse::getPosition(window).x / window.getSize().x - 1.0f,
            (2.0f * sf::Mouse::getPosition(window).y / window.getSize().y - 1.0f) * -1.0f
        };
    }
};

const AABB& EntitySelectorBox::getAABB() const
{
    return m_AABB;
}

bool EntitySelectorBox::isActive() const
{
    return m_enabled && isMinimumSize();
}

bool EntitySelectorBox::isMinimumSize() const
{
    return (m_AABB.getRight() - m_AABB.getLeft() >= MINIMUM_SIZE) || 
        (m_AABB.getForward() - m_AABB.getBack() >= MINIMUM_SIZE);
}

void EntitySelectorBox::setStartingPosition(const sf::Window& window, const glm::vec3& position)
{
    m_worldStartingPosition = position;
    m_startingMousePosition = convertMousePositionToNDC(window);
    m_enabled = true;
}

void EntitySelectorBox::update(const Camera& camera, const sf::Window& window)
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

void EntitySelectorBox::reset()
{
    m_enabled = false;
    m_AABB.reset();
}

void EntitySelectorBox::render(const sf::Window& window, ShaderHandler& shaderHandler) const
{
    if (isActive())
    {
        std::array<glm::vec2, 6> quadCoords = getSelectionBoxQuadCoords(m_startingMousePosition,
            convertMousePositionToNDC(window) - m_startingMousePosition);

        shaderHandler.setUniformVec3(eShaderType::Widjet, "uColor", COLOR);
        shaderHandler.setUniform1f(eShaderType::Widjet, "uOpacity", OPACITY);

        m_VBO.bind();
        glBufferData(GL_ARRAY_BUFFER, quadCoords.size() * sizeof(glm::vec2), quadCoords.data(), GL_STATIC_DRAW);

        m_VAO.bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, glm::vec2::length(), GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const void*)0);

        glDrawArrays(GL_TRIANGLES, 0, quadCoords.size());
    }
}