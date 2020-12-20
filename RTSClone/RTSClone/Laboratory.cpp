#include "Laboratory.h"
#include "ModelManager.h"
#include "Globals.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "Faction.h"
#include "ShaderHandler.h"
#include "Camera.h"

namespace
{
	const float INCREASE_SHIELD_TIMER_EXPIRATION = 5.0f;
	const float PROGRESS_BAR_YOFFSET = 125.0f;
}

Laboratory::Laboratory(const glm::vec3& startingPosition, const Faction& owningFaction)
	: Entity(ModelManager::getInstance().getModel(LABORATORY_MODEL_NAME), startingPosition, eEntityType::Laboratory,
		Globals::LABORATORY_STARTING_HEALTH, owningFaction.getCurrentShieldAmount()),
	m_increaseShieldCommands(),
	m_increaseShieldTimer(INCREASE_SHIELD_TIMER_EXPIRATION, false),
	m_progressBarSprite()
{
	GameMessenger::getInstance().broadcast<GameMessages::AddToMap>({ m_AABB, getID() });
}

Laboratory::~Laboratory()
{
	GameMessenger::getInstance().broadcast<GameMessages::RemoveFromMap>({ m_AABB });
}

void Laboratory::addIncreaseShieldCommand(const std::function<void()>& command)
{
	if (m_increaseShieldCommands.empty())
	{
		m_increaseShieldTimer.resetElaspedTime();
		m_increaseShieldTimer.setActive(true);
	}
	
	m_increaseShieldCommands.push(command);
}

void Laboratory::update(float deltaTime)
{
	if (m_increaseShieldTimer.isActive())
	{
		assert(!m_increaseShieldCommands.empty());

		m_increaseShieldTimer.update(deltaTime);
		if (m_increaseShieldTimer.isExpired())
		{
			m_increaseShieldCommands.front()();
			m_increaseShieldCommands.pop();

			m_increaseShieldTimer.setActive(!m_increaseShieldCommands.empty());
			m_increaseShieldTimer.resetElaspedTime();
		}
	}
}

void Laboratory::renderProgressBar(ShaderHandler& shaderHandler, const Camera& camera, glm::uvec2 windowSize) const
{
	if (m_increaseShieldTimer.isActive())
	{
		assert(!m_increaseShieldCommands.empty());
		
		glm::vec4 positionNDC = camera.getProjection(glm::ivec2(windowSize.x, windowSize.y)) * camera.getView() * glm::vec4(m_position, 1.0f);
		positionNDC /= positionNDC.w;
		float width = 0.0f;
		float height = Globals::DEFAULT_PROGRESS_BAR_HEIGHT / windowSize.y * 2.0f;
		float yOffset = 0.0f;
		float currentTime = m_increaseShieldTimer.getElaspedTime() / m_increaseShieldTimer.getExpiredTime();
		width = Globals::LABORATORY_STAT_BAR_WIDTH * currentTime / windowSize.x * 2.0f;
		yOffset = PROGRESS_BAR_YOFFSET / windowSize.y * 2.0f;

		shaderHandler.setUniformVec3(eShaderType::HealthBar, "uMaterialColor", Globals::PROGRESS_BAR_COLOR);
		m_progressBarSprite.render(glm::vec2(positionNDC), windowSize, width, height, yOffset);
	}
}