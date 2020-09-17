#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameMessageType.h"
#include <imgui/imgui.h>

UIManager::UIManager()
{
	GameMessenger::getInstance().subscribe<GameMessages::UIDisplayPlayerDetails>([this](const GameMessages::UIDisplayPlayerDetails& gameMessage)
		{return onDisplayResourceCount(gameMessage); }, this);
}

UIManager::~UIManager()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplayPlayerDetails>(this);
}

void UIManager::onDisplayResourceCount(const GameMessages::UIDisplayPlayerDetails& gameMessage)
{
	ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoFocusOnAppearing))
	{
		ImGui::Text("Resources:");
		ImGui::Text(std::to_string(gameMessage.resourceAmount).c_str());
		ImGui::Text("Population");
		ImGui::Text(std::to_string(gameMessage.currentPopulationAmount).c_str());
		ImGui::Text("Max Population");
		ImGui::Text(std::to_string(gameMessage.maximumPopulationAmount).c_str());
	}
	ImGui::End();
}