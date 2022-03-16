#include "framework.h"
#include "Game/Lara/lara_basic.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/health.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_slide.h"
#include "Game/Lara/lara_monkey.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Scripting/GameFlowScript.h"

// ------------------------------
// BASIC MOVEMENT & MISCELLANEOUS
// Control & Collision Functions
// ------------------------------

// --------------
// MISCELLANEOUS:
// --------------

void lara_void_func(ITEM_INFO* item, CollisionInfo* coll)
{
	return;
}

void lara_default_col(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);
}

void lara_as_special(ITEM_INFO* item, CollisionInfo* coll)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_null(ITEM_INFO* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

void lara_as_controlled(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1)
	{
		lara->Control.HandStatus = HandStatus::Free;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlled_no_look(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

// State:		LS_VAULT (164),
//				LS_VAULT_2_STEPS (165), LS_VAULT_3_STEPS (166),
//				VAULT_1_STEP_CROUCH (167), VAULT_2_STEPS_CROUCH (168), VAULT_3_STEPS_CROUCH (169)
// Collision:	lara_void_func()
void lara_as_vault(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	EaseOutLaraHeight(item, lara->ProjectedFloorHeight - item->Position.yPos);
	ApproachLaraTargetAngle(item, lara->TargetFacingAngle, 2.5f);
	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_AUTO_JUMP (62)
// Collision:	lara_lara_void_func()
void lara_as_auto_jump(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	
	ApproachLaraTargetAngle(item, lara->TargetFacingAngle, 2.5f);
}

// ---------------
// BASIC MOVEMENT:
// ---------------

// State:		LS_WALK_FORWARD (0)
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Count.RunJump++;
	if (lara->Control.Count.RunJump > (LARA_RUN_JUMP_TIME / 2 + 4))
		lara->Control.Count.RunJump = LARA_RUN_JUMP_TIME / 2 + 4;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	// TODO: Implement item alignment properly someday. @Sezz 2021.11.01
	if (lara->Control.IsMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK) [[likely]]
			item->Animation.TargetState = LS_WALK_FORWARD;
		else
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_RUN_FORWARD (1)
// Collision:	lara_col_run_forward()
void lara_as_run_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Count.RunJump++;
	if (lara->Control.Count.RunJump > LARA_RUN_JUMP_TIME)
		lara->Control.Count.RunJump = LARA_RUN_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_FAST_TURN_MAX)
			lara->Control.TurnRate = -LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_FAST_TURN_MAX)
			lara->Control.TurnRate = LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if ((TrInput & IN_JUMP || lara->Control.RunJumpQueued) &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		if (lara->Control.Count.RunJump >= LARA_RUN_JUMP_TIME &&
			TestLaraRunJumpForward(item, coll))
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if (TrInput & IN_SPRINT && lara->SprintEnergy &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_SPRINT;
		return;
	}

	if ((TrInput & (IN_ROLL | IN_FORWARD & IN_BACK)) && !lara->Control.RunJumpQueued &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)) &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK)
			item->Animation.TargetState = LS_WALK_FORWARD;
		else [[likely]]
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run_forward()
void lara_col_run_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->Position.zRot = 0;

		if (TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)) ||
			coll->HitTallObject)
		{
			item->Animation.TargetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Animation.ActiveState = LS_SPLAT;
				return;
			}
		}

		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_IDLE (2), LS_SPLAT_SOFT (170)
