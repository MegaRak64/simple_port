#include "aimbot.h"
#include "autowall.h"

// Default aimbot settings
bool Settings::Aimbot::enabled = false;
bool Settings::Aimbot::silent = false;
bool Settings::Aimbot::pSilent = false;
bool Settings::Aimbot::friendly = false;
Bone Settings::Aimbot::bone = Bone::BONE_HEAD;
ButtonCode_t Settings::Aimbot::aimkey = ButtonCode_t::MOUSE_MIDDLE;
bool Settings::Aimbot::aimkeyOnly = false;
bool Settings::Aimbot::Smooth::enabled = false;
float Settings::Aimbot::Smooth::value = 0.5f;
SmoothType Settings::Aimbot::Smooth::type = SmoothType::SLOW_END;
bool Settings::Aimbot::ErrorMargin::enabled = false;
float Settings::Aimbot::ErrorMargin::value = 0.0f;
bool Settings::Aimbot::Curve::enabled = false;
float Settings::Aimbot::Curve::value = 0.5f;
bool Settings::Aimbot::AutoAim::enabled = false;
float Settings::Aimbot::AutoAim::fov = 180.0f;
bool Settings::Aimbot::AutoAim::realDistance = false;
bool Settings::Aimbot::AutoAim::closestBone = false;
bool Settings::Aimbot::AutoAim::desiredBones[] = {true, true, true, true, true, true, true, // center mass
							  false, false, false, false, false, false, false, // left arm
							  false, false, false, false, false, false, false, // right arm
							  false, false, false, false, false, // left leg
							  false, false, false, false, false  // right leg
};
bool Settings::Aimbot::AutoAim::engageLock = false;
bool Settings::Aimbot::AutoAim::engageLockTR = false; // engage lock Target Reacquisition ( re-target after getting a kill when spraying ).
int Settings::Aimbot::AutoAim::engageLockTTR = 700; // Time to Target Reacquisition in ms
bool Settings::Aimbot::SpreadLimit::enabled = false;
bool Settings::Aimbot::SpreadLimit::distanceBased = false;
float Settings::Aimbot::SpreadLimit::value = 0.1f;
bool Settings::Aimbot::AutoWall::enabled = false;
float Settings::Aimbot::AutoWall::value = 10.0f;
bool Settings::Aimbot::AutoWall::bones[] = { true, false, false, false, false, false };
bool Settings::Aimbot::AimStep::enabled = false;
float Settings::Aimbot::AimStep::value = 25.0f;
bool Settings::Aimbot::AutoPistol::enabled = false;
bool Settings::Aimbot::AutoShoot::enabled = false;
bool Settings::Aimbot::AutoShoot::velocityCheck = false;
bool Settings::Aimbot::AutoShoot::autoscope = false;
bool Settings::Aimbot::RCS::enabled = false;
bool Settings::Aimbot::RCS::always_on = false;
float Settings::Aimbot::RCS::valueX = 2.0f;
float Settings::Aimbot::RCS::valueY = 2.0f;
bool Settings::Aimbot::AutoCrouch::enabled = false;
bool Settings::Aimbot::NoShoot::enabled = false;
bool Settings::Aimbot::IgnoreJump::enabled = false;
bool Settings::Aimbot::SmokeCheck::enabled = false;
bool Settings::Aimbot::FlashCheck::enabled = false;
bool Settings::Aimbot::Smooth::Salting::enabled = false;
float Settings::Aimbot::Smooth::Salting::multiplier = 0.0f;
bool Settings::Aimbot::AutoSlow::enabled = false;
bool Settings::Aimbot::AutoSlow::goingToSlow = false;
bool Settings::Aimbot::Prediction::enabled = false;
bool Settings::Aimbot::AutoCockRevolver::enabled = false;
bool Settings::Aimbot::HitChance::enabled = false;
int Settings::Aimbot::HitChance::hitRays = 100;
float Settings::Aimbot::HitChance::value = 0.5f;

bool Aimbot::aimStepInProgress = false;
std::vector<int64_t> Aimbot::friends = { };
std::vector<long> killTimes = { 0 }; // the Epoch time from when we kill someone
float autoCockDifference = 0.0f;
bool Settings::Aimbot::moveMouse = false;

bool shouldAim;
QAngle AimStepLastAngle;
QAngle RCSLastPunch;


static xdo_t *xdo = xdo_new(NULL);

int Aimbot::targetAimbot = -1;

std::unordered_map<Hitbox, std::vector<const char*>, Util::IntHash<Hitbox>> hitboxes = {
		{ Hitbox::HITBOX_HEAD, { "head_0" } },
		{ Hitbox::HITBOX_NECK, { "neck_0" } },
		{ Hitbox::HITBOX_PELVIS, { "pelvis" } },
		{ Hitbox::HITBOX_SPINE, { "spine_0", "spine_1", "spine_2", "spine_3" } },
		{ Hitbox::HITBOX_LEGS, { "leg_upper_L", "leg_upper_R", "leg_lower_L", "leg_lower_R", "ankle_L", "ankle_R" } },
		{ Hitbox::HITBOX_ARMS, { "hand_L", "hand_R", "arm_upper_L", "arm_lower_L", "arm_upper_R", "arm_lower_R" } },
};

