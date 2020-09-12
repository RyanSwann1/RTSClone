#include "Player.h"

Player::Player()
	: m_minerals(),
	m_HQ(eModelName::HQ, { 0.0f, 0.0f, 0.0f })
{
	for(auto& mineral : m_minerals)
	{
		mineral.setModelName(eModelName::Mineral);
		mineral.setPosition({ 0.0f, 0.0f, 0.0f });
	}
}

void Player::render(ShaderHandler& shaderHandler) const
{
	for (const auto& mineral : m_minerals)
	{
		mineral.render(shaderHandler);
	}

	m_HQ.render(shaderHandler);
}