// Collision:	lara_col_idle()
void lara_as_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = ((isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) || item->Animation.AnimNumber == LA_SWANDIVE_ROLL) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	// Handles waterskin and clockwork beetle.
	// TODO: Hardcoding.
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_LOOK && lara->Control.CanLook)
		LookUpDown(item);

	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))	// JUMP locks y rotation.
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			PseudoLaraAsSwampIdle(item, coll);
		else [[likely]]
			PseudoLaraAsWadeIdle(item, coll);

		return;
	}

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
			item->Animation.TargetState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_SPRINT ||
			lara->Control.TurnRate <= -LARA_SLOW_TURN_MAX ||
			(lara->Control.HandStatus == HandStatus::WeaponReady && lara->Control.Weapon.GunType != LaraWeaponType::Torch) ||
			(lara->Control.HandStatus == HandStatus::WeaponDraw && lara->Control.Weapon.GunType != LaraWeaponType::Flare))
		{
			item->Animation.TargetState = LS_TURN_LEFT_FAST;
		}
		else [[likely]]
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_SPRINT ||
			lara->Control.TurnRate >= LARA_SLOW_TURN_MAX ||
			(lara->Control.HandStatus == HandStatus::WeaponReady && lara->Control.Weapon.GunType != LaraWeaponType::Torch) ||
			(lara->Control.HandStatus == HandStatus::WeaponDraw && lara->Control.Weapon.GunType != LaraWeaponType::Flare))
		{
			item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		}
		else [[likely]]
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	// TODO: Without animation blending, the AFK state's
	// movement lock will be rather obnoxious.
	// Adding some idle breathing would also be nice. @Sezz 2021.10.31
	if (lara->Control.Count.Pose >= LARA_POSE_TIME && TestLaraPose(item, coll) &&
		g_GameFlow->Animations.HasPose)
	{
		item->Animation.TargetState = LS_POSE;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// TODO: Future-proof for rising water.
// TODO: Make these into true states someday? It may take a bit of work. @Sezz 2021.10.13
// Pseudo-state for idling in wade-height water.
void PseudoLaraAsWadeIdle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_PREPARE;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for idling in swamps.
void PseudoLaraAsSwampIdle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll))
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeftSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRightSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_IDLE (2), LS_POSE (4), LS_SPLAT_SOFT (170)
// Control:		lara_as_idle(), lara_as_pose()
void lara_col_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	lara->Control.MoveAngle = (item->Animation.Velocity >= 0) ? item->Position.yRot : (item->Position.yRot + ANGLE(180.0f));
	coll->Setup.LowerFloorBound = isSwamp ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = isSwamp ? false : true;
	coll->Setup.FloorSlopeIsWall = isSwamp ? false : true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_POSE (4)
// Collision:	lara_col_idle()
void lara_as_pose(ITEM_INFO* item, CollisionInfo* coll)
{
	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TestLaraPose(item, coll))
	{
		if (TrInput & IN_ROLL)
		{
			item->Animation.TargetState = LS_ROLL_FORWARD;
			return;
		}

		if (TrInput & IN_WAKE)
		{
			item->Animation.TargetState = LS_IDLE;
			return;
		}

		item->Animation.TargetState = LS_POSE;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Collision:	lara_col_run_back()
void lara_as_run_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Control:		lara_as_run_back()
void lara_col_run_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	item->Animation.VerticalVelocity = 0;
	item->Animation.Airborne = false;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2))
	{
		SetLaraFallBackAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Collision:	lara_col_turn_right_slow()
void lara_as_turn_right_slow(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	lara->Control.TurnRate += LARA_TURN_RATE;
	if (lara->Control.TurnRate < 0)
		lara->Control.TurnRate = 0;
	else if (lara->Control.TurnRate > LARA_MED_FAST_TURN_MAX)
		lara->Control.TurnRate = LARA_MED_FAST_TURN_MAX;

	if (lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			PsuedoLaraAsSwampTurnRightSlow(item, coll);
		else [[likely]]
			PsuedoLaraAsWadeTurnRightSlow(item, coll);

		return;
	}

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_WALK) // TODO: This hasn't worked since TR1.
		{
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

			if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
				lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
		}
		else if (lara->Control.TurnRate > LARA_SLOW_MED_TURN_MAX)
			item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		else [[likely]]
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in wade-height water.
void PsuedoLaraAsWadeTurnRightSlow(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.TurnRate > LARA_WADE_TURN_MAX)
		lara->Control.TurnRate = LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_PREPARE;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in swamps.
void PsuedoLaraAsSwampTurnRightSlow(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.TurnRate > LARA_SWAMP_TURN_MAX)
		lara->Control.TurnRate = LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll)) [[likely]]
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeftSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRightSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Control:		lara_as_turn_right_slow()
void lara_col_turn_right_slow(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_left_slow()
void lara_as_turn_left_slow(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	lara->Control.TurnRate -= LARA_TURN_RATE;
	if (lara->Control.TurnRate > 0)
		lara->Control.TurnRate = 0;
	else if (lara->Control.TurnRate < -LARA_MED_FAST_TURN_MAX)
		lara->Control.TurnRate = -LARA_MED_FAST_TURN_MAX;

	if (lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			PsuedoLaraAsSwampTurnLeftSlow(item, coll);
		else [[likely]]
			PsuedoLaraAsWadeTurnLeftSlow(item, coll);

		return;
	}

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if(TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_WALK)
		{
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

			if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
				lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
		}
		else if (lara->Control.TurnRate < -LARA_SLOW_MED_TURN_MAX)
			item->Animation.TargetState = LS_TURN_LEFT_FAST;
		else [[likely]]
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in wade-height water.
void PsuedoLaraAsWadeTurnLeftSlow(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.TurnRate < -LARA_WADE_TURN_MAX)
		lara->Control.TurnRate = -LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_PREPARE;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in swamps.
void PsuedoLaraAsSwampTurnLeftSlow(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.TurnRate < -LARA_SWAMP_TURN_MAX)
		lara->Control.TurnRate = -LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_left_slow()
void lara_col_turn_left_slow(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_turn_right_slow(item, coll);
}

// State:		LS_DEATH (8)
// Collision:	lara_col_death()
void lara_as_death(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(ANGLE(80.0f));
		item->MeshBits = -1;
		lara->Inventory.IsBusy = false;
	}
}

// State:		LS_DEATH (8)
// Control:		lara_as_death()
void lara_col_death(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.Airborne = false;
	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Radius = LARA_RAD_DEATH;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	StopSoundEffect(SFX_TR4_LARA_FALL);
	item->HitPoints = -1;
	lara->Air = -1;

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPLAT (12)
// Collision:	lara_col_splat()
void lara_as_splat(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
}

// State:		LS_SPLAT (12)
// Control:		lara_as_splat()
void lara_col_splat(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	ShiftItem(item, coll);

	if (abs(coll->Middle.Floor) <= CLICK(1))
		LaraSnapToHeight(item, coll);
}

// State:		LS_WALK_BACK (16)
// Collision:	lara_col_walk_back()
void lara_as_walk_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara->Control.IsMoving)
		return;

	if (isSwamp &&
		lara->Control.WaterStatus == WaterStatus::Wade)
	{
		PseudoLaraAsSwampWalkBack(item, coll);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);

	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || lara->Control.WaterStatus == WaterStatus::Wade))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for walking back in swamps.
