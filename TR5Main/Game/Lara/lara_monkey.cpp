#include "framework.h"
#include "Game/Lara/lara_monkey.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Scripting/GameFlowScript.h"

// -----------------------------
// MONKEY SWING
// Control & Collision Functions
// -----------------------------

// State:		LS_MONKEY_IDLE (75)
// Collision:	lara_col_monkey_idle()
void lara_as_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	// TODO: overhang
	//SlopeMonkeyExtra(item, coll);

	// This check is needed to prevent stealing goal state from previously set.
	//if (item->TargetState == LS_MONKEYSWING_IDLE)
	//	return;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX / 2)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX / 2)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX / 2;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		if (TrInput & IN_ROLL)
		{
			item->TargetState = LS_MONKEY_TURN_180;
			return;
		}

		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->TargetState = LS_MONKEY_FORWARD;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->TargetState = LS_MONKEY_BACK;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->TargetState = LS_MONKEY_TURN_LEFT;
			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->TargetState = LS_MONKEY_TURN_RIGHT;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_IDLE (75)
// Control:		lara_as_monkey_idle()
void lara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot;
	item->VerticalVelocity = 0;
	item->Airborne = false;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);
	
	// HACK: This prevents ShiftItem() from causing an instantaneous snap and interfering with DoLaraMonkeyStep() when going down a step. @Sezz 2022.01.28
	if (coll->Shift.y >= 0 && coll->Shift.y <= CLICK(1.25f))
		coll->Shift.y = 0;
	ShiftItem(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_FORWARD (76)
// Collision:	lara_col_monkey_forward()
void lara_as_monkey_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		// TODO
		/*if (TrInput & IN_ROLL | IN_BACK)
		{
			item->TargetState = LS_MONKEY_TURN_180;
			return;
		}*/

		if (TrInput & IN_FORWARD)
		{
			item->TargetState = LS_MONKEY_FORWARD;
			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_FORWARD (76)
// Control:		lara_as_monkey_forward()
void lara_col_monkey_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_BACK (163)
// Collision:	lara_col_monkey_back()
void lara_as_monkey_back(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		if (TrInput & IN_BACK)
		{
			item->TargetState = LS_MONKEY_BACK;
			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_BACK (163)
// Control:		lara_as_monkey_back()
void lara_col_monkey_back(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_SHIMMY_LEFT (77)
// Collision:	lara_col_monkey_shimmy_left()
void lara_as_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		if (TrInput & IN_LSTEP)
		{
			item->TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_SHIMMY_LEFT (7)
// Control:		lara_as_monkey_shimmy_left()
void lara_col_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(0.5f);
	coll->Setup.UpperCeilingBound = -CLICK(0.5f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_SHIMMY_RIGHT (78)
// Collision:	lara_col_monkey_shimmy_right()
void lara_as_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		if (TrInput & IN_RSTEP)
		{
			item->TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_SHIMMY_RIGHT (78)
// Control:		lara_as_monkey_shimmy_right()
void lara_col_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(0.5f);
	coll->Setup.UpperCeilingBound = -CLICK(0.5f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_TURN_180 (79)
// Collision:	lara_as_monkey_turn_180()
void lara_as_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	item->TargetState = LS_MONKEY_IDLE;
}

// State:		LS_MONKEY_TURN_180 (79)
// Control:		lara_as_monkey_turn_180()
void lara_col_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Collision:	lara_col_monkey_turn_left()
void lara_as_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->TargetState = LS_MONKEY_FORWARD;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->TargetState = LS_MONKEY_BACK;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->TargetState = LS_MONKEY_TURN_LEFT;

			info->Control.TurnRate -= LARA_TURN_RATE;
			if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
				info->Control.TurnRate = -LARA_SLOW_TURN_MAX;

			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Control:		lara_as_monkey_turn_left()
void lara_col_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Collision:	lara_col_monkey_turn_right()
void lara_as_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	info->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_ACTION && info->Control.CanMonkeySwing)
	{
		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->TargetState = LS_MONKEY_FORWARD;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->TargetState = LS_MONKEY_BACK;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		if (TrInput & IN_RIGHT)
		{
			item->TargetState = LS_MONKEY_TURN_RIGHT;

			info->Control.TurnRate += LARA_TURN_RATE;
			if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
				info->Control.TurnRate = LARA_SLOW_TURN_MAX;

			return;
		}

		item->TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Control:		lara_as_monkey_turn_right()
void lara_col_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}
