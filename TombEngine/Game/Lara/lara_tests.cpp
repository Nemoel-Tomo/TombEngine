#include "framework.h"
#include "Game/Lara/lara_tests.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_monkey.h"
#include "Renderer/Renderer11.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

// Test if a ledge in front of item is valid to climb.
bool TestValidLedge(ItemInfo* item, CollisionInfo* coll, bool ignoreHeadroom, bool heightLimit)
{
	// Determine probe base left/right points
	int xl = sin(coll->NearestLedgeAngle - Angle::DegToRad(90.0f)) * coll->Setup.Radius;
	int zl = cos(coll->NearestLedgeAngle - Angle::DegToRad(90.0f)) * coll->Setup.Radius;
	int xr = sin(coll->NearestLedgeAngle + Angle::DegToRad(90.0f)) * coll->Setup.Radius;
	int zr = cos(coll->NearestLedgeAngle + Angle::DegToRad(90.0f)) * coll->Setup.Radius;

	// Determine probe top point
	int y = item->Pose.Position.y - coll->Setup.Height;

	// Get frontal collision data
	auto frontLeft  = GetCollision(item->Pose.Position.x + xl, y, item->Pose.Position.z + zl, GetRoom(item->Location, item->Pose.Position.x, y, item->Pose.Position.z).roomNumber);
	auto frontRight = GetCollision(item->Pose.Position.x + xr, y, item->Pose.Position.z + zr, GetRoom(item->Location, item->Pose.Position.x, y, item->Pose.Position.z).roomNumber);

	// If any of the frontal collision results intersects item bounds, return false, because there is material intersection.
	// This check helps to filter out cases when Lara is formally facing corner but ledge check returns true because probe distance is fixed.
	if (frontLeft.Position.Floor < (item->Pose.Position.y - CLICK(0.5f)) || frontRight.Position.Floor < (item->Pose.Position.y - CLICK(0.5f)))
		return false;
	if (frontLeft.Position.Ceiling > (item->Pose.Position.y - coll->Setup.Height) || frontRight.Position.Ceiling > (item->Pose.Position.y - coll->Setup.Height))
		return false;

	//g_Renderer.AddDebugSphere(Vector3(item->pos.Position.x + xl, left, item->pos.Position.z + zl), 64, Vector4::One, RENDERER_DEBUG_PAGE::LOGIC_STATS);
	//g_Renderer.AddDebugSphere(Vector3(item->pos.Position.x + xr, right, item->pos.Position.z + zr), 64, Vector4::One, RENDERER_DEBUG_PAGE::LOGIC_STATS);
	
	// Determine ledge probe embed offset.
	// We use 0.2f radius extents here for two purposes. First - we can't guarantee that shifts weren't already applied
	// and misfire may occur. Second - it guarantees that Lara won't land on a very thin edge of diagonal geometry.
	int xf = sin(coll->NearestLedgeAngle) * (coll->Setup.Radius * 1.2f);
	int zf = cos(coll->NearestLedgeAngle) * (coll->Setup.Radius * 1.2f);

	// Get floor heights at both points
	auto left = GetCollision(item->Pose.Position.x + xf + xl, y, item->Pose.Position.z + zf + zl, GetRoom(item->Location, item->Pose.Position.x, y, item->Pose.Position.z).roomNumber).Position.Floor;
	auto right = GetCollision(item->Pose.Position.x + xf + xr, y, item->Pose.Position.z + zf + zr, GetRoom(item->Location, item->Pose.Position.x, y, item->Pose.Position.z).roomNumber).Position.Floor;

	// If specified, limit vertical search zone only to nearest height
	if (heightLimit && (abs(left - y) > CLICK(0.5f) || abs(right - y) > CLICK(0.5f)))
		return false;

	// Determine allowed slope difference for a given collision radius
	float slopeDelta = ((float)STEPUP_HEIGHT / (float)SECTOR(1)) * (coll->Setup.Radius * 2);

	// Discard if there is a slope beyond tolerance delta
	if (abs(left - right) >= slopeDelta)
		return false;

	// Discard if ledge is not within distance threshold
	if (abs(coll->NearestLedgeDistance) > OFFSET_RADIUS(coll->Setup.Radius))
		return false;

	// Discard if ledge is not within angle threshold
	if (!TestValidLedgeAngle(item, coll))
		return false;
	
	if (!ignoreHeadroom)
	{
		auto headroom = (coll->Front.Floor + coll->Setup.Height) - coll->Middle.Ceiling;
		if (headroom < CLICK(1))
			return false;
	}
	
	return true;
}

bool TestValidLedgeAngle(ItemInfo* item, CollisionInfo* coll)
{
	return (abs(Angle::Normalize(coll->NearestLedgeAngle - item->Pose.Orientation.GetY())) <= LARA_GRAB_THRESHOLD);
}

bool TestLaraHang(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	auto angle = lara->Control.MoveAngle;

	auto climbShift = 0;
	if (lara->Control.MoveAngle == Angle::Normalize(item->Pose.Orientation.GetY() - Angle::DegToRad(90.0f)))
		climbShift = -coll->Setup.Radius;
	else if (lara->Control.MoveAngle == Angle::Normalize(item->Pose.Orientation.GetY() + Angle::DegToRad(90.0f)))
		climbShift = coll->Setup.Radius;

	// Temporarily move item a bit closer to the wall to get more precise coll results
	auto oldPose = item->Pose;

	TranslateItem(item, item->Pose.Orientation.GetY(), coll->Setup.Radius * 0.5f);

	// Get height difference with side spaces (left or right, depending on movement direction)
	auto hdif = LaraFloorFront(item, lara->Control.MoveAngle, coll->Setup.Radius * 1.4f);

	// Set stopped flag, if floor height is above footspace which is step size
	auto stopped = hdif < CLICK(0.5f);

	// Set stopped flag, if ceiling height is below headspace which is step size
	if (LaraCeilingFront(item, lara->Control.MoveAngle, coll->Setup.Radius * 1.5f, 0) > -950)
		stopped = true;

	// Backup item pos to restore it after coll tests
	item->Pose = oldPose;

	// Setup coll lara
	lara->Control.MoveAngle = item->Pose.Orientation.GetY();
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;

	// When Lara is about to move, use larger embed offset for stabilizing diagonal shimmying)
	int embedOffset = 4;
	if (TrInput & (IN_LEFT | IN_RIGHT))
		embedOffset = 16;

	TranslateItem(item, item->Pose.Orientation.GetY(), embedOffset);

	GetCollisionInfo(coll, item);

	bool result = false;

	// Ladder case.
	if (lara->Control.CanClimbLadder)
	{
		if (TrInput & IN_ACTION)
		{
			lara->Control.MoveAngle = angle;

			if (!TestLaraHangOnClimbableWall(item, coll))
			{
				if (item->Animation.AnimNumber != LA_LADDER_TO_HANG_RIGHT &&
					item->Animation.AnimNumber != LA_LADDER_TO_HANG_LEFT)
				{
					LaraSnapToEdgeOfBlock(item, coll, GetQuadrant(item->Pose.Orientation.GetY()));
					item->Pose.Position.y = coll->Setup.OldPosition.y;
					SetAnimation(item, LA_REACH_TO_HANG, 21);
				}

				result = true;
			}
			else
			{
				if (((item->Animation.AnimNumber == LA_REACH_TO_HANG && item->Animation.FrameNumber == GetFrameNumber(item, 21)) ||
						item->Animation.AnimNumber == LA_HANG_IDLE) &&
					TestLaraClimbIdle(item, coll))
				{
					item->Animation.TargetState = LS_LADDER_IDLE;
				}
			}
		}
	}
	// Normal case.
	else
	{
		if (TrInput & IN_ACTION && coll->Front.Floor <= 0)
		{
			if (stopped && hdif > 0 && climbShift != 0 &&
				((climbShift > 0) == (coll->MiddleLeft.Floor > coll->MiddleRight.Floor)))
			{
				stopped = false;
			}

			auto verticalShift = coll->Front.Floor - GetBoundsAccurate(item)->Y1;
			auto x = item->Pose.Position.x;
			auto z = item->Pose.Position.z;

			lara->Control.MoveAngle = angle;

			if (climbShift != 0)
			{
				float sinMoveAngle = sin(lara->Control.MoveAngle);
				float cosMoveAngle = cos(lara->Control.MoveAngle);
				auto testShift = Vector2(sinMoveAngle * climbShift, cosMoveAngle * climbShift);

				x += testShift.x;
				z += testShift.y;
			}

			if ((256 << GetQuadrant(item->Pose.Orientation.GetY())) & GetClimbFlags(x, item->Pose.Position.y, z, item->RoomNumber))
			{
				if (!TestLaraHangOnClimbableWall(item, coll))
					verticalShift = 0; // Ignore vertical shift if ladder is encountered next block
			}
			else if (!TestValidLedge(item, coll, true))
			{
				if ((climbShift < 0 && coll->FrontLeft.Floor != coll->Front.Floor) ||
					(climbShift > 0 && coll->FrontRight.Floor != coll->Front.Floor))
				{
					stopped = true;
				}
			}

			if (!stopped &&
				coll->Middle.Ceiling < 0 && coll->CollisionType == CT_FRONT && !coll->HitStatic &&
				abs(verticalShift) < SLOPE_DIFFERENCE && TestValidLedgeAngle(item, coll))
			{
				if (item->Animation.Velocity != 0)
					SnapItemToLedge(item, coll);

				item->Pose.Position.y += verticalShift;
			}
			else
			{
				item->Pose.Position = coll->Setup.OldPosition;

				if (item->Animation.ActiveState == LS_SHIMMY_LEFT ||
					item->Animation.ActiveState == LS_SHIMMY_RIGHT)
				{
					SetAnimation(item, LA_REACH_TO_HANG, 21);
				}

				result = true;
			}
		}
	}

	return result;
}