std::unordered_map<ItemDefinitionIndex, AimbotWeapon_t, Util::IntHash<ItemDefinitionIndex>> Settings::Aimbot::weapons = {
		{ ItemDefinitionIndex::INVALID, { false, false, false, false, false, false, false, 700, Bone::BONE_HEAD, ButtonCode_t::MOUSE_MIDDLE, false, false, 1.0f, SmoothType::SLOW_END, false, 0.0f, false, 0.0f, false, 0.5f, true, 180.0f, false, 25.0f, false, false, 2.0f, 2.0f, false, false, false, false, false, false, false, false, false, false, 0.1f, false, 10.0f, false, false, 5.0f, false, false, 100, 0.5f } },
};

static QAngle ApplyErrorToAngle(QAngle* angles, float margin)
{
	QAngle error;
	error.Random(-1.0f, 1.0f);
	error *= margin;
	angles->operator+=(error);
	return error;
}
static inline void ApplyOffsetToAngle(QAngle *angles, QAngle *offset)
{
	angles->operator+=(*offset);
}

void Aimbot::XDOCleanup()
{
	xdo_free(xdo);
}

bool Aimbot::HitChance(const Vector& point, bool teamCheck, C_BasePlayer* localplayer)
{
	if (!Settings::Aimbot::HitChance::enabled)
		return true;
	
	int hitCount = 0;
	for (int i = 0; i < Settings::Aimbot::HitChance::hitRays; i++) {
		Vector dst = point;
	
		C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
		if (!activeWeapon)
			return false;

		float a = (float)M_PI * 2.0f * ((float)(rand() % 1000)/1000.0f);
		float b = activeWeapon->GetSpread() * ((float)(rand() % 1000)/1000.0f) * 90.0f;
		float c = (float)M_PI * 2.0f * ((float)(rand() % 1000)/1000.0f);
		float d = activeWeapon->GetInaccuracy() * ((float)(rand() % 1000)/1000.0f) * 90.0f;

		Vector dir, src, dest;
    trace_t tr;
    Ray_t ray;
    CTraceFilterEntitiesOnly filter;

    src = localplayer->GetEyePosition();
    QAngle angles = Math::CalcAngle(src, dst);
    angles.x += (cos(a) * b) + (cos(c) * d);
  	angles.y += (sin(a) * b) + (sin(c) * d);
    Math::AngleVectors(angles, dir);
    dest = src + (dir * 8192);
		
		ray.Init(src, dest);
    filter.pSkip = localplayer;
		trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	
		C_BasePlayer* player = (C_BasePlayer*) tr.m_pEntityHit;
    if (player && player->GetClientClass()->m_ClassID == EClassIds::CCSPlayer && player != localplayer && !player->GetDormant() && player->GetAlive() && !player->GetImmune() && (player->GetTeam() != localplayer->GetTeam() || Settings::Aimbot::friendly))	
			hitCount++;
	}

	//cvar->ConsoleDPrintf("HitCount: %d/%d - %f\n", hitCount, Settings::Aimbot::HitChance::hitRays, Settings::Aimbot::HitChance::value);


	return ((float)hitCount/(float)Settings::Aimbot::HitChance::hitRays > Settings::Aimbot::HitChance::value);
}

float AutoWallBestBone(C_BasePlayer *player, int &bestBone)
{
	float bestDamage = Settings::Aimbot::AutoWall::value;
	const std::map<int, int> *modelType = Util::GetModelTypeBoneMap(player);

	static int len = sizeof(Settings::Aimbot::AutoAim::desiredBones) / sizeof(Settings::Aimbot::AutoAim::desiredBones[0]);

	for( int i = 0; i < len; i++ )
	{
		if( !Settings::Aimbot::AutoAim::desiredBones[i] )
			continue;
		int boneID = (*modelType).at(i);
		if( boneID == (int)Bone::INVALID ) // bone not available on this modeltype.
			continue;

		Vector bone3D = player->GetBonePosition(boneID);

		Autowall::FireBulletData data;
		float boneDamage = Autowall::GetDamage(bone3D, !Settings::Aimbot::friendly, data);
		if( boneDamage > bestDamage )
		{
			bestBone = boneID;
			if( boneDamage >= player->GetHealth() )
			{
				return boneDamage;
			}
			bestDamage = boneDamage;
		}
	}
	return bestDamage;
}

float GetRealDistanceFOV(float distance, QAngle angle, CUserCmd* cmd)
{
	/*    n
	    w + e
	      s        'real distance'
	                      |
	   a point -> x --..  v
	              |     ''-- x <- a guy
	              |          /
	             |         /
	             |       /
	            | <------------ both of these lines are the same length
	            |    /      /
	           |   / <-----'
	           | /
	          o
	     localplayer
	*/

	Vector aimingAt;
	Math::AngleVectors(cmd->viewangles, aimingAt);
	aimingAt *= distance;

	Vector aimAt;
	Math::AngleVectors(angle, aimAt);
	aimAt *= distance;

	return aimingAt.DistTo(aimAt);
}

