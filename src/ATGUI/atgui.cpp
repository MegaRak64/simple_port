#include "atgui.h"

bool UI::isVisible = false;
bool Settings::Watermark::enabled = true;
char* Settings::Watermark::text = strdup("AyyTux");
ColorVar Settings::Watermark::color = ImColor(255, 255, 255, 255);

bool Settings::ScreenshotCleaner::enabled = false;

ColorVar Settings::UI::mainColor = ImColor(0, 218, 70, 156);
ColorVar Settings::UI::bodyColor = ImColor(5, 24, 32, 238);
ColorVar Settings::UI::fontColor = ImColor(0, 254, 219, 255);

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

void SetupMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8 * 2.0f, 4 * 2.0f));

		ImGui::Selectable("Main Window", &Main::showWindow, 0, ImVec2(ImGui::CalcTextSize("Main Window", NULL, true).x, 0.0f));
		ImGui::SameLine();

		if (ModSupport::current_mod != ModType::CSCO)
		{
			ImGui::Selectable("Skin & Model Changer Window", &SkinModelChanger::showWindow, 0, ImVec2(ImGui::CalcTextSize("Skin & Model Changer Window", NULL, true).x, 0.0f));
			ImGui::SameLine();
		}

		ImGui::Selectable("Config Window", &Configs::showWindow, 0, ImVec2(ImGui::CalcTextSize("Config Window", NULL, true).x, 0.0f));
		ImGui::SameLine();

		ImGui::Selectable("Spectators Window", &Settings::ShowSpectators::enabled, 0, ImVec2(ImGui::CalcTextSize("Spectators Window", NULL, true).x, 0.0f));
		ImGui::SameLine();

		ImGui::Selectable("Colors Window", &Colors::showWindow, 0, ImVec2(ImGui::CalcTextSize("Colors Window", NULL, true).x, 0.0f));
		ImGui::SameLine();

		ImGui::Selectable("Player List Window", &PlayerList::showWindow, 0, ImVec2(ImGui::CalcTextSize("Player List Window", NULL, true).x, 0.0f));
		ImGui::SameLine();

		ImGui::Selectable("Walk Bot Window", &Walk::showWindow, 0, ImVec2(ImGui::CalcTextSize("Walk Bot Window", NULL, true).x, 0.0f));

		ImGui::PopStyleVar();
		ImGui::EndMainMenuBar();
	}
}

void UI::QuickToggle()
{
	QuickMenu::showWindow = !QuickMenu::showWindow;
}

void UI::SwapWindow()
{
	if (UI::isVisible) {
		Draw::ImDrawText(ImVec2(4.f, 4.f), ImColor(255, 255, 255, 255), "Right click for the quick menu", NULL, 0.0f, NULL, ImFontFlags_Shadow);
		return;
	}

	if (engine->IsInGame())
		return;

	if (Settings::Watermark::enabled)
		Draw::ImDrawText(ImVec2(4.f, 4.f), Settings::Watermark::color.Color(), Settings::Watermark::text, NULL, 0.0f, NULL, ImFontFlags_Outline);
}

void UI::SetVisible(bool visible)
{
	UI::isVisible = visible;
	cvar->FindVar("cl_mouseenable")->SetValue(!UI::isVisible);
}

void UI::SetupWindows()
{
	if (UI::isVisible)
	{
		//SetupMainMenuBar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(960, 645));
			Main::RenderWindow();
		ImGui::PopStyleVar();

		if (ModSupport::current_mod != ModType::CSCO)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1050, 645));
				SkinModelChanger::RenderWindow();
			ImGui::PopStyleVar();
		}

		Configs::RenderWindow();
		Colors::RenderWindow();
		PlayerList::RenderWindow();
		Walk::RenderWindow();
		QuickMenu::RenderWindow();
	}

	ShowSpectators::RenderWindow();
	Radar::RenderWindow();
}