void PseudoLaraAsSwampWalkBack(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX / 3)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX / 3;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);

	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX / 3)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX / 3;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_BACK)
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_walk_back()
void lara_col_walk_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (lara->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = isSwamp ? false : true;
	coll->Setup.FloorSlopeIsWall = isSwamp ? false : true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_TURN_RIGHT_FAST (20)
// Collision:	lara_col_turn_right_fast()
void lara_as_turn_right_fast(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	lara->Control.TurnRate += LARA_TURN_RATE;
	if (lara->Control.TurnRate < LARA_MED_TURN_MAX)
		lara->Control.TurnRate = LARA_MED_TURN_MAX;
	else if (lara->Control.TurnRate > LARA_FAST_TURN_MAX)
		lara->Control.TurnRate = LARA_FAST_TURN_MAX;

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)) &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
		{
			if (TestLaraRunForward(item, coll))
			{
				item->Animation.TargetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	// TODO: Hold WALK to slow down again.
	if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_FAST (20)
// Control:		lara_as_turn_right_fast()
void lara_col_turn_right_fast(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_FAST (152)
// Collision:	lara_col_turn_left_fast()
void lara_as_turn_left_fast(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	lara->Control.TurnRate -= LARA_TURN_RATE;
	if (lara->Control.TurnRate > -LARA_MED_TURN_MAX)
		lara->Control.TurnRate = -LARA_MED_TURN_MAX;
	else if (lara->Control.TurnRate < -LARA_FAST_TURN_MAX)
		lara->Control.TurnRate = -LARA_FAST_TURN_MAX;

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)) &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if(lara->Control.WaterStatus == WaterStatus::Wade)
		{
			if (TestLaraRunForward(item, coll))
			{
				item->Animation.TargetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && 	TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_FAST;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_LEFT_FAST (152)
// Control:		lara_as_turn_left_fast()
void lara_col_turn_left_fast(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_STEP_RIGHT (21)
// Collision:	lara_col_step_right()
void lara_as_step_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara->Control.IsMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_RSTEP)
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}
	
	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_step_right()
void lara_col_step_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (lara->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = isSwamp ? false : true;
	coll->Setup.FloorSlopeIsWall = isSwamp ? false : true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || isSwamp)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_STEP_LEFT (22)
// Collision:	lara_col_step_left()
void lara_as_step_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara->Control.IsMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_LSTEP)
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_step_left()
void lara_col_step_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (lara->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = isSwamp ? false : true;
	coll->Setup.FloorSlopeIsWall = isSwamp ? false : true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || isSwamp)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_ROLL_BACK (23)
// Collision:	lara_col_roll_back()
void lara_as_roll_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_BACK;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_ROLL_BACK (23)
// Control:		lara_as_roll_back()
void lara_col_roll_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	Camera.laraNode = 0;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2)) // Was 200.
	{
		SetLaraFallBackAnimation(item);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_ROLL_FORWARD (45)
// Collision:	lara_col_roll_forward()
void lara_as_roll_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	// TODO: Change swandive roll anim state to something sensible.
	if (TrInput & IN_FORWARD &&
		item->Animation.AnimNumber == LA_SWANDIVE_ROLL) // Hack.
	{
		item->Animation.TargetState = LS_RUN_FORWARD;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_ROLL_FORWARD (45)
// Control:		lara_as_roll_forward()
void lara_col_roll_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	item->Animation.Airborne = false;
	item->Animation.VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_WADE_FORWARD (65)
// Collision:	lara_col_wade_forward()
void lara_as_wade_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_IDLE;
		return;
	}

	if (isSwamp)
	{
		PseudoLaraAsSwampWadeForward(item, coll);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_FAST_TURN_MAX)
			lara->Control.TurnRate = -LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_FAST_TURN_MAX)
			lara->Control.TurnRate = LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Dry)
			item->Animation.TargetState = LS_RUN_FORWARD;
		else [[likely]]
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for wading in swamps.
void PseudoLaraAsSwampWadeForward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SWAMP_TURN_MAX)
			lara->Control.TurnRate = -LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SWAMP_TURN_MAX)
			lara->Control.TurnRate = LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else [[likely]]
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade_forward()
void lara_col_wade_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = isSwamp ? false : true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->Position.zRot = 0;

		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || isSwamp)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPRINT (73)