Vector VelocityExtrapolate(C_BasePlayer* player, Vector aimPos)
{
	return aimPos + (player->GetVelocity() * globalVars->interval_per_tick);
}

/* Credits to: https://github.com/goldenguy00 ( study! study! study! :^) ) */
int GetClosestBone( CUserCmd* cmd, C_BasePlayer* localPlayer, C_BasePlayer* enemy, AimTargetType aimTargetType = AimTargetType::FOV)
{
	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);

	float tempFov = Settings::Aimbot::AutoAim::fov;
	float tempDistance = Settings::Aimbot::AutoAim::fov * 5.f;

	Vector pVecTarget = localPlayer->GetEyePosition();

	int tempBone = (int)Bone::INVALID;

	const std::map<int, int> *modelType = Util::GetModelTypeBoneMap(enemy);

	static int len = sizeof(Settings::Aimbot::AutoAim::desiredBones) / sizeof(Settings::Aimbot::AutoAim::desiredBones[0]);
	for( int i = 0; i < len; i++ )
	{
		if( !Settings::Aimbot::AutoAim::desiredBones[i] )
			continue;

		int boneID = (*modelType).at(i);
		if( boneID == (int)Bone::INVALID )
			continue;

		Vector cbVecTarget = enemy->GetBonePosition(boneID);

		if( aimTargetType == AimTargetType::FOV )
		{
			float cbFov = Math::GetFov(viewAngles, Math::CalcAngle(pVecTarget, cbVecTarget));

			if( cbFov < tempFov )
			{
				if(Entity::IsVisibleThroughEnemies(enemy, boneID) )
				{
					tempFov = cbFov;
					tempBone = boneID;
				}
			}
		}
		else if( aimTargetType == AimTargetType::REAL_DISTANCE )
		{
			float cbDistance = pVecTarget.DistTo(cbVecTarget);
			float cbRealDistance = GetRealDistanceFOV(cbDistance, Math::CalcAngle(pVecTarget, cbVecTarget), cmd);

			if( cbRealDistance < tempDistance )
			{
				if(Entity::IsVisibleThroughEnemies(enemy, boneID) )
				{
					tempDistance = cbRealDistance;
					tempBone = boneID;
				}
			}
		}
	}
	return tempBone;
}

C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, bool visible, int& bestBone, float& bestDamage, AimTargetType aimTargetType = AimTargetType::FOV)
{
	if (Settings::Aimbot::AutoAim::realDistance)
		aimTargetType = AimTargetType::REAL_DISTANCE;

	bestBone = (int)Settings::Aimbot::bone;
	static C_BasePlayer* lockedOn = NULL;
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	C_BasePlayer* closestEntity = NULL;

	float bestFov = Settings::Aimbot::AutoAim::fov;
	float bestRealDistance = Settings::Aimbot::AutoAim::fov * 5.f;

	if( lockedOn )
	{
		if( lockedOn->GetAlive() && !Entity::IsVisibleThroughEnemies(lockedOn, bestBone) )
		{
			lockedOn = NULL;
			return NULL;
		}
		if (!(cmd->buttons & IN_ATTACK || inputSystem->IsButtonDown(Settings::Aimbot::aimkey)) || lockedOn->GetDormant())//|| !Entity::IsVisible(lockedOn, bestBone, 180.f, Settings::ESP::Filters::smokeCheck))
		{
			lockedOn = NULL;
		}
		else
		{
			if( !lockedOn->GetAlive() )
			{
				if( Settings::Aimbot::AutoAim::engageLockTR )
				{
					if(Util::GetEpochTime() - killTimes.back() > Settings::Aimbot::AutoAim::engageLockTTR) // if we got the kill over the TTR time, engage another foe.
					{
						lockedOn = NULL;
					}
				}
				return NULL;
			}

			if( Settings::Aimbot::AutoAim::closestBone )
			{
				int tempBone = GetClosestBone(cmd, localplayer, lockedOn, aimTargetType);
				if( tempBone == (int)Bone::INVALID )
				{
					return NULL;
				}
				bestBone = tempBone;
			}

			return lockedOn;
		}
	}

	

	for (int i = 1; i < engine->GetMaxClients(); ++i)
	{
		C_BasePlayer* player = (C_BasePlayer*) entityList->GetClientEntity(i);
		Aimbot::targetAimbot = i;

		if (!player
			|| player == localplayer
			|| player->GetDormant()
			|| !player->GetAlive()
			|| player->GetImmune())
			continue;

		if (!Settings::Aimbot::friendly && player->GetTeam() == localplayer->GetTeam())
			continue;

		if( !Aimbot::friends.empty() ) // check for friends, if any
		{
			IEngineClient::player_info_t entityInformation;
			engine->GetPlayerInfo(i, &entityInformation);

			if (std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid) != Aimbot::friends.end())
				continue;
		}

		Vector eVecTarget = player->GetBonePosition((int) Settings::Aimbot::bone);
		if( Settings::Aimbot::AutoAim::closestBone )
		{
			int tempBone = GetClosestBone(cmd, localplayer, player, aimTargetType);
			if( tempBone == (int)Bone::INVALID || !Entity::IsVisibleThroughEnemies(player, tempBone) )
				continue;
			bestBone = tempBone;
		}

		Vector pVecTarget = localplayer->GetEyePosition();

		QAngle viewAngles;
		engine->GetViewAngles(viewAngles);

		float distance = pVecTarget.DistTo(eVecTarget);
		float fov = Math::GetFov(viewAngles, Math::CalcAngle(pVecTarget, eVecTarget));

		if (aimTargetType == AimTargetType::FOV && fov > bestFov)
			continue;

		float realDistance = GetRealDistanceFOV(distance, Math::CalcAngle(pVecTarget, eVecTarget), cmd);

		if (aimTargetType == AimTargetType::REAL_DISTANCE && realDistance > bestRealDistance)
			continue;

		if (visible && !Settings::Aimbot::AutoWall::enabled && !Entity::IsVisible(player, (Settings::Aimbot::AutoAim::closestBone) ? bestBone : (int)Settings::Aimbot::bone))
			continue;

		if (Settings::Aimbot::AutoWall::enabled)
		{
			int bone = (int)Bone::INVALID;
			float damage = AutoWallBestBone(player, bone); // sets bone param, returns damage of hitting that bone.

			if (bone != (int)Bone::INVALID)
			{
				bestDamage = damage;
				bestBone = bone;
				closestEntity = player;
			}
		}
		else
		{
			closestEntity = player;
			bestFov = fov;
			bestRealDistance = realDistance;
		}
	}
	if( Settings::Aimbot::AutoAim::engageLock )
	{
		if( !lockedOn )
		{
			if( (cmd->buttons & IN_ATTACK) || inputSystem->IsButtonDown(Settings::Aimbot::aimkey) )
			{
				if( Util::GetEpochTime() - killTimes.back() > 100 ) // if we haven't gotten a kill in under 100ms.
				{
					lockedOn = closestEntity; // This is to prevent a Rare condition when you one-tap someone without the aimbot, it will lock on to another target.
				}
			}
			else
			{
				return NULL;
			}
		}
	}
	if( bestBone == (int)Bone::INVALID )
		return NULL;
	/*
	if( closestEntity )
	{
		IEngineClient::player_info_t playerInfo;
		engine->GetPlayerInfo(closestEntity->GetIndex(), &playerInfo);
		cvar->ConsoleDPrintf("%'s is Closest. Bone:%d\n", playerInfo.name, bestBone);
	}
	*/

	return closestEntity;
}

