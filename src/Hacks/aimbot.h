#pragma once

#include "../settings.h"
#include "../SDK/SDK.h"
#include "../interfaces.h"
#include "../Utils/entity.h"
#include "../Utils/math.h"

extern "C"
{
	#include <xdo.h>
}

namespace Aimbot
{
	extern bool aimStepInProgress;
	extern std::vector<int64_t> friends;
    extern int targetAimbot;

	bool HitChance(const Vector& vecPoint, bool teamCheck, C_BasePlayer* localplayer);
	void RCS(QAngle& angle, C_BasePlayer* player, CUserCmd* cmd);
	void AimStep(C_BasePlayer* player, QAngle& angle, CUserCmd* cmd);
	void Smooth(C_BasePlayer* player, QAngle& angle, CUserCmd* cmd);
	void AutoCrouch(C_BasePlayer* player, CUserCmd* cmd);
	void AutoSlow(C_BasePlayer* player, float& forward, float& sideMove, float& bestDamage, C_BaseCombatWeapon* active_weapon, CUserCmd* cmd);
	void AutoPistol(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd);
	void AutoShoot(C_BasePlayer* player, int bone, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd);
	float AutoWallBestBone(C_BasePlayer *player, int &bestBone);
	void ShootCheck(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd);
	void NoShoot(C_BaseCombatWeapon* activeWeapon, C_BasePlayer* player, C_BasePlayer* localplayer, CUserCmd* cmd);
	void AutoCockRevolver( C_BaseCombatWeapon* activeWeapon, C_BasePlayer* localplayer, CUserCmd* cmd);

	void CreateMove(CUserCmd* cmd);
	void FireGameEvent(IGameEvent* event);
	void UpdateValues();

	void XDOCleanup();
}
