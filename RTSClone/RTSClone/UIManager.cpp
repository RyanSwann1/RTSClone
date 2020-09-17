#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameMessageType.h"
#include <imgui/imgui.h>

UIManager::UIManager()
{
	GameMessenger::getInstance().subscribe<GameMessages::UIDisplayResourceCount>([this](const GameMessages::UIDisplayResourceCount& gameMessage)
		{return onDisplayResourceCount(gameMessage); }, this);
}

UIManager::~UIManager()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplayResourceCount>(this);
}

void UIManager::onDisplayResourceCount(const GameMessages::UIDisplayResourceCount& gameMessage)
{
	ImGui::SetNextWindowSize(ImVec2(175, 175), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::Text("Resources:");
		ImGui::Text(std::to_string(gameMessage.resourceAmount).c_str());
	}
	ImGui::End();
}