void Aimbot::RCS(QAngle& angle, C_BasePlayer* player, CUserCmd* cmd)
{
	if (!Settings::Aimbot::RCS::enabled)
		return;

	if (!(cmd->buttons & IN_ATTACK))
		return;

	bool hasTarget = Settings::Aimbot::AutoAim::enabled && shouldAim && player;

	if (!Settings::Aimbot::RCS::always_on && !hasTarget)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	QAngle CurrentPunch = *localplayer->GetAimPunchAngle();

	if (Settings::Aimbot::silent || hasTarget)
	{
		angle.x -= CurrentPunch.x * Settings::Aimbot::RCS::valueX;
		angle.y -= CurrentPunch.y * Settings::Aimbot::RCS::valueY;
	}
	else if (localplayer->GetShotsFired() > 1)
	{
		QAngle NewPunch = { CurrentPunch.x - RCSLastPunch.x, CurrentPunch.y - RCSLastPunch.y, 0 };

		angle.x -= NewPunch.x * Settings::Aimbot::RCS::valueX;
		angle.y -= NewPunch.y * Settings::Aimbot::RCS::valueY;
	}

	RCSLastPunch = CurrentPunch;
}

void Aimbot::AimStep(C_BasePlayer* player, QAngle& angle, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AimStep::enabled)
		return;

	if (!Settings::Aimbot::AutoAim::enabled)
		return;

	if (Settings::Aimbot::Smooth::enabled)
		return;

	if (!shouldAim)
		return;

	if (!Aimbot::aimStepInProgress)
		AimStepLastAngle = cmd->viewangles;

	if (!player)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	Vector eVecTarget = player->GetBonePosition((int) Settings::Aimbot::bone);
	Vector pVecTarget = localplayer->GetEyePosition();
	float fov = Math::GetFov(AimStepLastAngle, Math::CalcAngle(pVecTarget, eVecTarget));

	Aimbot::aimStepInProgress = fov > Settings::Aimbot::AimStep::value;

	if (!Aimbot::aimStepInProgress)
		return;

	QAngle AimStepDelta = AimStepLastAngle - angle;

	if (AimStepDelta.y < 0)
		AimStepLastAngle.y += Settings::Aimbot::AimStep::value;
	else
		AimStepLastAngle.y -= Settings::Aimbot::AimStep::value;

	AimStepLastAngle.x = angle.x;
	angle = AimStepLastAngle;
}

void Aimbot::AutoCockRevolver(C_BaseCombatWeapon* activeWeapon, C_BasePlayer* localplayer, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoCockRevolver::enabled)
		return;

	if (cmd->buttons & IN_RELOAD)
		return;
	
	if (*activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
		return;
	
	cmd->buttons |= IN_ATTACK;
	float postponeFireReady = activeWeapon->GetPostponeFireReadyTime();
	if (cmd->buttons & IN_ATTACK2)
		cmd->buttons |= IN_ATTACK;
	else if (postponeFireReady > 0 && postponeFireReady < globalVars->curtime)
	{
		cmd->buttons &= ~IN_ATTACK;
	}
}

