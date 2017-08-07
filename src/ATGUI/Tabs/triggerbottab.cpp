#include "triggerbottab.h"

void Triggerbot::RenderTab()
{
	ImGui::Checkbox("Enabled", &Settings::Triggerbot::enabled);
	ImGui::Separator();
	ImGui::Columns(2, NULL, true);
	{
		ImGui::BeginChild("TRIG1", ImVec2(0, 0), true);
		{
			ImGui::Text("Keybind");
			ImGui::Separator();
			ImGui::Columns(2, NULL, true);
			{
				ImGui::ItemSize(ImVec2(0.0f, 0.0f), 0.0f);
				ImGui::Text("Trigger Key");
			}
			ImGui::NextColumn();
			{
				UI::KeyBindButton(&Settings::Triggerbot::key);
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Text("Random Delay");
			ImGui::Separator();
			ImGui::Columns(2, NULL, true);
			{
				ImGui::Checkbox("Enabled", &Settings::Triggerbot::RandomDelay::enabled);
				SetTooltip("Adds a random delay(ms) to the Triggerbot");
				if( Settings::Triggerbot::RandomDelay::lastRoll != 0 )
				{
					ImGui::Text("Last delay: %dms", Settings::Triggerbot::RandomDelay::lastRoll);
				}
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Minimum ms");
				ImGui::SliderInt("##TRIGGERRANDOMLOW", &Settings::Triggerbot::RandomDelay::lowBound, 5, 220);
				if( Settings::Triggerbot::RandomDelay::lowBound >= Settings::Triggerbot::RandomDelay::highBound )
				{
					Settings::Triggerbot::RandomDelay::highBound = Settings::Triggerbot::RandomDelay::lowBound + 1;
				}
				ImGui::Text("Maximum ms");
				ImGui::SliderInt("##TRIGGERRANDOMHIGH", &Settings::Triggerbot::RandomDelay::highBound, (Settings::Triggerbot::RandomDelay::lowBound+1), 225);
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::Text("Auto Knife & Zeus");
			ImGui::Separator();
			ImGui::Columns(2, NULL, true);
			{
					ImGui::PushItemWidth(-1);
					ImGui::Checkbox("Auto Knife", &Settings::AutoKnife::enabled);
					SetTooltip("Enable automatic Knife attack if enemy in reach");
					ImGui::Checkbox("On Key", &Settings::AutoKnife::onKey);
					SetTooltip("Attack if Triggerbot key is pressed");

			}
			ImGui::NextColumn();
			{
					ImGui::PushItemWidth(-1);
					ImGui::Checkbox("Enemies", &Settings::AutoKnife::Filters::enemies);
					SetTooltip("Attack enemies");
					ImGui::Checkbox("Allies", &Settings::AutoKnife::Filters::allies);
					SetTooltip("Attack allies");
			}
			ImGui::EndChild();
		}
	}
	ImGui::NextColumn();
	{
		ImGui::BeginChild("TRIG2", ImVec2(0, 0), true);
		{
			ImGui::Text("Minimum Inaccuracy");
			ImGui::Separator();
			ImGui::Columns(2, NULL, true);
			{
				ImGui::Checkbox("Enabled", &Settings::Triggerbot::Inaccuracy::enabled);
				SetTooltip("Adds option to ignore the Triggerbot above certain inaccuracy");
			}
			ImGui::NextColumn();
			{
				ImGui::Text("Minimum Value");
				ImGui::SliderFloat("##TRIGGERMININACC", &Settings::Triggerbot::Inaccuracy::minInaccuracy, 0, 1);
			}
			ImGui::Columns(1);
			ImGui::Text("Filter");
			ImGui::Separator();
			ImGui::Columns(2, NULL, true);
			{
				ImGui::Checkbox("Enemies", &Settings::Triggerbot::Filters::enemies);
				SetTooltip("Trigger on enemies");
				ImGui::Checkbox("Walls", &Settings::Triggerbot::Filters::walls);
				SetTooltip("Trigger through walls");
				ImGui::Checkbox("Head", &Settings::Triggerbot::Filters::head);
				SetTooltip("Trigger on head");
				ImGui::Checkbox("Chest", &Settings::Triggerbot::Filters::chest);
				SetTooltip("Trigger on chest");
				ImGui::Checkbox("Legs", &Settings::Triggerbot::Filters::legs);
				SetTooltip("Trigger on legs");
			}
			ImGui::NextColumn();
			{
				ImGui::Checkbox("Allies", &Settings::Triggerbot::Filters::allies);
				SetTooltip("Trigger on allies");
				ImGui::Checkbox("Smoke check", &Settings::Triggerbot::Filters::smokeCheck);
				SetTooltip("Don't shoot through smokes");
				ImGui::Checkbox("Flash check", &Settings::Triggerbot::Filters::flashCheck);
				SetTooltip("Don't shoot while flashed");
				ImGui::Checkbox("Stomach", &Settings::Triggerbot::Filters::stomach);
				SetTooltip("Trigger on stomach");
				ImGui::Checkbox("Arms", &Settings::Triggerbot::Filters::arms);
				SetTooltip("Trigger on arms");
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();
		}
	}
}