bool DoLaraLedgeHang(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) || lara->Control.HandStatus != HandStatus::Free || coll->HitStatic)
		return false;

	// Grab monkey swing.
	if (TestLaraMonkeyGrab(item, coll))
	{
		if (item->Animation.ActiveState == LS_JUMP_UP)
			SetAnimation(item, LA_JUMP_UP_TO_MONKEY);
		else
			SetAnimation(item, LA_REACH_TO_MONKEY);

		SetLaraHang(item);
		item->Pose.Position.y += coll->Middle.Ceiling + (LARA_HEIGHT_MONKEY - coll->Setup.Height);
		return true;
	}

	// Grab ledge.
	auto ledgeHangResult = TestLaraLedgeHang(item, coll);
	if (ledgeHangResult.Success)
	{
		//TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 31, 0, 100, 0);
		//return true;

		if (item->Animation.ActiveState == LS_JUMP_UP)
			SetAnimation(item, LA_REACH_TO_HANG, 12);
		else if (TestHangSwingIn(item, coll))
			SetAnimation(item, LA_REACH_TO_HANG_OSCILLATE);
		else
			SetAnimation(item, LA_REACH_TO_HANG);

		SetLaraHang(item);
		SnapItemToLedge(item, coll);
		ResetLaraFlex(item);
		item->Pose.Position.y = ledgeHangResult.Height + LARA_HEIGHT_STRETCH - CLICK(0.6f);
		return true;
	}

	// Grab ladder.
	bool ladder = TestLaraHangOnClimbableWall(item, coll);
	if (ladder)
	{
		if (item->Animation.ActiveState == LS_JUMP_UP)
			SetAnimation(item, LA_REACH_TO_HANG, 12);
		else if (TestHangSwingIn(item, coll))
			SetAnimation(item, LA_REACH_TO_HANG_OSCILLATE);
		else
			SetAnimation(item, LA_REACH_TO_HANG);

		SetLaraHang(item);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		ResetLaraFlex(item);
		item->Pose.Position.y -= CLICK(0.6f);
		return true;
	}

	return false;
}

bool TestLaraClimbIdle(ItemInfo* item, CollisionInfo* coll)
{
	int shiftRight, shiftLeft;

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -700, CLICK(2), &shiftRight) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -700, CLICK(2), &shiftLeft) != 1)
		return false;

	if (shiftRight)
	{
		if (shiftLeft)
		{
			if (shiftRight < 0 != shiftLeft < 0)
				return false;

			if ((shiftRight < 0 && shiftLeft < shiftRight) ||
				(shiftRight > 0 && shiftLeft > shiftRight))
			{
				item->Pose.Position.y += shiftLeft;
				return true;
			}
		}

		item->Pose.Position.y += shiftRight;
	}
	else if (shiftLeft)
		item->Pose.Position.y += shiftLeft;

	return true;
}

bool TestLaraHangOnClimbableWall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);
	int shift, result;

	if (!lara->Control.CanClimbLadder)
		return false;

	if (item->Animation.VerticalVelocity < 0)
		return false;

	// HACK: Climb wall tests are highly fragile and depend on quadrant shifts.
	// Until climb wall tests are fully refactored, we need to recalculate CollisionInfo.

	auto coll2 = *coll;
	coll2.Setup.Mode = CollisionProbeMode::Quadrants;
	GetCollisionInfo(&coll2, item);

	switch (GetQuadrant(item->Pose.Orientation.GetY()))
	{
	case NORTH:
	case SOUTH:
		item->Pose.Position.z += coll2.Shift.z;
		break;

	case EAST:
	case WEST:
		item->Pose.Position.x += coll2.Shift.x;
		break;

	default:
		break;
	}

	auto bounds = GetBoundsAccurate(item);

	if (lara->Control.MoveAngle != item->Pose.Orientation.GetY())
	{
		int l = LaraCeilingFront(item, item->Pose.Orientation.GetY(), 0, 0);
		int r = LaraCeilingFront(item, lara->Control.MoveAngle, CLICK(0.5f), 0);

		if (abs(l - r) > SLOPE_DIFFERENCE)
			return false;
	}

	if (LaraTestClimbPos(item, LARA_RADIUS, LARA_RADIUS, bounds->Y1, bounds->Y2 - bounds->Y1, &shift) &&
		LaraTestClimbPos(item, LARA_RADIUS, -LARA_RADIUS, bounds->Y1, bounds->Y2 - bounds->Y1, &shift))
	{
		result = LaraTestClimbPos(item, LARA_RADIUS, 0, bounds->Y1, bounds->Y2 - bounds->Y1, &shift);
		if (result)
		{
			if (result != 1)
				item->Pose.Position.y += shift;

			return true;
		}
	}

	return false;
}

bool TestLaraValidHangPosition(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Get incoming ledge height and own Lara's upper bound.
	// First one will be negative while first one is positive.
	// Difference between two indicates difference in height between ledges.
	auto frontFloor = GetCollision(item, lara->Control.MoveAngle, coll->Setup.Radius + CLICK(0.5f), -LARA_HEIGHT).Position.Floor;
	int laraUpperBound = item->Pose.Position.y - coll->Setup.Height;

	// If difference is above 1/2 click, return false (ledge is out of reach).
	if (abs(frontFloor - laraUpperBound) > CLICK(0.5f))
		return false;

	// Embed Lara into wall to make collision test succeed
	item->Pose.Position.x += sin(item->Pose.Orientation.GetY()) * 8;
	item->Pose.Position.z += cos(item->Pose.Orientation.GetY()) * 8;

	// Setup new GCI call
	lara->Control.MoveAngle = item->Pose.Orientation.GetY();
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -CLICK(2);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Mode = CollisionProbeMode::FreeFlat;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;

	GetCollisionInfo(coll, item);

	// Filter out narrow ceiling spaces, no collision cases and statics in front.
	if (coll->Middle.Ceiling >= 0 || coll->CollisionType != CT_FRONT || coll->HitStatic)
		return false;

	// Finally, do ordinary ledge checks (slope difference etc.)
	return TestValidLedge(item, coll);
}

CornerType TestLaraHangCorner(ItemInfo* item, CollisionInfo* coll, float testAngle)
{
	auto* lara = GetLaraInfo(item);

	// Lara isn't in stop state yet, bypass test
	if (item->Animation.AnimNumber != LA_REACH_TO_HANG && item->Animation.AnimNumber != LA_HANG_IDLE)
		return CornerType::None;

	// Static is in the way, bypass test
	if (coll->HitStatic)
		return CornerType::None;

	// INNER CORNER TESTS

	// Backup old Lara position and frontal collision
	auto oldPos = item->Pose;
	auto oldMoveAngle = lara->Control.MoveAngle;

	auto cornerResult = TestItemAtNextCornerPosition(item, coll, testAngle, false);

	// Do further testing only if test angle is equal to resulting edge angle
	if (cornerResult.Success)
	{
		// Get bounding box height for further ledge height calculations
		auto bounds = GetBoundsAccurate(item);

		// Store next position
		item->Pose = cornerResult.RealPositionResult;
		lara->NextCornerPos.Position.x = item->Pose.Position.x;
		lara->NextCornerPos.Position.y = GetCollision(item, item->Pose.Orientation.GetY(), coll->Setup.Radius * 2, -(abs(bounds->Y1) + LARA_HEADROOM)).Position.Floor + abs(bounds->Y1);
		lara->NextCornerPos.Position.z = item->Pose.Position.z;
		lara->NextCornerPos.Orientation.SetY(item->Pose.Orientation.GetY());
		lara->Control.MoveAngle = item->Pose.Orientation.GetY();

		item->Pose = cornerResult.ProbeResult;
		auto result = TestLaraValidHangPosition(item, coll);

		// Restore original item positions
		item->Pose = oldPos;
		lara->Control.MoveAngle = oldMoveAngle;

		if (result)
			return CornerType::Inner;

		if (lara->Control.CanClimbLadder)
		{
			auto& angleSet = testAngle > 0 ? LeftExtRightIntTab : LeftIntRightExtTab;
			if (GetClimbFlags(lara->NextCornerPos.Position.x, item->Pose.Position.y, lara->NextCornerPos.Position.z, item->RoomNumber) & (short)angleSet[GetQuadrant(item->Pose.Orientation.GetY())])
			{
				lara->NextCornerPos.Position.y = item->Pose.Position.y; // Restore original Y pos for ladder tests because we don't snap to ledge height in such case.
				return CornerType::Inner;
			}
		}
	}

	// Restore original item positions
	item->Pose = oldPos;
	lara->Control.MoveAngle = oldMoveAngle;

	// OUTER CORNER TESTS

	// Test if there's a material obstacles blocking outer corner pathway
	if ((LaraFloorFront(item, item->Pose.Orientation.GetY() + Angle::DegToRad(testAngle), coll->Setup.Radius + CLICK(1)) < 0) ||
		(LaraCeilingFront(item, item->Pose.Orientation.GetY() + Angle::DegToRad(testAngle), coll->Setup.Radius + CLICK(1), coll->Setup.Height) > 0))
		return CornerType::None;

	// Last chance for possible diagonal vs. non-diagonal cases: ray test
	if (!LaraPositionOnLOS(item, item->Pose.Orientation.GetY() + Angle::DegToRad(testAngle), coll->Setup.Radius + CLICK(1)))
		return CornerType::None;

	cornerResult = TestItemAtNextCornerPosition(item, coll, testAngle, true);

	// Additional test if there's a material obstacles blocking outer corner pathway
	if ((LaraFloorFront(item, item->Pose.Orientation.GetY(), 0) < 0) ||
		(LaraCeilingFront(item, item->Pose.Orientation.GetY(), 0, coll->Setup.Height) > 0))
		cornerResult.Success = false;

	// Do further testing only if test angle is equal to resulting edge angle
	if (cornerResult.Success)
	{
		// Get bounding box height for further ledge height calculations
		auto bounds = GetBoundsAccurate(item);

		// Store next position
		item->Pose = cornerResult.RealPositionResult;
		lara->NextCornerPos.Position.x = item->Pose.Position.x;
		lara->NextCornerPos.Position.y = GetCollision(item, item->Pose.Orientation.GetY(), coll->Setup.Radius * 2, -(abs(bounds->Y1) + LARA_HEADROOM)).Position.Floor + abs(bounds->Y1);
		lara->NextCornerPos.Position.z = item->Pose.Position.z;
		lara->NextCornerPos.Orientation.SetY(item->Pose.Orientation.GetY());
		lara->Control.MoveAngle = item->Pose.Orientation.GetY();

		item->Pose = cornerResult.ProbeResult;
		auto result = TestLaraValidHangPosition(item, coll);

		// Restore original item positions
		item->Pose = oldPos;
		lara->Control.MoveAngle = oldMoveAngle;

		if (result)
			return CornerType::Outer;

		if (lara->Control.CanClimbLadder)
		{
			auto& angleSet = testAngle > 0 ? LeftIntRightExtTab : LeftExtRightIntTab;
			if (GetClimbFlags(lara->NextCornerPos.Position.x, item->Pose.Position.y, lara->NextCornerPos.Position.z, item->RoomNumber) & (short)angleSet[GetQuadrant(item->Pose.Orientation.GetY())])
			{
				lara->NextCornerPos.Position.y = item->Pose.Position.y; // Restore original Y pos for ladder tests because we don't snap to ledge height in such case.
				return CornerType::Outer;
			}
		}
	}

	// Restore original item positions
	item->Pose = oldPos;
	lara->Control.MoveAngle = oldMoveAngle;

	return CornerType::None;
}