// Collision:	lara_col_sprint()
void lara_as_sprint(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->SprintEnergy--;

	lara->Control.Count.RunJump++;
	if (lara->Control.Count.RunJump > LARA_RUN_JUMP_TIME)
		lara->Control.Count.RunJump = LARA_RUN_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if (TrInput & IN_JUMP)
	{
		item->Animation.TargetState = LS_SPRINT_DIVE;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// while meeting some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && lara->Control.HandStatus == HandStatus::Free &&
			!TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)) && // HACK: Allow immediate vault only in the case of a soft splat.
			vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_RUN_FORWARD;	// TODO: Dispatch to wade forward state directly. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->Animation.TargetState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && lara->SprintEnergy > 0) [[likely]]
			item->Animation.TargetState = LS_SPRINT;
		else
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_SPRINT (73)
// Control:		lara_as_sprint()
void lara_col_sprint(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->Position.zRot = 0;

		if (TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)) ||
			coll->HitTallObject)
		{
			item->Animation.TargetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Animation.ActiveState = LS_SPLAT;
				return;
			}
		}

		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPRINT_DIVE (74)
// Collision:	lara_col_sprint_dive()
void lara_as_sprint_dive(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Count.RunJump++;
	if (lara->Control.Count.RunJump > LARA_RUN_JUMP_TIME)
		lara->Control.Count.RunJump = LARA_RUN_JUMP_TIME;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -(LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, (LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}

	item->Animation.TargetState = LS_RUN_FORWARD;
}

// State:		LS_SPRINT_DIVE (74)
// Control:		lara_col_sprint_dive()
void lara_col_sprint_dive(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = (item->Animation.Velocity >= 0) ? item->Position.yRot : item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (item->Animation.Velocity < 0)
		lara->Control.MoveAngle = item->Position.yRot; // ???

	if (coll->Middle.Floor <= 0 && item->Animation.VerticalVelocity > 0)
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0) // TODO: It seems Core wanted to make the sprint dive a true jump.
			item->Animation.TargetState = LS_DEATH;
		else if (!(TrInput & IN_FORWARD) || TrInput & IN_WALK || lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_IDLE;
		else
			item->Animation.TargetState = LS_RUN_FORWARD;

		item->Animation.Airborne = false;
		item->Animation.VerticalVelocity = 0;
		item->Position.yPos += coll->Middle.Floor;
		item->Animation.Velocity = 0;

		AnimateLara(item);
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}
