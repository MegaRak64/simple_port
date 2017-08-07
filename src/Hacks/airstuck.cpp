#include "airstuck.h"

bool Settings::Airstuck::enabled = false;
bool Settings::Airstuck::alwaysOn = false;
int Settings::Airstuck::moveSpeed = 0;
ButtonCode_t Settings::Airstuck::key = ButtonCode_t::KEY_F;

static int stuckCount;

void Airstuck::CreateMove(CUserCmd* cmd)
{
	if (!Settings::Airstuck::enabled)
		return;

	if (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2)
		return;
	
	stuckCount++;


	if (stuckCount > Settings::Airstuck::moveSpeed && (inputSystem->IsButtonDown(Settings::Airstuck::key) || Settings::Airstuck::alwaysOn)) {
		cmd->tick_count = 16777216;
		stuckCount = 0;
	}

}