void Salt(float& smooth)
{
	float sine = sin (globalVars->tickcount);
	float salt = sine * Settings::Aimbot::Smooth::Salting::multiplier;
	float oval = smooth + salt;
	smooth *= oval;
}

void Aimbot::Smooth(C_BasePlayer* player, QAngle& angle, CUserCmd* cmd)
{
	if (!Settings::Aimbot::Smooth::enabled)
		return;

	if (Settings::AntiAim::Pitch::enabled || Settings::AntiAim::Yaw::enabled)
		return;

	if (!shouldAim || !player)
		return;

	if (Settings::Aimbot::silent)
		return;

	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);

	QAngle delta = angle - viewAngles;
	Math::NormalizeAngles(delta);

	float smooth = powf(Settings::Aimbot::Smooth::value, 0.4f); // Makes more slider space for actual useful values

	smooth = std::min(0.99f, smooth);

	if (Settings::Aimbot::Smooth::Salting::enabled)
		Salt(smooth);

	QAngle toChange = QAngle();

	SmoothType type = Settings::Aimbot::Smooth::type;

	if (type == SmoothType::SLOW_END)
		toChange = delta - (delta * smooth);
	else if (type == SmoothType::CONSTANT || type == SmoothType::FAST_END)
	{
		float coeff = (1.0f - smooth) / delta.Length() * 4.f;

		if (type == SmoothType::FAST_END)
			coeff = powf(coeff, 2.f) * 10.f;

		coeff = std::min(1.f, coeff);
		toChange = delta * coeff;
	}

	angle = viewAngles + toChange;
}

void Aimbot::AutoCrouch(C_BasePlayer* player, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoCrouch::enabled)
		return;

	if (!player)
		return;

	cmd->buttons |= IN_DUCK;
}

void Aimbot::AutoSlow(C_BasePlayer* player, float& forward, float& sideMove, float& bestDamage, C_BaseCombatWeapon* active_weapon, CUserCmd* cmd)
{

	if (!Settings::Aimbot::AutoSlow::enabled){
		Settings::Aimbot::AutoSlow::goingToSlow = false;
		return;
	}

	if (!player){
		Settings::Aimbot::AutoSlow::goingToSlow = false;
		return;
	}

	float nextPrimaryAttack = active_weapon->GetNextPrimaryAttack();

	if (nextPrimaryAttack > globalVars->curtime){
		Settings::Aimbot::AutoSlow::goingToSlow = false;
		return;
	}

	Settings::Aimbot::AutoSlow::goingToSlow = true;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon || activeWeapon->GetAmmo() == 0)
		return;

	if( localplayer->GetVelocity().Length() > (activeWeapon->GetCSWpnData()->GetMaxPlayerSpeed() / 3) ) // https://youtu.be/ZgjYxBRuagA
	{
		cmd->buttons |= IN_WALK;
		forward = -forward;
		sideMove = -sideMove;
		cmd->upmove = 0;
	}
}

void Aimbot::AutoPistol(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoPistol::enabled)
		return;

	if (!activeWeapon || activeWeapon->GetCSWpnData()->GetWeaponType() != CSWeaponType::WEAPONTYPE_PISTOL)
		return;

	if (activeWeapon->GetNextPrimaryAttack() < globalVars->curtime)
		return;

	if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
		cmd->buttons &= ~IN_ATTACK2;
	else
		cmd->buttons &= ~IN_ATTACK;
}

float GetDistanceForward (C_BasePlayer* player, QAngle angles, CUserCmd* cmd)
{	
	Vector src3D, dst3D, forward;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	
	Math::AngleVectors(angles, forward);
	filter.pSkip = player;
	src3D = player->GetEyePosition();
	dst3D = src3D + (forward * 8192);
		
	ray.Init(src3D, dst3D);
	trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	
	float dX = tr.endpos.x - src3D.x;
	float dY = tr.endpos.y - src3D.y;
	float dZ = tr.endpos.z - src3D.z;

	return cbrt( (dX * dX) + (dY * dY) + (dZ * dZ));
}