CornerTestResult TestItemAtNextCornerPosition(ItemInfo* item, CollisionInfo* coll, float angle, bool outer)
{
	auto* lara = GetLaraInfo(item);

	auto result = CornerTestResult();

	// Determine real turning angle
	float turnAngle = outer ? angle : -angle;

	// Backup previous position into array
	PoseData pos[3] = { item->Pose, item->Pose, item->Pose };

	// Do a two-step rotation check. First step is real resulting position, and second step is probing
	// position. We need this because checking at exact ending position does not always return
	// correct results with nearest ledge angle.

	for (int i = 0; i < 2; i++)
	{
		// Determine collision box anchor point and rotate collision box around this anchor point.
		// Then determine new test position from centerpoint of new collision box position.

		// Push back item a bit to compensate for possible edge ledge cases
		pos[i].Position.x -= round((coll->Setup.Radius * (outer ? -0.2f : 0.2f)) * sin(pos[i].Orientation.GetY()));
		pos[i].Position.z -= round((coll->Setup.Radius * (outer ? -0.2f : 0.2f)) * cos(pos[i].Orientation.GetY()));

		// Move item at the distance of full collision diameter plus half-radius margin to movement direction 
		pos[i].Position.x += round((coll->Setup.Radius * (i == 0 ? 2.0f : 2.5f)) * sin(lara->Control.MoveAngle));
		pos[i].Position.z += round((coll->Setup.Radius * (i == 0 ? 2.0f : 2.5f)) * cos(lara->Control.MoveAngle));

		// Determine anchor point
		auto cX = pos[i].Position.x + round(coll->Setup.Radius * sin(pos[i].Orientation.GetY()));
		auto cZ = pos[i].Position.z + round(coll->Setup.Radius * cos(pos[i].Orientation.GetY()));
		cX += (coll->Setup.Radius * sin(pos[i].Orientation.GetY() + Angle::DegToRad(90.0f * -std::copysign(1.0f, angle))));
		cZ += (coll->Setup.Radius * cos(pos[i].Orientation.GetY() + Angle::DegToRad(90.0f * -std::copysign(1.0f, angle))));

		// Determine distance from anchor point to new item position
		auto dist = Vector2(pos[i].Position.x, pos[i].Position.z) - Vector2(cX, cZ);
		auto s = sin(Angle::DegToRad(turnAngle));
		auto c = cos(Angle::DegToRad(turnAngle));

		// Shift item to a new anchor point
		pos[i].Position.x = dist.x * c - dist.y * s + cX;
		pos[i].Position.z = dist.x * s + dist.y * c + cZ;

		// Virtually rotate item to new angle
		short newAngle = pos[i].Orientation.GetY() - Angle::DegToRad(turnAngle);
		pos[i].Orientation.SetY(newAngle);

		// Snap to nearest ledge, if any.
		item->Pose = pos[i];
		SnapItemToLedge(item, coll, item->Pose.Orientation.GetY(), true);

		// Copy resulting position to an array and restore original item position.
		pos[i] = item->Pose;
		item->Pose = pos[2];

		if (i == 1) // Both passes finished, construct the result.
		{
			result.RealPositionResult = pos[0];
			result.ProbeResult = pos[1];
			result.Success = newAngle == pos[i].Orientation.GetY();
		}
	}

	return result;
}

bool TestHangSwingIn(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item, item->Pose.Orientation.GetY(), OFFSET_RADIUS(coll->Setup.Radius));

	if ((probe.Position.Floor - y) > 0 &&
		(probe.Position.Ceiling - y) < -CLICK(1.6f) &&
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraHangSideways(ItemInfo* item, CollisionInfo* coll, float angle)
{
	auto* lara = GetLaraInfo(item);

	auto oldPos = item->Pose;

	lara->Control.MoveAngle = item->Pose.Orientation.GetY() + angle;

	static constexpr auto sidewayTestDistance = 16;
	item->Pose.Position.x += sin(lara->Control.MoveAngle) * sidewayTestDistance;
	item->Pose.Position.z += cos(lara->Control.MoveAngle) * sidewayTestDistance;

	coll->Setup.OldPosition.y = item->Pose.Position.y;

	bool res = TestLaraHang(item, coll);

	item->Pose = oldPos;

	return !res;
}

bool TestLaraWall(ItemInfo* item, int distance, int height, int side)
{
	float sinY = sin(item->Pose.Orientation.GetY());
	float cosY = cos(item->Pose.Orientation.GetY());

	auto start = GameVector(
		item->Pose.Position.x + (side * cosY),
		item->Pose.Position.y + height,
		item->Pose.Position.z + (-side * sinY),
		item->RoomNumber
	);

	auto end = GameVector(
		item->Pose.Position.x + (distance * sinY) + (side * cosY),
		item->Pose.Position.y + height,
		item->Pose.Position.z + (distance * cosY) + (-side * sinY),
		item->RoomNumber
	);

	return !LOS(&start, &end);
}

bool TestLaraFacingCorner(ItemInfo* item, float angle, int distance)
{
	float angleLeft = angle - Angle::DegToRad(15.0f);
	float angleRight = angle + Angle::DegToRad(15.0f);

	auto start = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end1 = GameVector(
		item->Pose.Position.x + distance * sin(angleLeft),
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z + distance * cos(angleLeft),
		item->RoomNumber
	);

	auto end2 = GameVector(
		item->Pose.Position.x + distance * sin(angleRight),
		item->Pose.Position.y - STEPUP_HEIGHT,
		item->Pose.Position.z + distance * cos(angleRight),
		item->RoomNumber
	);

	bool result1 = LOS(&start, &end1);
	bool result2 = LOS(&start, &end2);
	return (!result1 && !result2);
}

bool LaraPositionOnLOS(ItemInfo* item, float angle, int distance)
{
	auto start1 = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - LARA_HEADROOM,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto start2 = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y - LARA_HEIGHT + LARA_HEADROOM,
		item->Pose.Position.z,
		item->RoomNumber
	);
	
	auto end1 = GameVector(
		item->Pose.Position.x + distance * sin(angle),
		item->Pose.Position.y - LARA_HEADROOM,
		item->Pose.Position.z + distance * cos(angle),
		item->RoomNumber
	);

	auto end2 = GameVector(
		item->Pose.Position.x + distance * sin(angle),
		item->Pose.Position.y - LARA_HEIGHT + LARA_HEADROOM,
		item->Pose.Position.z + distance * cos(angle),
		item->RoomNumber
	);

	auto result1 = LOS(&start1, &end1);
	auto result2 = LOS(&start2, &end2);

	return (result1 && result2);
}

int LaraFloorFront(ItemInfo* item, float angle, int distance)
{
	return LaraCollisionFront(item, angle, distance).Position.Floor;
}

int LaraCeilingFront(ItemInfo* item, float angle, int distance, int height)
{
	return LaraCeilingCollisionFront(item, angle, distance, height).Position.Ceiling;
}

CollisionResult LaraCollisionFront(ItemInfo* item, float angle, int distance)
{
	auto probe = GetCollision(item, angle, distance, -LARA_HEIGHT);

	if (probe.Position.Floor != NO_HEIGHT)
		probe.Position.Floor -= item->Pose.Position.y;

	return probe;
}

CollisionResult LaraCeilingCollisionFront(ItemInfo* item, float angle, int distance, int height)
{
	auto probe = GetCollision(item, angle, distance, -height);

	if (probe.Position.Ceiling != NO_HEIGHT)
		probe.Position.Ceiling += height - item->Pose.Position.y;

	return probe;
}

bool TestLaraWaterStepOut(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (coll->CollisionType == CT_FRONT ||
		coll->Middle.FloorSlope ||
		coll->Middle.Floor >= 0)
	{
		return false;
	}

	if (coll->Middle.Floor >= -CLICK(0.5f))
		SetAnimation(item, LA_STAND_IDLE);
	else
	{
		SetAnimation(item, LA_ONWATER_TO_WADE_1_STEP);
		item->Animation.TargetState = LS_IDLE;
	}

	item->Pose.Position.y += coll->Middle.Floor + CLICK(2.75f) - 9;
	UpdateItemRoom(item, -(STEPUP_HEIGHT - 3));

	item->Pose.Orientation.SetX(0);
	item->Pose.Orientation.SetZ(0);
	item->Animation.Airborne = false;
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;
	lara->Control.WaterStatus = WaterStatus::Wade;

	return true;
}

