#include "main.h"

bool Main::showWindow = true;

int Main::page = 0;

void Main::RenderWindow()
{
	if (!Main::showWindow)
		return;


	ImGui::SetNextWindowSize(ImVec2(960, 645), ImGuiSetCond_FirstUseEver);
	if (ImGui::Begin("AyyTux", &Main::showWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
	{
		switch (page)
		{
			case 0:
				Aimbot::RenderTab();
				break;
			case 1:
				Triggerbot::RenderTab();
				break;
			case 2:
				Visuals::RenderTab();
				break;
			case 3:
				HvH::RenderTab();
				break;
			case 4:
				Misc::RenderTab();
				break;
		}
		ImGui::End();
	}
}