void Aimbot::AutoShoot(C_BasePlayer* player, int bone, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoShoot::enabled)
		return;

	if (Settings::Aimbot::AimStep::enabled && Aimbot::aimStepInProgress)
		return;

	if (!player || !activeWeapon || activeWeapon->GetAmmo() == 0)
		return;

	CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
	if (weaponType == CSWeaponType::WEAPONTYPE_KNIFE || weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE)
		return;

	if (cmd->buttons & IN_USE)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	
	
	float spreadValue = Settings::Aimbot::SpreadLimit::value;
	if (Settings::Aimbot::SpreadLimit::distanceBased)
		spreadValue *= 100.0f / GetDistanceForward(localplayer, cmd->viewangles, cmd);
	if( Settings::Aimbot::SpreadLimit::enabled && ((activeWeapon->GetSpread() + activeWeapon->GetInaccuracy()) > spreadValue))
		return;

	if (Settings::Aimbot::HitChance::enabled && !Aimbot::HitChance(player->GetBonePosition(bone), !Settings::Aimbot::friendly, localplayer))
		return;
	
	if( Settings::Aimbot::AutoShoot::velocityCheck && localplayer->GetVelocity().Length() > (activeWeapon->GetCSWpnData()->GetMaxPlayerSpeed() / 3) )
		return;

	float nextPrimaryAttack = activeWeapon->GetNextPrimaryAttack();

	if (nextPrimaryAttack > globalVars->curtime)
	{
		if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
			cmd->buttons &= ~IN_ATTACK2;
		else
			cmd->buttons &= ~IN_ATTACK;
	}
	else
	{
		if (Settings::Aimbot::AutoShoot::autoscope && activeWeapon->GetCSWpnData()->GetZoomLevels() > 0 && !localplayer->IsScoped())
			cmd->buttons |= IN_ATTACK2;
		else if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
			cmd->buttons |= IN_ATTACK2;
		else
			cmd->buttons |= IN_ATTACK;
	}
}

void Aimbot::ShootCheck(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
	if (!Settings::AntiAim::Pitch::enabled && !Settings::AntiAim::Yaw::enabled)
		return;

	if (!Settings::Aimbot::silent)
		return;

	if (!(cmd->buttons & IN_ATTACK))
		return;

	if (activeWeapon->GetNextPrimaryAttack() < globalVars->curtime)
		return;

	if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_C4)
		return;

	if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
		cmd->buttons &= ~IN_ATTACK2;
	else
		cmd->buttons &= ~IN_ATTACK;
}

void Aimbot::NoShoot(C_BaseCombatWeapon* activeWeapon, C_BasePlayer* player, C_BasePlayer* localplayer, CUserCmd* cmd)
{
	if (Settings::Aimbot::NoShoot::enabled) {
		CTraceFilterEntitiesOnly filter;

		Vector src, dir, dest;
		trace_t tr;
		Ray_t ray;
		src = localplayer->GetEyePosition();
		QAngle angles = cmd->viewangles;
		Math::AngleVectors(angles, dir);
		dest = src + (dir * 8192);

		ray.Init(src, dest);
		filter.pSkip = localplayer;
		trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

		C_BasePlayer* player = (C_BasePlayer*) tr.m_pEntityHit;
		if (!player || player->GetClientClass()->m_ClassID != EClassIds::CCSPlayer || player == localplayer || player->GetDormant() || !player->GetAlive() || player->GetImmune() || (player->GetTeam() == localplayer->GetTeam() && !Settings::Aimbot::friendly))
		{
			if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_C4)
				return;
	
			if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
				cmd->buttons &= ~IN_ATTACK2;
			else
				cmd->buttons &= ~IN_ATTACK;
		}
	}
}