bool TestLaraLadderClimbOut(ItemInfo* item, CollisionInfo* coll) // NEW function for water to ladder move
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) ||
		!lara->Control.CanClimbLadder ||
		coll->CollisionType != CT_FRONT)
	{
		return false;
	}

	if (lara->Control.HandStatus != HandStatus::Free &&
		(lara->Control.HandStatus != HandStatus::WeaponReady || lara->Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return false;
	}

	if (!TestLaraClimbIdle(item, coll))
		return false;

	float facing = item->Pose.Orientation.GetY();

	if (facing >= Angle::DegToRad(-35.0f) && facing <= Angle::DegToRad(35.0f))
		facing = 0;
	else if (facing >= Angle::DegToRad(55.0f) && facing <= Angle::DegToRad(125.0f))
		facing = Angle::DegToRad(90.0f);
	else if (facing >= Angle::DegToRad(145.0f) || facing <= Angle::DegToRad(-145.0f))
		facing = Angle::DegToRad(180.0f);
	else if (facing >= Angle::DegToRad(-125.0f) && facing <= Angle::DegToRad(-55.0f))
		facing = Angle::DegToRad(-90.0f);

	if (Angle::RadToShrt(facing) & 0x3FFF)
		return false;

	// TODO: Resolve short form angle.
	switch ((unsigned short)Angle::RadToShrt(facing / Angle::DegToRad(90.0f)))
	{
	case NORTH:
		item->Pose.Position.z = (item->Pose.Position.z | (SECTOR(1) - 1)) - LARA_RADIUS - 1;
		break;

	case EAST:
		item->Pose.Position.x = (item->Pose.Position.x | (SECTOR(1) - 1)) - LARA_RADIUS - 1;
		break;

	case SOUTH:
		item->Pose.Position.z = (item->Pose.Position.z & -SECTOR(1)) + LARA_RADIUS + 1;
		break;

	case WEST:
		item->Pose.Position.x = (item->Pose.Position.x & -SECTOR(1)) + LARA_RADIUS + 1;
		break;
	}

	SetAnimation(item, LA_ONWATER_IDLE);
	item->Animation.TargetState = LS_LADDER_IDLE;
	AnimateLara(item);

	item->Pose.Position.y -= 10; // Otherwise she falls back into the water.
	item->Pose.Orientation.Set(0.0f, facing, 0.0f);
	item->Animation.Airborne = false;
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;
	lara->Control.TurnRate.SetY();
	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.WaterStatus = WaterStatus::Dry;
	return true;
}

void TestLaraWaterDepth(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	auto probe = GetCollision(item);
	int waterDepth = GetWaterDepth(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, probe.RoomNumber);

	if (waterDepth == NO_HEIGHT)
	{
		item->Animation.VerticalVelocity = 0;
		item->Pose.Position = coll->Setup.OldPosition;
	}
	// Height check was at CLICK(2) before but changed to this 
	// because now Lara surfaces on a head level, not mid-body level.
	else if (waterDepth <= LARA_HEIGHT - LARA_HEADROOM / 2)
	{
		SetAnimation(item, LA_UNDERWATER_TO_STAND);
		item->Animation.TargetState = LS_IDLE;
		item->Pose.Position.y = probe.Position.Floor;
		item->Pose.Orientation.SetX(0);
		item->Pose.Orientation.SetZ(0);
		item->Animation.Airborne = false;
		item->Animation.Velocity = 0;
		item->Animation.VerticalVelocity = 0;
		lara->Control.WaterStatus = WaterStatus::Wade;
	}
}

#ifndef NEW_TIGHTROPE
void GetTightropeFallOff(ItemInfo* item, int regularity)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0 || item->HitStatus)
		SetAnimation(item, LA_TIGHTROPE_FALL_LEFT);

	if (!lara->Control.Tightrope.Fall && !(GetRandomControl() & regularity))
		lara->Control.Tightrope.Fall = 2 - ((GetRandomControl() & 0xF) != 0);
}
#endif

bool IsStandingWeapon(LaraWeaponType weaponType)
{
	if (weaponType == LaraWeaponType::Shotgun ||
		weaponType == LaraWeaponType::HK ||
		weaponType == LaraWeaponType::Crossbow ||
		weaponType == LaraWeaponType::Torch ||
		weaponType == LaraWeaponType::GrenadeLauncher ||
		weaponType == LaraWeaponType::HarpoonGun ||
		weaponType == LaraWeaponType::RocketLauncher||
		weaponType == LaraWeaponType::Snowmobile)
	{
		return true;
	}

	return false;
}

bool IsVaultState(LaraState state)
{
	if (state == LS_VAULT ||
		state == LS_VAULT_2_STEPS ||
		state == LS_VAULT_3_STEPS ||
		state == LS_VAULT_1_STEP_CROUCH ||
		state == LS_VAULT_2_STEPS_CROUCH ||
		state == LS_VAULT_3_STEPS_CROUCH ||
		state == LS_AUTO_JUMP)
	{
		return true;
	}

	return false;
}

bool IsJumpState(LaraState state)
{
	if (state == LS_JUMP_FORWARD ||
		state == LS_JUMP_BACK ||
		state == LS_JUMP_LEFT ||
		state == LS_JUMP_RIGHT ||
		state == LS_JUMP_UP ||
		state == LS_FALL_BACK ||
		state == LS_REACH ||
		state == LS_SWAN_DIVE ||
		state == LS_FREEFALL_DIVE ||
		state == LS_FREEFALL)
	{
		return true;
	}

	return false;
}

bool IsRunJumpQueueableState(LaraState state)
{
	if (state == LS_RUN_FORWARD ||
		state == LS_SPRINT ||
		state == LS_STEP_UP ||
		state == LS_STEP_DOWN)
	{
		return true;
	}

	return false;
}

bool IsRunJumpCountableState(LaraState state)
{
	if (state == LS_RUN_FORWARD ||
		state == LS_WALK_FORWARD ||
		state == LS_JUMP_FORWARD ||
		state == LS_SPRINT ||
		state == LS_SPRINT_DIVE)
	{
		return true;
	}

	return false;
}

bool TestLaraPose(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return false;

	if (!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		lara->Control.HandStatus == HandStatus::Free &&				// Hands are free.
		(lara->Control.Weapon.GunType != LaraWeaponType::Flare ||	// Flare is not being handled.
			lara->Flare.Life) &&
		lara->Vehicle == NO_ITEM)									// Not in a vehicle.
	{
		return true;
	}

	return false;
}

bool TestLaraKeepLow(ItemInfo* item, CollisionInfo* coll)
{
	// HACK: coll->Setup.Radius is only set to LARA_RADIUS_CRAWL
	// in the collision function, then reset by LaraAboveWater().
	// For tests called in control functions, then, it will store the wrong radius. @Sezz 2021.11.05
	int radius = (item->Animation.ActiveState == LS_CROUCH_IDLE ||
		item->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
		item->Animation.ActiveState == LS_CROUCH_TURN_RIGHT)
		? LARA_RADIUS_CRAWL : LARA_RADIUS;

	auto probeFront = GetCollision(item, item->Pose.Orientation.GetY(), radius, -coll->Setup.Height);
	auto probeBack = GetCollision(item, item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f), radius, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	if (abs(probeFront.Position.Ceiling - probeFront.Position.Floor) < LARA_HEIGHT ||	// Front is not a clamp.
		abs(probeBack.Position.Ceiling - probeBack.Position.Floor) < LARA_HEIGHT ||		// Back is not a clamp.
		abs(probeMiddle.Position.Ceiling - probeMiddle.Position.Floor) < LARA_HEIGHT)	// Middle is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraSlide(ItemInfo* item, CollisionInfo* coll)
{
	int y = item->Pose.Position.y;
	auto probe = GetCollision(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return false;

	if (abs(probe.Position.Floor - y) <= STEPUP_HEIGHT &&
		probe.Position.FloorSlope)
	{
		return true;
	}

	return false;
}

bool TestLaraLand(ItemInfo* item, CollisionInfo* coll)
{
	int heightFromFloor = GetCollision(item).Position.Floor - item->Pose.Position.y;

	if (item->Animation.Airborne &&
		item->Animation.VerticalVelocity >= 0 &&
		(heightFromFloor <= item->Animation.VerticalVelocity ||
			TestEnvironment(ENV_FLAG_SWAMP, item)))
	{
		return true;
	}
	
	return false;
}

bool TestLaraFall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item);

	if ((probe.Position.Floor - y) <= STEPUP_HEIGHT ||
		lara->Control.WaterStatus == WaterStatus::Wade)	// TODO: This causes a legacy floor snap bug when Lara wades off a ledge into a dry room. @Sezz 2021.09.26
	{
		return false;
	}

	return true;
}

