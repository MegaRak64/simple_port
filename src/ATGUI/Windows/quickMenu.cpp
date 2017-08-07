#include "quickMenu.h"

bool QuickMenu::showWindow = true;
ImVec2 QuickMenu::windowPos = ImVec2(200, 200);
ImVec2 QuickMenu::windowSize = ImVec2(260, 260);

void QuickMenu::RenderWindow()
{
	if (!QuickMenu::showWindow) {
		QuickMenu::windowPos = ImGui::GetIO().MousePos;
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(QuickMenu::windowPos.x - (QuickMenu::windowSize.x/2), QuickMenu::windowPos.y - (QuickMenu::windowSize.y/2)), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(QuickMenu::windowSize, ImGuiSetCond_Always);
	if (ImGui::Begin("QuickMenu", &QuickMenu::showWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoResize))
	{
		const char* tabs[] = {
				"Aimbot",
				"Triggerbot",
				"Visuals",
				"HvH",
				"Misc",
		};
		
		ImVec2 sizeHalf = ImVec2(ImGui::GetWindowContentRegionWidth()/2 -4, 0);
		ImVec2 sizeFull = ImVec2(ImGui::GetWindowContentRegionWidth (), 0);

		ImGui::Text("Main Window");
		ImGui::Separator();
		for (int i = 0; i < IM_ARRAYSIZE(tabs); i++) {
			if (ImGui::Button(tabs[i], (i % 2 == 0 && i == IM_ARRAYSIZE(tabs)-1)? sizeFull : sizeHalf))
				Main::page = i;
			if (i % 2 == 0 && i != IM_ARRAYSIZE(tabs)-1) {
				ImGui::SameLine();
			}
		}
		
		ImGui::Text("Windows");
		ImGui::Separator();

		ImGui::Selectable("Main", &Main::showWindow, 0, sizeHalf);
		ImGui::SameLine();

		ImGui::Selectable("Configs", &Configs::showWindow, 0, sizeHalf);

		if (ModSupport::current_mod != ModType::CSCO)
		{
			ImGui::Selectable("Skin & Model Changer", &SkinModelChanger::showWindow, 0, sizeFull);
		}

		ImGui::Selectable("Spectators", &Settings::ShowSpectators::enabled, 0, sizeHalf);
		ImGui::SameLine();

		ImGui::Selectable("Colors", &Colors::showWindow, 0, sizeHalf);

		ImGui::Selectable("Player List", &PlayerList::showWindow, 0, sizeHalf);
		ImGui::SameLine();

		ImGui::Selectable("Walk Bot", &Walk::showWindow, 0, sizeHalf);


		ImGui::End();
	}
}