void Aimbot::CreateMove(CUserCmd* cmd)
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer || !localplayer->GetAlive())
		return;

	Aimbot::UpdateValues();

	if (!Settings::Aimbot::enabled)
		return;

	QAngle oldAngle;
	engine->GetViewAngles(oldAngle);
	float oldForward = cmd->forwardmove;
	float oldSideMove = cmd->sidemove;

	QAngle angle = cmd->viewangles;
	static bool newTarget = true;
	static QAngle lastRandom = {0,0,0};

	shouldAim = Settings::Aimbot::AutoShoot::enabled;

	if (Settings::Aimbot::IgnoreJump::enabled && !(localplayer->GetFlags() & FL_ONGROUND))
		return;

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon || activeWeapon->GetInReload())
		return;

	CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
	if (weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE || weaponType == CSWeaponType::WEAPONTYPE_KNIFE)
		return;

	int aw_bone;
	float bestDamage = 0.0f;
	C_BasePlayer* player = GetClosestPlayer(cmd, true, aw_bone, bestDamage);

	if (player)
	{
		bool skipPlayer = false;
		
		Vector eVecTarget = player->GetBonePosition(aw_bone);
		Vector pVecTarget = localplayer->GetEyePosition();

		if (Settings::Aimbot::SmokeCheck::enabled && LineGoesThroughSmoke(pVecTarget, eVecTarget, true))
			skipPlayer = true;

		if (Settings::Aimbot::FlashCheck::enabled && localplayer->GetFlashBangTime() - globalVars->curtime > 2.0f)
			skipPlayer = true;

		if (skipPlayer)
			player = nullptr;

		if (Settings::Aimbot::AutoAim::enabled && !skipPlayer)
		{
			if (cmd->buttons & IN_ATTACK && !Settings::Aimbot::aimkeyOnly)
				shouldAim = true;

			if (inputSystem->IsButtonDown(Settings::Aimbot::aimkey))
				shouldAim = true;

			if (shouldAim)
			{
				//IEngineClient::player_info_t playerInfo;
				//engine->GetPlayerInfo(player->GetIndex(), &playerInfo);
				//studiohdr_t* pStudioModel = modelInfo->GetStudioModel(player->GetModel());
				//cvar->ConsoleDPrintf("Model ID: %d, Name: %s, NumOfBones: %d, len %d\n", pStudioModel->id, pStudioModel->name, pStudioModel->numbones);


				//cvar->ConsoleDPrintf("Aiming at: %s, he is a %s \n", playerInfo.name, Util::ModelTypeToString(Util::GetModelTypeID(player)).c_str());


				if (Settings::Aimbot::Prediction::enabled)
				{
					pVecTarget = VelocityExtrapolate(localplayer, pVecTarget); // get eye pos next tick
					eVecTarget = VelocityExtrapolate(player, eVecTarget); // get target pos next tick
				}
				
				angle = Math::CalcAngle(pVecTarget, eVecTarget);
				if (Settings::Aimbot::Smooth::enabled && Settings::Aimbot::Curve::enabled) {
					float dist = Math::ClampYaw(angle.y - oldAngle.y);
					if (dist > 180.0f) dist = 360.0f - dist;
					eVecTarget += Vector(0, 0, Settings::Aimbot::Curve::value * dist);
					angle = Math::CalcAngle(pVecTarget, eVecTarget);
				}
				//cvar->ConsoleDPrintf("Raw Angle = (%.2f, %.2f, %.2f)\n", angle.x, angle.y, angle.z);

				if (Settings::Aimbot::ErrorMargin::enabled)
				{
					static int lastShotFired = 0;
					if( (localplayer->GetShotsFired() > lastShotFired) || newTarget )//get new random spot when firing a shot or when aiming at a new target
					{
						lastRandom = ApplyErrorToAngle(&angle, Settings::Aimbot::ErrorMargin::value);
					}

					if( lastRandom.x != 0 && lastRandom.y != 0 && lastRandom.z != 0 )
					{
						ApplyOffsetToAngle(&angle,&lastRandom);
					}

					lastShotFired = localplayer->GetShotsFired();
				}
				newTarget = false;
			}
			else
			{
				newTarget = true;
				lastRandom = {0,0,0};
			}
		}
	}
	else
	{
		newTarget = true;
		lastRandom = {0,0,0};
	}


	Aimbot::AimStep(player, angle, cmd);
	Aimbot::AutoCrouch(player, cmd);
	Aimbot::AutoSlow(player, oldForward, oldSideMove, bestDamage, activeWeapon, cmd);
	Aimbot::AutoPistol(activeWeapon, cmd);
	Aimbot::AutoCockRevolver(activeWeapon, player, cmd);
	Aimbot::AutoShoot(player, aw_bone, activeWeapon, cmd);
	Aimbot::RCS(angle, player, cmd);
	Aimbot::Smooth(player, angle, cmd);
	Aimbot::ShootCheck(activeWeapon, cmd);
	Aimbot::NoShoot(activeWeapon, player, localplayer, cmd);

	Math::NormalizeAngles(angle);
	Math::ClampAngles(angle);

	if(Settings::Aimbot::moveMouse && !Settings::Aimbot::silent) // not a good idea to use mouse-based aimbot and silent aim at the same time
		{
			float sensitivity = cvar->FindVar("sensitivity")->GetFloat();
			float m_pitch = cvar->FindVar("m_pitch")->GetFloat();
			float m_yaw = cvar->FindVar("m_yaw")->GetFloat();
			
			xdo_move_mouse_relative(xdo, (int) -( (angle.y - oldAngle.y) / (m_pitch * sensitivity) ),
									     (int) ( (angle.x - oldAngle.x) / (m_yaw * sensitivity))
								   );
		}
		else
		{
			cmd->viewangles = angle;
		}

	Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);

	bool bulletTime = true;

	if (activeWeapon->GetNextPrimaryAttack() > globalVars->curtime)
		bulletTime = false;

	if (Settings::Aimbot::pSilent && (cmd->buttons & IN_ATTACK) && bulletTime)
		CreateMove::sendPacket = false;

	if (!Settings::Aimbot::silent)
		engine->SetViewAngles(cmd->viewangles);
}

void Aimbot::FireGameEvent(IGameEvent* event)
{
	if (!event)
		return;

	if (strcmp(event->GetName(), "player_connect_full") == 0 || strcmp(event->GetName(), "cs_game_disconnected") == 0 )
	{
		if (event->GetInt("userid") && engine->GetPlayerForUserID(event->GetInt("userid")) != engine->GetLocalPlayer())
			return;
		Aimbot::friends.clear();
	}
	if( strcmp(event->GetName(), "player_death") == 0 )
	{
		int attacker_id = engine->GetPlayerForUserID(event->GetInt("attacker"));
		int deadPlayer_id = engine->GetPlayerForUserID(event->GetInt("userid"));

		if (attacker_id == deadPlayer_id) // suicide
			return;

		if (attacker_id != engine->GetLocalPlayer())
			return;

		killTimes.push_back(Util::GetEpochTime());
	}
}