bool TestLaraMonkeyGrab(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	auto probe = GetCollision(item);

	if (lara->Control.CanMonkeySwing && (probe.Position.Ceiling - y) <= CLICK(0.5f) &&
		((probe.Position.Ceiling - y) >= 0 || coll->CollisionType == CT_TOP || coll->CollisionType == CT_TOP_FRONT) &&
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_MONKEY)
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyFall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	auto probe = GetCollision(item);

	if (!lara->Control.CanMonkeySwing ||					// No monkey sector.
		(probe.Position.Ceiling - y) > CLICK(1.25f) ||		// Outside lower bound.
		(probe.Position.Ceiling - y) < -CLICK(1.25f) ||		// Outside upper bound.
		(probe.Position.CeilingSlope &&						// Is ceiling slope (if applicable).
			!g_GameFlow->HasOverhangClimb()) ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraStep(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (abs(coll->Middle.Floor) > 0 &&
		(coll->Middle.Floor <= STEPUP_HEIGHT ||					// Within lower floor bound...
			lara->Control.WaterStatus == WaterStatus::Wade) &&		// OR Lara is wading.
		coll->Middle.Floor >= -STEPUP_HEIGHT &&					// Within upper floor bound.
		coll->Middle.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraStepUp(ItemInfo* item, CollisionInfo* coll)
{
	if (coll->Middle.Floor < -CLICK(0.5f) &&	// Within lower floor bound.
		coll->Middle.Floor >= -STEPUP_HEIGHT)	// Within upper floor bound.
	{
		return true;
	}

	return false;
}

bool TestLaraStepDown(ItemInfo* item, CollisionInfo* coll)
{
	if (coll->Middle.Floor <= STEPUP_HEIGHT &&	// Within lower floor bound.
		coll->Middle.Floor > CLICK(0.5f))		// Within upper floor bound.
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll)
{
	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	auto probe = GetCollision(item);

	if ((probe.Position.Ceiling - y) <= CLICK(1.25f) &&		// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(1.25f) &&	// Within upper ceiling bound.
		probe.Position.Ceiling != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

// TODO: This function and its clone TestLaraCrawlMoveTolerance() should become obsolete with more accurate and accessible collision detection in the future.
// For now, it supersedes old probes and is used alongside COLL_INFO. @Sezz 2021.10.24
bool TestLaraMoveTolerance(ItemInfo* item, CollisionInfo* coll, MoveTestSetup testSetup, bool useCrawlSetup)
{
	// HACK: coll->Setup.Radius and coll->Setup.Height are set in
	// lara_col functions, then reset by LaraAboveWater() to defaults.
	// This means they store the wrong values for move tests called in crawl lara_as functions.
	// When states become objects, collision setup should occur at the beginning of each state, eliminating the need
	// for the useCrawlSetup argument. @Sezz 2022.03.16
	int laraRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
	int laraHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(laraRadius);
	auto probe = GetCollision(item, testSetup.Angle, distance, -laraHeight);

	bool isSlopeDown = testSetup.CheckSlopeDown ? (probe.Position.FloorSlope && probe.Position.Floor > y) : false;
	bool isSlopeUp = testSetup.CheckSlopeUp ? (probe.Position.FloorSlope && probe.Position.Floor < y) : false;
	bool isDeath = testSetup.CheckDeath ? probe.Block->Flags.Death : false;

	auto start1 = GameVector(
		item->Pose.Position.x,
		y + testSetup.UpperFloorBound - 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end1 = GameVector(
		probe.Coordinates.x,
		y + testSetup.UpperFloorBound - 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	auto start2 = GameVector(
		item->Pose.Position.x,
		y - laraHeight + 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end2 = GameVector(
		probe.Coordinates.x,
		probe.Coordinates.y + 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	// Discard walls.
	if (probe.Position.Floor == NO_HEIGHT)
		return false;

	// Check for slope or death sector (if applicable).
	if (isSlopeDown || isSlopeUp || isDeath)
		return false;

	// Assess ray/room collision.
	if (!LOS(&start1, &end1) || !LOS(&start2, &end2))
		return false;

	// Assess point/room collision.
	if ((probe.Position.Floor - y) <= testSetup.LowerFloorBound &&			// Within lower floor bound.
		(probe.Position.Floor - y) >= testSetup.UpperFloorBound &&			// Within upper floor bound.
		(probe.Position.Ceiling - y) < -laraHeight &&						// Within lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > laraHeight)	// Space is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraRunForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in run state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY(),
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, true, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWalkForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in walk state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY(),
		STEPUP_HEIGHT, -STEPUP_HEIGHT
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWalkBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in walk back state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f),
		STEPUP_HEIGHT, -STEPUP_HEIGHT
	};

	return TestLaraMoveTolerance(item, coll, testSetup);	
}

bool TestLaraRunBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in hop back state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f),
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepLeft(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in step left state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() - Angle::DegToRad(90.0f),
		CLICK(0.8f), -CLICK(0.8f)
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepRight(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in step right state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(90.0f),
		CLICK(0.8f), -CLICK(0.8f)
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWadeForwardSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in wade forward state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY(),
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraWalkBackSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using UpperFloorBound defined in walk back state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f),
		NO_LOWER_BOUND, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepLeftSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using UpperFloorBound defined in step left state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() - Angle::DegToRad(90.0f),
		NO_LOWER_BOUND, -CLICK(0.8f),
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraStepRightSwamp(ItemInfo* item, CollisionInfo* coll)
{
	// Using UpperFloorBound defined in step right state collision function.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(90.0f),
		NO_LOWER_BOUND, -CLICK(0.8f),
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testSetup);
}

bool TestLaraCrawlForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in crawl state collision functions.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY(),
		CLICK(1) - 1, -(CLICK(1) - 1)
	};

	return TestLaraMoveTolerance(item, coll, testSetup, true);
}

bool TestLaraCrawlBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperFloorBound defined in crawl state collision functions.

	MoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f),
		CLICK(1) - 1, -(CLICK(1) - 1)
	};

	return TestLaraMoveTolerance(item, coll, testSetup, true);
}

bool TestLaraCrouchRoll(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Assess water depth.
	if (lara->WaterSurfaceDist < -CLICK(1))
		return false;

	// Assess continuity of path.
	int distance = 0;
	auto probeA = GetCollision(item);
	while (distance < SECTOR(1))
	{
		distance += CLICK(1);
		auto probeB = GetCollision(item, item->Pose.Orientation.GetY(), distance, -LARA_HEIGHT_CRAWL);

		if (abs(probeA.Position.Floor - probeB.Position.Floor) > (CLICK(1) - 1) ||			// Ensure floor height difference is within a threshold.
			abs(probeB.Position.Ceiling - probeB.Position.Floor) <= LARA_HEIGHT_CRAWL ||	// Avoid clamps.
			probeB.Position.FloorSlope)														// Avoid slopes.
		{
			return false;
		}

		probeA = probeB;
	}

	return true;
}

bool TestLaraCrouch(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if ((lara->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(lara->Control.Weapon.GunType)) &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchToCrawl(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		lara->Control.HandStatus == HandStatus::Free &&				// Hands are free.
		(lara->Control.Weapon.GunType != LaraWeaponType::Flare ||	// Not handling flare. TODO: Should be allowed, but the flare animation bugs out right now. @Sezz 2022.03.18
			lara->Flare.Life))
	{
		return true;
	}

	return false;
}

bool TestLaraFastTurn(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if ((lara->Control.HandStatus == HandStatus::WeaponReady && lara->Control.Weapon.GunType != LaraWeaponType::Torch) ||
		(lara->Control.HandStatus == HandStatus::WeaponDraw && lara->Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyMoveTolerance(ItemInfo* item, CollisionInfo* coll, MonkeyMoveTestSetup testSetup)
{
	int y = item->Pose.Position.y - LARA_HEIGHT_MONKEY;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probe = GetCollision(item, testSetup.Angle, distance);

	auto start1 = GameVector(
		item->Pose.Position.x,
		y + testSetup.LowerCeilingBound + 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end1 = GameVector(
		probe.Coordinates.x,
		probe.Coordinates.y - LARA_HEIGHT_MONKEY + testSetup.LowerCeilingBound + 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	auto start2 = GameVector(
		item->Pose.Position.x,
		y + LARA_HEIGHT_MONKEY - 1,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto end2 = GameVector(
		probe.Coordinates.x,
		probe.Coordinates.y - 1,
		probe.Coordinates.z,
		item->RoomNumber
	);

	// Discard walls.
	if (probe.Position.Ceiling == NO_HEIGHT)
		return false;

	// Check for ceiling slope.
	if (probe.Position.CeilingSlope)
		return false;

	// Assess ray/room collision.
	if (!LOS(&start1, &end1) || !LOS(&start2, &end2))
		return false;

	// Assess point/room collision.
	if (probe.BottomBlock->Flags.Monkeyswing &&										// Is monkey sector.
		(probe.Position.Floor - y) > LARA_HEIGHT_MONKEY &&							// Within highest floor bound.
		(probe.Position.Ceiling - y) <= testSetup.LowerCeilingBound &&				// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= testSetup.UpperCeilingBound &&				// Within upper ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_MONKEY)	// Space is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyForward(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey forward collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY(),
		CLICK(1.25f), -CLICK(1.25f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

bool TestLaraMonkeyBack(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey back collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f),
		CLICK(1.25f), -CLICK(1.25f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

bool TestLaraMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey shimmy left collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() - Angle::DegToRad(90.0f),
		CLICK(0.5f), -CLICK(0.5f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

bool TestLaraMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll)
{
	// Using Lower/UpperCeilingBound defined in monkey shimmy right collision function.

	MonkeyMoveTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(90.0f),
		CLICK(0.5f), -CLICK(0.5f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testSetup);
}

VaultTestResult TestLaraVaultTolerance(ItemInfo* item, CollisionInfo* coll, VaultTestSetup testSetup)
{
	auto* lara = GetLaraInfo(item);

	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
	bool swampTooDeep = testSetup.CheckSwampDepth ? (isSwamp && lara->WaterSurfaceDist < -CLICK(3)) : isSwamp;
	int y = isSwamp ? item->Pose.Position.y : probeMiddle.Position.Floor; // HACK: Avoid cheese when in the midst of performing a step. Can be done better. @Sezz 2022.04.08	

	// Check swamp depth (if applicable).
	if (swampTooDeep)
		return VaultTestResult{ false };

	// NOTE: Where the point/room probe finds that
	// a) the "wall" in front is formed by a ceiling, or
	// b) the space between the floor and ceiling is a clamp (i.e. it is too narrow),
	// any potentially climbable floor in a room above will be missed. The following block considers this.
	
	// Raise y position of point/room probe by increments of CLICK(0.5f) to find potential vault ledge.
	int yOffset = testSetup.LowerFloorBound;
	while (((probeFront.Position.Ceiling - y) > -coll->Setup.Height ||								// Ceiling is below Lara's height...
			abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMin ||		// OR clamp is too small
			abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMax) &&		// OR clamp is too large (future-proofing; not possible right now).
		yOffset > (testSetup.UpperFloorBound - coll->Setup.Height))									// Offset is not too high.
	{
		probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, yOffset);
		yOffset -= std::max<int>(CLICK(0.5f), testSetup.ClampMin);
	}

	// Discard walls.
	if (probeFront.Position.Floor == NO_HEIGHT)
		return VaultTestResult{ false };

	// Assess point/room collision.
	if ((probeFront.Position.Floor - y) < testSetup.LowerFloorBound &&							// Within lower floor bound.
		(probeFront.Position.Floor - y) >= testSetup.UpperFloorBound &&							// Within upper floor bound.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMin &&	// Within clamp min.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMax &&	// Within clamp max.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= testSetup.GapMin)		// Gap is optically permissive.
	{
		return VaultTestResult{ true, probeFront.Position.Floor };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraVault2Steps(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
	// Clamp range: (-LARA_HEIGHT, -MAX_HEIGHT]

	VaultTestSetup testSetup
	{
		-STEPUP_HEIGHT, -CLICK(2.5f),
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1)
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	testResult.Height += CLICK(2);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault3Steps(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
	// Clamp range: (-LARA_HEIGHT, -MAX_HEIGHT]

	VaultTestSetup testSetup
	{
		-CLICK(2.5f), -CLICK(3.5f),
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	testResult.Height += CLICK(3);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault1StepToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (0, -STEPUP_HEIGHT]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestSetup testSetup
	{
		0, -STEPUP_HEIGHT,
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	testResult.Height += CLICK(1);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault2StepsToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestSetup testSetup
	{
		-STEPUP_HEIGHT, -CLICK(2.5f),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	testResult.Height += CLICK(2);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraVault3StepsToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestSetup testSetup
	{
		-CLICK(2.5f), -CLICK(3.5f),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	testResult.Height += CLICK(3);
	testResult.SetBusyHands = true;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = false;
	return testResult;
}

VaultTestResult TestLaraLedgeAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: (-CLICK(3.5f), -CLICK(7.5f)]
	// Clamp range: (-CLICK(0.1f), -MAX_HEIGHT]

	VaultTestSetup testSetup
	{
		-CLICK(3.5f), -CLICK(7.5f),
		CLICK(0.1f)/* TODO: Is this enough hand room?*/, -MAX_HEIGHT,
		CLICK(0.1f),
		false
	};

	auto testResult = TestLaraVaultTolerance(item, coll, testSetup);
	testResult.SetBusyHands = false;
	testResult.SnapToLedge = true;
	testResult.SetJumpVelocity = true;
	return testResult;
}

VaultTestResult TestLaraLadderAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Check ledge angle.
	if (!TestValidLedgeAngle(item, coll))
		return VaultTestResult{ false };

	if (lara->Control.CanClimbLadder &&								// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(6.5f) &&		// Within lowest middle ceiling bound. (Synced with TestLaraLadderMount())
		((probeFront.Position.Floor - y) <= -CLICK(6.5f) ||			// Floor height is appropriate, OR
			(probeFront.Position.Ceiling - y) > -CLICK(6.5f)) &&		// Ceiling height is appropriate. (Synced with TestLaraLadderMount())
		coll->NearestLedgeDistance <= coll->Setup.Radius)			// Appropriate distance from wall.
	{
		return VaultTestResult{ true, probeMiddle.Position.Ceiling, false, true, true };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraLadderMount(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Check ledge angle.
	if (!TestValidLedgeAngle(item, coll))
		return VaultTestResult{ false };

	if (lara->Control.CanClimbLadder &&							// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(4.5f) &&	// Within lower middle ceiling bound.
		(probeMiddle.Position.Ceiling - y) > -CLICK(6.5f) &&	// Within upper middle ceiling bound.
		(probeMiddle.Position.Floor - y) > -CLICK(6.5f) &&		// Within upper middle floor bound. (Synced with TestLaraAutoJump())
		(probeFront.Position.Ceiling - y) <= -CLICK(4.5f) &&	// Within lowest front ceiling bound.
		coll->NearestLedgeDistance <= coll->Setup.Radius)		// Appropriate distance from wall.
	{
		return VaultTestResult{ true, NO_HEIGHT, true, true, false };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraMonkeyAutoJump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item);

	if (lara->Control.CanMonkeySwing &&							// Monkey swing sector flag set.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_MONKEY &&	// Within lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(7))				// Within upper ceiling bound.
	{
		return VaultTestResult{ true, probe.Position.Ceiling, false, false, true };
	}

	return VaultTestResult{ false };
}

VaultTestResult TestLaraVault(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) || lara->Control.HandStatus != HandStatus::Free)
		return VaultTestResult{ false };

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && lara->WaterSurfaceDist < -CLICK(3))
		return VaultTestResult{ false };

	VaultTestResult vaultResult;

	// Attempt ledge vault.
	if (TestValidLedge(item, coll))
	{
		// Vault to crouch up one step.
		vaultResult = TestLaraVault1StepToCrouch(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_VAULT_1_STEP_CROUCH;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to stand up two steps.
		vaultResult = TestLaraVault2Steps(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_VAULT_2_STEPS;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to crouch up two steps.
		vaultResult = TestLaraVault2StepsToCrouch(item, coll);
		if (vaultResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			vaultResult.TargetState = LS_VAULT_2_STEPS_CROUCH;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to stand up three steps.
		vaultResult = TestLaraVault3Steps(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_VAULT_3_STEPS;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Vault to crouch up three steps.
		vaultResult = TestLaraVault3StepsToCrouch(item, coll);
		if (vaultResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			vaultResult.TargetState = LS_VAULT_3_STEPS_CROUCH;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}

		// Auto jump to ledge.
		vaultResult = TestLaraLedgeAutoJump(item, coll);
		if (vaultResult.Success)
		{
			vaultResult.TargetState = LS_AUTO_JUMP;
			vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
			return vaultResult;
		}
	}

	// TODO: Move ladder checks here when ladders are less prone to breaking.
	// In this case, they fail due to a reliance on ShiftItem(). @Sezz 2021.02.05

	// Auto jump to monkey swing.
	vaultResult = TestLaraMonkeyAutoJump(item, coll);
	if (vaultResult.Success &&
		g_GameFlow->HasMonkeyAutoJump())
	{
		vaultResult.TargetState = LS_AUTO_JUMP;
		vaultResult.Success = HasStateDispatch(item, vaultResult.TargetState);
		return vaultResult;
	}
	
	return VaultTestResult{ false };
}

// Temporary solution to ladder mounts until ladders stop breaking whenever anyone tries to do anything with them. @Sezz 2022.02.05
bool TestAndDoLaraLadderClimb(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_FORWARD) || lara->Control.HandStatus != HandStatus::Free)
		return false;

	if (TestEnvironment(ENV_FLAG_SWAMP, item) && lara->WaterSurfaceDist < -CLICK(3))
		return false;

	// Auto jump to ladder.
	auto vaultResult = TestLaraLadderAutoJump(item, coll);
	if (vaultResult.Success)
	{
		// TODO: Somehow harmonise CalculatedJumpVelocity to work for both ledge and ladder auto jumps, because otherwise there will be a need for an odd workaround in the future.
		lara->Control.CalculatedJumpVelocity = -3 - sqrt(-9600 - 12 * std::max((vaultResult.Height - item->Pose.Position.y + CLICK(0.2f)), -CLICK(7.1f)));
		item->Animation.AnimNumber = LA_STAND_SOLID;
		item->Animation.FrameNumber = GetFrameNumber(item, 0);
		item->Animation.TargetState = LS_JUMP_UP;
		item->Animation.ActiveState = LS_IDLE;
		lara->Control.TurnRate.SetY();

		ShiftItem(item, coll);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		lara->TargetOrientation = EulerAngles(0, item->Pose.Orientation.GetY(), 0);
		AnimateLara(item);

		return true;
	}

	// Mount ladder.
	vaultResult = TestLaraLadderMount(item, coll);
	if (vaultResult.Success &&
		TestLaraClimbIdle(item, coll))
	{
		item->Animation.AnimNumber = LA_STAND_SOLID;
		item->Animation.FrameNumber = GetFrameNumber(item, 0);
		item->Animation.TargetState = LS_LADDER_IDLE;
		item->Animation.ActiveState = LS_IDLE;
		lara->Control.HandStatus = HandStatus::Busy;
		lara->Control.TurnRate.SetY();

		ShiftItem(item, coll);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		AnimateLara(item);

		return true;
	}

	return false;
}

CrawlVaultTestResult TestLaraCrawlVaultTolerance(ItemInfo* item, CollisionInfo* coll, CrawlVaultTestSetup testSetup)
{
	int y = item->Pose.Position.y;
	auto probeA = GetCollision(item, item->Pose.Orientation.GetY(), testSetup.CrossDist, -LARA_HEIGHT_CRAWL);	// Crossing.
	auto probeB = GetCollision(item, item->Pose.Orientation.GetY(), testSetup.DestDist, -LARA_HEIGHT_CRAWL);	// Approximate destination.
	auto probeMiddle = GetCollision(item);

	bool isSlope = testSetup.CheckSlope ? probeB.Position.FloorSlope : false;
	bool isDeath = testSetup.CheckDeath ? probeB.Block->Flags.Death : false;

	// Discard walls.
	if (probeA.Position.Floor == NO_HEIGHT || probeB.Position.Floor == NO_HEIGHT)
		return CrawlVaultTestResult{ false };

	// Check for slope or death sector (if applicable).
	if (isSlope || isDeath)
		return CrawlVaultTestResult{ false };

	// Assess point/room collision.
	if ((probeA.Position.Floor - y) <= testSetup.LowerFloorBound &&							// Within lower floor bound.
		(probeA.Position.Floor - y) >= testSetup.UpperFloorBound &&							// Within upper floor bound.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > testSetup.ClampMin &&		// Crossing clamp limit.
		abs(probeB.Position.Ceiling - probeB.Position.Floor) > testSetup.ClampMin &&		// Destination clamp limit.
		abs(probeMiddle.Position.Ceiling - probeA.Position.Floor) >= testSetup.GapMin &&	// Gap is optically permissive (going up).
		abs(probeA.Position.Ceiling - probeMiddle.Position.Floor) >= testSetup.GapMin &&	// Gap is optically permissive (going down).
		abs(probeA.Position.Floor - probeB.Position.Floor) <= testSetup.FloorBound &&		// Crossing/destination floor height difference suggests continuous crawl surface.
		(probeA.Position.Ceiling - y) < -testSetup.GapMin)									// Ceiling height is permissive.
	{
		return CrawlVaultTestResult{ true };
	}

	return CrawlVaultTestResult{ false };
}

CrawlVaultTestResult TestLaraCrawlUpStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [-CLICK(1), -STEPUP_HEIGHT]

	CrawlVaultTestSetup testSetup
	{
		-CLICK(1), -STEPUP_HEIGHT,
		LARA_HEIGHT_CRAWL,
		CLICK(0.6f),
		CLICK(1.2f),
		CLICK(2),
		CLICK(1) - 1
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlDownStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [STEPUP_HEIGHT, CLICK(1)]

	CrawlVaultTestSetup testSetup
	{
		STEPUP_HEIGHT, CLICK(1),
		LARA_HEIGHT_CRAWL,
		CLICK(0.6f),
		CLICK(1.2f),
		CLICK(2),
		CLICK(1) - 1
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlExitDownStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [STEPUP_HEIGHT, CLICK(1)]

	CrawlVaultTestSetup testSetup
	{
		STEPUP_HEIGHT, CLICK(1),
		LARA_HEIGHT,
		CLICK(1.25f),
		CLICK(1.2f),
		CLICK(1.5f),
		-MAX_HEIGHT,
		false, false
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlExitJump(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [NO_LOWER_BOUND, STEPUP_HEIGHT)

	CrawlVaultTestSetup testSetup
	{
		NO_LOWER_BOUND, STEPUP_HEIGHT + 1,
		LARA_HEIGHT,
		CLICK(1.25f),
		CLICK(1.2f),
		CLICK(1.5f),
		NO_LOWER_BOUND,
		false, false
	};

	return TestLaraCrawlVaultTolerance(item, coll, testSetup);
}

CrawlVaultTestResult TestLaraCrawlVault(ItemInfo* item, CollisionInfo* coll)
{
	if (!(TrInput & (IN_ACTION | IN_JUMP)))
		return CrawlVaultTestResult{ false };

	// Crawl vault exit down 1 step.
	auto crawlVaultResult = TestLaraCrawlExitDownStep(item, coll);
	if (crawlVaultResult.Success)
	{
		if (TrInput & IN_CROUCH && TestLaraCrawlDownStep(item, coll).Success)
			crawlVaultResult.TargetState = LS_CRAWL_STEP_DOWN;
		else [[likely]]
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_STEP_DOWN;

		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault exit jump.
	crawlVaultResult = TestLaraCrawlExitJump(item, coll);
	if (crawlVaultResult.Success)
	{
		if (TrInput & IN_WALK)
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_FLIP;
		else [[likely]]
			crawlVaultResult.TargetState = LS_CRAWL_EXIT_JUMP;

		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault up 1 step.
	crawlVaultResult = TestLaraCrawlUpStep(item, coll);
	if (crawlVaultResult.Success)
	{
		crawlVaultResult.TargetState = LS_CRAWL_STEP_UP;
		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	// Crawl vault down 1 step.
	crawlVaultResult = TestLaraCrawlDownStep(item, coll);
	if (crawlVaultResult.Success)
	{
		crawlVaultResult.TargetState = LS_CRAWL_STEP_DOWN;
		crawlVaultResult.Success = HasStateDispatch(item, crawlVaultResult.TargetState);
		return crawlVaultResult;
	}

	return CrawlVaultTestResult{ false };
}

bool TestLaraCrawlToHang(ItemInfo* item, CollisionInfo* coll)
{
	int y = item->Pose.Position.y;
	int distance = CLICK(1.2f);
	auto probe = GetCollision(item, item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f), distance, -LARA_HEIGHT_CRAWL);

	bool objectCollided = TestLaraObjectCollision(item, item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f), CLICK(1.2f), -LARA_HEIGHT_CRAWL);

	if (!objectCollided &&										// No obstruction.
		(probe.Position.Floor - y) >= LARA_HEIGHT_STRETCH &&	// Highest floor bound.
		(probe.Position.Ceiling - y) <= -CLICK(0.75f) &&		// Gap is optically permissive.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

WaterClimbOutTestResult TestLaraWaterClimbOutTolerance(ItemInfo* item, CollisionInfo* coll, WaterClimbOutTestSetup testSetup)
{
	int y = item->Pose.Position.y;
	int distance = OFFSET_RADIUS(coll->Setup.Radius);
	auto probeFront = GetCollision(item, coll->NearestLedgeAngle, distance, -(coll->Setup.Height + CLICK(1)));
	auto probeMiddle = GetCollision(item);

	// Assess vault candidate location.
	if ((probeFront.Position.Floor - y) <= testSetup.LowerFloorBound &&							// Within lower floor bound.
		(probeFront.Position.Floor - y) > testSetup.UpperFloorBound &&	//check						// Within upper floor bound.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMin &&	// Within clamp min.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMax &&	// Within clamp max.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= testSetup.GapMin)		// Gap is optically permissive.
	{
		return WaterClimbOutTestResult{ true, probeFront.Position.Floor };
	}

	return WaterClimbOutTestResult{ false };
}

WaterClimbOutTestResult TestLaraWaterClimbOutDownStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [CLICK(1.25f) - 4, CLICK(0.5f))
	// Clamp range: (LARA_HEIGHT, -MAX_HEIGHT]

	WaterClimbOutTestSetup testSetup
	{
		CLICK(1.25f) - 4, CLICK(0.5f),
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraWaterClimbOutTolerance(item, coll, testSetup);
	testResult.Height -= CLICK(1);
	return testResult;
}

WaterClimbOutTestResult TestLaraWaterClimbOutDownStepToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [CLICK(1.25f) - 4, CLICK(0.5f))
	// Clamp range: (LARA_HEIGHT_CRAWL, LARA_HEIGHT]

	WaterClimbOutTestSetup testSetup
	{
		CLICK(1.25f) - 4, CLICK(0.5f),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraWaterClimbOutTolerance(item, coll, testSetup);
	testResult.Height -= CLICK(1);
	return testResult;
}

WaterClimbOutTestResult TestLaraWaterClimbOutFlatStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [CLICK(0.5f), -CLICK(1))
	// Clamp range: (LARA_HEIGHT, -MAX_HEIGHT]

	WaterClimbOutTestSetup testSetup
	{
		CLICK(0.5f), -CLICK(1),
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1),
	};

	return TestLaraWaterClimbOutTolerance(item, coll, testSetup);
}

WaterClimbOutTestResult TestLaraWaterClimbOutFlatStepToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [CLICK(0.5f), -CLICK(1))
	// Clamp range: (LARA_HEIGHT_CRAWL, LARA_HEIGHT]

	WaterClimbOutTestSetup testSetup
	{
		CLICK(0.5f), -CLICK(1),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	return TestLaraWaterClimbOutTolerance(item, coll, testSetup);
}

WaterClimbOutTestResult TestLaraWaterClimbOutUpStep(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [-CLICK(1), -CLICK(2))
	// Clamp range: (LARA_HEIGHT, -MAX_HEIGHT]

	WaterClimbOutTestSetup testSetup
	{
		-CLICK(1), -CLICK(2),
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraWaterClimbOutTolerance(item, coll, testSetup);
	testResult.Height += CLICK(1);
	return testResult;
}

WaterClimbOutTestResult TestLaraWaterClimbOutUpStepToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	// Floor range: [-CLICK(1), -CLICK(2))
	// Clamp range: (LARA_HEIGHT_CRAWL, LARA_HEIGHT]

	WaterClimbOutTestSetup testSetup
	{
		-CLICK(1), -CLICK(2),
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
	};

	auto testResult = TestLaraWaterClimbOutTolerance(item, coll, testSetup);
	testResult.Height += CLICK(1);
	return testResult;
}

WaterClimbOutTestResult TestLaraWaterClimbOut(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!(TrInput & IN_ACTION) || lara->Control.HandStatus != HandStatus::Free &&
		(lara->Control.HandStatus != HandStatus::WeaponReady || lara->Control.Weapon.GunType != LaraWeaponType::Flare))
	{
		return WaterClimbOutTestResult{ false };
	}

	WaterClimbOutTestResult climbOutResult;

	if (TestValidLedge(item, coll))
	{
		// Climb out down step.
		climbOutResult = TestLaraWaterClimbOutDownStep(item, coll);
		if (climbOutResult.Success)
		{
			SetAnimation(item, LA_ONWATER_TO_STAND_M1_STEP);
			return climbOutResult;
		}

		// Climb out down step to crouch.
		climbOutResult = TestLaraWaterClimbOutDownStepToCrouch(item, coll);
		if (climbOutResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			SetAnimation(item, LA_ONWATER_TO_CROUCH_M1_STEP);
			return climbOutResult;
		}

		// Climb out on flat step.
		climbOutResult = TestLaraWaterClimbOutFlatStep(item, coll);
		if (climbOutResult.Success)
		{
			SetAnimation(item, LA_ONWATER_TO_STAND_0_STEP);
			return climbOutResult;
		}

		// Climb out on flat step to crouch.
		climbOutResult = TestLaraWaterClimbOutFlatStepToCrouch(item, coll);
		if (climbOutResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			SetAnimation(item, LA_ONWATER_TO_CROUCH_0_STEP);
			return climbOutResult;
		}

		// Climb out up step.
		climbOutResult = TestLaraWaterClimbOutUpStep(item, coll);
		if (climbOutResult.Success)
		{
			SetAnimation(item, LA_ONWATER_TO_STAND_1_STEP);
			return climbOutResult;
		}

		// Climb out up step to crouch.
		climbOutResult = TestLaraWaterClimbOutUpStepToCrouch(item, coll);
		if (climbOutResult.Success &&
			g_GameFlow->HasCrawlExtended())
		{
			SetAnimation(item, LA_ONWATER_TO_CROUCH_1_STEP);
			return climbOutResult;
		}
	}

	return WaterClimbOutTestResult{ false };
}

LedgeHangTestResult TestLaraLedgeHang(ItemInfo* item, CollisionInfo* coll)
{
	int y = item->Pose.Position.y - coll->Setup.Height;
	auto probeFront = GetCollision(item, item->Pose.Orientation.GetY(), OFFSET_RADIUS(coll->Setup.Radius), -coll->Setup.Height);
	auto probeMiddle = GetCollision(item);

	// Debug
	g_Renderer.PrintDebugMessage("Ledge: %d", probeFront.Position.Floor);
	g_Renderer.PrintDebugMessage("Hands: %d", y);
	g_Renderer.PrintDebugMessage("Dif: %d", probeFront.Position.Floor - y);
	g_Renderer.PrintDebugMessage("VertVel: %d", item->Animation.VerticalVelocity);

	if (!TestValidLedge(item, coll, true))
		return LedgeHangTestResult{ false };

	int sign = copysign(1, item->Animation.VerticalVelocity);
	if (
		// TODO: Will need to modify this when ledge grabs are made to occur from control functions.
		(probeFront.Position.Floor * sign) >= (y * sign) &&											// Ledge is lower/higher than player's current position.
		(probeFront.Position.Floor * sign) <= ((y + item->Animation.VerticalVelocity) * sign) &&	// Ledge is higher/lower than player's projected position.
		
		//abs(probeFront.Position.Floor - y) <= abs(item->Animation.VerticalVelocity) &&			// Ledge height within snap threshold. TODO: Not ideal.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) >= CLICK(0.1f) &&				// Adequate hand room.
		abs(probeFront.Position.Floor - probeMiddle.Position.Floor) >= LARA_HEIGHT_STRETCH)			// Ledge high enough.
	{
		return LedgeHangTestResult{ true, probeFront.Position.Floor };
	}

	return LedgeHangTestResult{ false };
}

bool TestLaraShimmyLeft(ItemInfo* item, CollisionInfo* coll)
{
	if (TestLaraHangSideways(item, coll, -Angle::DegToRad(90.0f)))
		return true;

	return false;
}

bool TestLaraShimmyRight(ItemInfo* item, CollisionInfo* coll)
{
	if (TestLaraHangSideways(item, coll, Angle::DegToRad(90.0f)))
		return true;

	return false;
}

bool TestLaraLadderShimmyUp(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y - LARA_HEIGHT_STRETCH;
	auto probeLeft = GetCollision(item, Angle::Normalize(item->Pose.Orientation.GetY() - Angle::DegToRad(90.0f)), coll->Setup.Radius);
	auto probeRight = GetCollision(item, Angle::Normalize(item->Pose.Orientation.GetY() + Angle::DegToRad(90.0f)), coll->Setup.Radius);
	auto probeMiddle = GetCollision(item);

	if (lara->Control.CanClimbLadder &&
		(probeLeft.Position.Ceiling - y) <= -CLICK(1) &&
		(probeRight.Position.Ceiling - y) <= -CLICK(1) &&
		(probeMiddle.Position.Ceiling - y) <= -CLICK(1))
	{
		return true;
	}

	return false;
}

bool TestLaraLadderShimmyDown(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probeMiddle = GetCollision(item);

	if (lara->Control.CanClimbLadder &&
		(probeMiddle.Position.Floor - y) >= CLICK(1))
	{
		return true;
	}

	return false;
}

bool TestLaraHangClimbTolerance(ItemInfo* item, CollisionInfo* coll, HangClimbTestSetup testSetup)
{
	int y = item->Pose.Position.y - LARA_HEIGHT_STRETCH;
	int height = LARA_HEIGHT_STRETCH + CLICK(0.5f);
	auto probeFront = GetCollision(item, item->Pose.Orientation.GetY(), coll->Setup.Radius, -height);
	auto probeMiddle = GetCollision(item);

	bool isSlope = testSetup.CheckSlope ? probeFront.Position.FloorSlope : false;

	// Check for object.
	TestForObjectOnLedge(item, coll);
	if (coll->HitStatic)
		return false;

	// Check for slope (if applicable).
	if (isSlope)
		return false;

	// Assess climb feasibility.
	if (TestValidLedge(item, coll) &&
		abs(probeFront.Position.Floor - y) <= CLICK(1) &&
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testSetup.ClampMin &&	// Within clamp min.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testSetup.ClampMax &&	// Within clamp max.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= testSetup.GapMin)		// Gap is optically permissive.
	{
		return true;
	}

	return false;
}

bool TestLaraHangToCrouch(ItemInfo* item, CollisionInfo* coll)
{
	HangClimbTestSetup testSetup
	{
		LARA_HEIGHT_CRAWL, LARA_HEIGHT,
		CLICK(1),
		true
	};

	return TestLaraHangClimbTolerance(item, coll, testSetup);
}

bool TestLaraHangToStand(ItemInfo* item, CollisionInfo* coll)
{
	HangClimbTestSetup testSetup
	{
		LARA_HEIGHT, -MAX_HEIGHT,
		CLICK(1),
		false
	};

	return TestLaraHangClimbTolerance(item, coll, testSetup);
}

bool TestLaraJumpTolerance(ItemInfo* item, CollisionInfo* coll, JumpTestSetup testSetup)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	auto probe = GetCollision(item, testSetup.Angle, testSetup.Distance, -coll->Setup.Height);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);
	bool isWading = testSetup.CheckWadeStatus ? (lara->Control.WaterStatus == WaterStatus::Wade) : false;

	// Discard walls.
	if (probe.Position.Floor == NO_HEIGHT)
		return false;

	// Check for swamp or wade status (if applicable).
	if (isSwamp || isWading)
		return false;

	// Assess point/room collision.
	if (!TestLaraFacingCorner(item, testSetup.Angle, testSetup.Distance) &&						// Avoid jumping through corners.
		(probe.Position.Floor - y) >= -STEPUP_HEIGHT &&										// Within highest floor bound.
		((probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||	// Within lowest ceiling bound... 
			((probe.Position.Ceiling - y) < -coll->Setup.Height &&								// OR ceiling is level with Lara's head
				(probe.Position.Floor - y) >= CLICK(0.5f))))										// AND there is a drop below.
	{
		return true;
	}

	return false;
}

bool TestLaraRunJumpForward(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.GetY(),
		CLICK(1.5f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpForward(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.GetY()
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpBack(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpLeft(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.GetY() - Angle::DegToRad(90.0f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpRight(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		item->Pose.Orientation.GetY() + Angle::DegToRad(90.0f)
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraJumpUp(ItemInfo* item, CollisionInfo* coll)
{
	JumpTestSetup testSetup
	{
		0,
		0,
		false
	};

	return TestLaraJumpTolerance(item, coll, testSetup);
}

bool TestLaraSlideJump(ItemInfo* item, CollisionInfo* coll)
{
	return true;

	// TODO: Broken on diagonal slides?
	if (g_GameFlow->HasSlideExtended())
	{
		auto probe = GetCollision(item);

		float direction = GetLaraSlideDirection(item, coll);
		float steepness = GetSurfaceSteepnessAngle(probe.FloorTilt.x, probe.FloorTilt.y);
		return (abs(coll->Setup.ForwardAngle - direction) <= abs(steepness));
	}

	return true;
}

bool TestLaraCrawlspaceDive(ItemInfo* item, CollisionInfo* coll)
{
	auto probe = GetCollision(item, coll->Setup.ForwardAngle, coll->Setup.Radius, -coll->Setup.Height);
	
	if (abs(probe.Position.Ceiling - probe.Position.Floor) < LARA_HEIGHT ||
		TestLaraKeepLow(item, coll))
	{
		return true;
	}

	return false;
}

bool TestLaraTightropeDismount(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	auto probe = GetCollision(item);

	if (probe.Position.Floor == item->Pose.Position.y &&
		lara->Control.Tightrope.CanDismount)
	{
		return true;
	}

	return false;
}

bool TestLaraPoleCollision(ItemInfo* item, CollisionInfo* coll, bool up, float offset)
{
	static constexpr auto poleProbeCollRadius = 16.0f;

	bool atLeastOnePoleCollided = false;

	if (GetCollidedObjects(item, SECTOR(1), true, CollidedItems, nullptr, 0) && CollidedItems[0])
	{
		auto laraBox = TO_DX_BBOX(item->Pose, GetBoundsAccurate(item));

		// HACK: because Core implemented upward pole movement as SetPosition command, we can't precisely
		// check her position. So we add a fixed height offset.

		auto sphere = BoundingSphere(laraBox.Center + Vector3(0, (laraBox.Extents.y + poleProbeCollRadius + offset) * (up ? -1 : 1), 0), poleProbeCollRadius);

		//g_Renderer.AddDebugSphere(sphere.Center, 16.0f, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

		int i = 0;
		while (CollidedItems[i] != NULL)
		{
			auto*& object = CollidedItems[i];
			i++;

			if (object->ObjectNumber != ID_POLEROPE)
				continue;

			auto poleBox = TO_DX_BBOX(object->Pose, GetBoundsAccurate(object));
			poleBox.Extents = poleBox.Extents + Vector3(coll->Setup.Radius, 0, coll->Setup.Radius);

			//g_Renderer.AddDebugBox(poleBox, Vector4(0, 0, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			if (poleBox.Intersects(sphere))
			{
				atLeastOnePoleCollided = true;
				break;
			}
		}
	}

	return atLeastOnePoleCollided;
}

bool TestLaraPoleUp(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestLaraPoleCollision(item, coll, true, CLICK(1)))
		return false;

	return (coll->Middle.Ceiling < -CLICK(1));
}

bool TestLaraPoleDown(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestLaraPoleCollision(item, coll, false))
		return false;

	return (coll->Middle.Floor > 0);
}