void Aimbot::UpdateValues()
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon)
		return;

	ItemDefinitionIndex index = ItemDefinitionIndex::INVALID;
	if (Settings::Aimbot::weapons.find(*activeWeapon->GetItemDefinitionIndex()) != Settings::Aimbot::weapons.end())
		index = *activeWeapon->GetItemDefinitionIndex();

	const AimbotWeapon_t& currentWeaponSetting = Settings::Aimbot::weapons.at(index);

	Settings::Aimbot::enabled = currentWeaponSetting.enabled;
	Settings::Aimbot::silent = currentWeaponSetting.silent;
	Settings::Aimbot::friendly = currentWeaponSetting.friendly;
	Settings::Aimbot::bone = currentWeaponSetting.bone;
	Settings::Aimbot::aimkey = currentWeaponSetting.aimkey;
	Settings::Aimbot::aimkeyOnly = currentWeaponSetting.aimkeyOnly;
	Settings::Aimbot::Smooth::enabled = currentWeaponSetting.smoothEnabled;
	Settings::Aimbot::Smooth::value = currentWeaponSetting.smoothAmount;
	Settings::Aimbot::Smooth::type = currentWeaponSetting.smoothType;
	Settings::Aimbot::Curve::enabled = currentWeaponSetting.curveEnabled;
	Settings::Aimbot::Curve::value = currentWeaponSetting.curveAmount;
	Settings::Aimbot::ErrorMargin::enabled = currentWeaponSetting.errorMarginEnabled;
	Settings::Aimbot::ErrorMargin::value = currentWeaponSetting.errorMarginValue;
	Settings::Aimbot::AutoAim::enabled = currentWeaponSetting.autoAimEnabled;
	Settings::Aimbot::AutoAim::fov = currentWeaponSetting.autoAimFov;
	Settings::Aimbot::AutoAim::closestBone = currentWeaponSetting.closestBone;
	Settings::Aimbot::AutoAim::engageLock = currentWeaponSetting.engageLock;
	Settings::Aimbot::AutoAim::engageLockTR = currentWeaponSetting.engageLockTR;
	Settings::Aimbot::AutoAim::engageLockTTR = currentWeaponSetting.engageLockTTR;
	Settings::Aimbot::AimStep::enabled = currentWeaponSetting.aimStepEnabled;
	Settings::Aimbot::AimStep::value = currentWeaponSetting.aimStepValue;
	Settings::Aimbot::AutoPistol::enabled = currentWeaponSetting.autoPistolEnabled;
	Settings::Aimbot::AutoShoot::enabled = currentWeaponSetting.autoShootEnabled;
	Settings::Aimbot::AutoShoot::autoscope = currentWeaponSetting.autoScopeEnabled;
	Settings::Aimbot::RCS::enabled = currentWeaponSetting.rcsEnabled;
	Settings::Aimbot::RCS::always_on = currentWeaponSetting.rcsAlwaysOn;
	Settings::Aimbot::RCS::valueX = currentWeaponSetting.rcsAmountX;
	Settings::Aimbot::RCS::valueY = currentWeaponSetting.rcsAmountY;
	Settings::Aimbot::AutoCockRevolver::enabled = currentWeaponSetting.autoCockRevolver;
	Settings::Aimbot::NoShoot::enabled = currentWeaponSetting.noShootEnabled;
	Settings::Aimbot::IgnoreJump::enabled = currentWeaponSetting.ignoreJumpEnabled;
	Settings::Aimbot::Smooth::Salting::enabled = currentWeaponSetting.smoothSaltEnabled;
	Settings::Aimbot::Smooth::Salting::multiplier = currentWeaponSetting.smoothSaltMultiplier;
	Settings::Aimbot::SmokeCheck::enabled = currentWeaponSetting.smokeCheck;
	Settings::Aimbot::FlashCheck::enabled = currentWeaponSetting.flashCheck;
	Settings::Aimbot::SpreadLimit::enabled = currentWeaponSetting.spreadLimitEnabled;
	Settings::Aimbot::SpreadLimit::distanceBased = currentWeaponSetting.spreadLimitDistance;
	Settings::Aimbot::SpreadLimit::value = currentWeaponSetting.spreadLimit;
	Settings::Aimbot::AutoWall::enabled = currentWeaponSetting.autoWallEnabled;
	Settings::Aimbot::AutoWall::value = currentWeaponSetting.autoWallValue;
	Settings::Aimbot::AutoSlow::enabled = currentWeaponSetting.autoSlow;
	Settings::Aimbot::HitChance::value = currentWeaponSetting.hitChanceValue;
	Settings::Aimbot::HitChance::hitRays = currentWeaponSetting.hitChanceRays;
	Settings::Aimbot::HitChance::enabled = currentWeaponSetting.hitChanceEnabled;

	for (int i = (int) Hitbox::HITBOX_HEAD; i <= (int) Hitbox::HITBOX_ARMS; i++)
		Settings::Aimbot::AutoWall::bones[i] = currentWeaponSetting.autoWallBones[i];

	for (int bone = (int) DesiredBones::BONE_PELVIS; bone <= (int) DesiredBones::BONE_RIGHT_SOLE; bone++)
		Settings::Aimbot::AutoAim::desiredBones[bone] = currentWeaponSetting.desiredBones[bone];

	Settings::Aimbot::AutoAim::realDistance = currentWeaponSetting.autoAimRealDistance;
	Settings::Aimbot::moveMouse = currentWeaponSetting.moveMouse;
}
