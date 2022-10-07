#include "framework.h"
#include "Objects/TR5/Object/tr5_pushable_block.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR5/Object/tr5_pushable_block_info.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;

auto PushableBlockPos = Vector3Int::Zero;
static OBJECT_COLLISION_BOUNDS PushableBlockBounds = 
{
	0, 0,
	-CLICK(0.25f), 0,
	0, 0,
	ANGLE(-10.0f), ANGLE(10.0f),
	ANGLE(-30.0f), ANGLE(30.0f),
	ANGLE(-10.0f), ANGLE(10.0f)
};

PushableInfo& GetPushableInfo(const ItemInfo& item)
{
	return (PushableInfo&)item.Data;
}

PushableInfo& GetPushableInfo(ItemInfo* item)
{
	return (PushableInfo&)item->Data;
}

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
{
	auto* floor = GetFloor(x, y, z, &roomNumber);
	if (floor->Box == NO_BOX)
		return;

	g_Level.Boxes[floor->Box].flags &= ~BLOCKED;
	short height = g_Level.Boxes[floor->Box].height;
	short baseRoomNumber = roomNumber;
	
	floor = GetFloor(x + SECTOR(1), y, z, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height &&
			(g_Level.Boxes[floor->Box].flags & BLOCKABLE) &&
			(g_Level.Boxes[floor->Box].flags & BLOCKED))
		{
			ClearMovableBlockSplitters(x + SECTOR(1), y, z, roomNumber);
		}
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - SECTOR(1), y, z, &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height &&
			(g_Level.Boxes[floor->Box].flags & BLOCKABLE) &&
			(g_Level.Boxes[floor->Box].flags & BLOCKED))
		{
			ClearMovableBlockSplitters(x - SECTOR(1), y, z, roomNumber);
		}
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + SECTOR(1), &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height &&
			(g_Level.Boxes[floor->Box].flags & BLOCKABLE) &&
			(g_Level.Boxes[floor->Box].flags & BLOCKED))
		{
			ClearMovableBlockSplitters(x, y, z + SECTOR(1), roomNumber);
		}
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - SECTOR(1), &roomNumber);
	if (floor->Box != NO_BOX)
	{
		if (g_Level.Boxes[floor->Box].height == height &&
			(g_Level.Boxes[floor->Box].flags & BLOCKABLE) &&
			(g_Level.Boxes[floor->Box].flags & BLOCKED))
		{
			ClearMovableBlockSplitters(x, y, z - SECTOR(1), roomNumber);
		}
	}
}

void InitialisePushableBlock(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	// TODO: Use itemFlags[1] to hold linked index for now.
	item.ItemFlags[1] = NO_ITEM;
	
	// Allocate new pushable info.
	item.Data = PushableInfo();
	auto& pushable = GetPushableInfo(item);
	
	// TODO: Lua.
	pushable.StackLimit = 3;
	pushable.Gravity = 8;
	pushable.Weight = 100;

	pushable.MoveX = item.Pose.Position.x;
	pushable.MoveZ = item.Pose.Position.z;

	// Read flags from OCB.
	int OCB = item.TriggerFlags;

	pushable.CanFall = OCB & 0x20;
	pushable.DisablePull = OCB & 0x80;
	pushable.DisablePush = OCB & 0x100;
	pushable.DisableW = pushable.DisableE = OCB & 0x200;
	pushable.DisableN = pushable.DisableS = OCB & 0x400;
	
	// TODO: Maybe there is a better way to handle this that doesn't involve OCBs.
	pushable.Climb = 0;
	/*pushable.Climb |= (OCB & 0x800) ? CLIMB_WEST : 0;
	pushable.Climb |= (OCB & 0x1000) ? CLIMB_NORTH : 0;
	pushable.Climb |= (OCB & 0x2000) ? CLIMB_EAST : 0;
	pushable.Climb |= (OCB & 0x4000) ? CLIMB_SOUTH : 0;*/

	pushable.HasFloorCeiling = false;

	int height;
	if ((OCB & 0x40) && (OCB & 0x1F) >= 2)
	{
		pushable.HasFloorCeiling = true;
		TEN::Floordata::AddBridge(itemNumber);
		height = (OCB & 0x1F) * CLICK(1);
	}
	else
		height = -GetBoundsAccurate(&item)->Y1;

	pushable.Height = height;

	// TODO: Lua.
	pushable.FallSoundID = SFX_TR4_BOULDER_FALL;

	// Check for stack formation when pushables are initialized.
	FindStack(itemNumber);
}

void PushableBlockControl(short itemNumber)
{
	auto* pushableItem = &g_Level.Items[itemNumber];
	auto& pushable = GetPushableInfo(pushableItem);
	auto* laraItem = LaraItem;
	auto* lara = GetLaraInfo(laraItem);

	lara->InteractedItem = itemNumber;

	auto pos = Vector3Int::Zero;
	short quadrant = GetQuadrant(laraItem->Pose.Orientation.y);

	int blockHeight = GetStackHeight(pushableItem);

	// Control falling.
	if (pushableItem->Animation.IsAirborne)
	{
		int floorHeight = GetCollision(pushableItem->Pose.Position.x, pushableItem->Pose.Position.y + 10, pushableItem->Pose.Position.z, pushableItem->RoomNumber).Position.Floor;

		if (pushableItem->Pose.Position.y < (floorHeight - pushableItem->Animation.Velocity.y))
		{
			if ((pushableItem->Animation.Velocity.y + pushable.Gravity) < CLICK(0.5f))
				pushableItem->Animation.Velocity.y += pushable.Gravity;
			else
				pushableItem->Animation.Velocity.y++;
			pushableItem->Pose.Position.y += pushableItem->Animation.Velocity.y;

			MoveStackY(itemNumber, pushableItem->Animation.Velocity.y);
		}
		else
		{
			pushableItem->Animation.IsAirborne = false;
			int relY = floorHeight - pushableItem->Pose.Position.y;
			pushableItem->Pose.Position.y = floorHeight;

			if (pushableItem->Animation.Velocity.y >= 96.0f)
				FloorShake(pushableItem);

			pushableItem->Animation.Velocity.y = 0.0f;
			SoundEffect(pushable.FallSoundID, &pushableItem->Pose, SoundEnvironment::Always);

			MoveStackY(itemNumber, relY);
			AddBridgeStack(itemNumber);

			// If fallen on top of existing pushables, don't test triggers.
			if (FindStack(itemNumber) == NO_ITEM)
				TestTriggers(pushableItem, true, pushableItem->Flags & IFLAG_ACTIVATION_MASK);
			
			RemoveActiveItem(itemNumber);
			pushableItem->Status = ITEM_NOT_ACTIVE;

			if (pushable.HasFloorCeiling)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(pushableItem, pushableItem->ItemFlags[0] + ANGLE(90.0f), false);
			}
		}

		return;
	}

	// Move pushable based on bbox->Z2 of lara->Z2.
	int displaceBox = GetBoundsAccurate(laraItem)->Z2 - 80;

	int x = 0;
	int z = 0;

	switch (laraItem->Animation.AnimNumber)
	{
	case LA_PUSHABLE_PUSH:

		if (laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		switch (quadrant) 
		{
		case 0:
			z = pushable.MoveZ + displaceBox;

			if (abs(pushableItem->Pose.Position.z - z) < CLICK(2) && pushableItem->Pose.Position.z < z)
				pushableItem->Pose.Position.z = z;

			break;

		case 1:
			x = pushable.MoveX + displaceBox;

			if (abs(pushableItem->Pose.Position.x - x) < CLICK(2) && pushableItem->Pose.Position.x < x)
				pushableItem->Pose.Position.x = x;

			break;

		case 2:
			z = pushable.MoveZ - displaceBox;

			if (abs(pushableItem->Pose.Position.z - z) < CLICK(2) && pushableItem->Pose.Position.z > z)
				pushableItem->Pose.Position.z = z;

			break;

		case 3:
			x = pushable.MoveX - displaceBox;

			if (abs(pushableItem->Pose.Position.x - x) < CLICK(2) && pushableItem->Pose.Position.x > x)
				pushableItem->Pose.Position.x = x;

			break;

		default:
			break;
		}

		MoveStackXZ(itemNumber);

		if (laraItem->Animation.FrameNumber == (g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd - 1))
		{
			// Check if pushable is about to fall.
			if (pushable.CanFall)
			{
				int floorHeight = GetCollision(pushableItem, 0, 0, 10).Position.Floor;
				if (floorHeight > pushableItem->Pose.Position.y)
				{
					pushableItem->Pose.Position.x = pushableItem->Pose.Position.x & 0xFFFFFE00 | 0x200;
					pushableItem->Pose.Position.z = pushableItem->Pose.Position.z & 0xFFFFFE00 | 0x200;
					MoveStackXZ(itemNumber);
					laraItem->Animation.TargetState = LS_IDLE;
					pushableItem->Animation.IsAirborne = true;
					return;
				}
			}

			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPush(pushableItem, blockHeight, quadrant))
					laraItem->Animation.TargetState = LS_IDLE;
				else
				{
					pushableItem->Pose.Position.x = pushable.MoveX = pushableItem->Pose.Position.x & 0xFFFFFE00 | 0x200;
					pushableItem->Pose.Position.z = pushable.MoveZ = pushableItem->Pose.Position.z & 0xFFFFFE00 | 0x200;
					TestTriggers(pushableItem, true, pushableItem->Flags & IFLAG_ACTIVATION_MASK);
				}
			}
			else
				laraItem->Animation.TargetState = LS_IDLE;
		}

		break;

	case LA_PUSHABLE_PULL:

		if (laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameBase)
		{
			RemoveFromStack(itemNumber);
			RemoveBridgeStack(itemNumber);
		}

		switch (quadrant)
		{
		case NORTH:
			z = pushable.MoveZ + displaceBox;

			if (abs(pushableItem->Pose.Position.z - z) < CLICK(2) && pushableItem->Pose.Position.z > z)
				pushableItem->Pose.Position.z = z;

			break;

		case EAST:
			x = pushable.MoveX + displaceBox;

			if (abs(pushableItem->Pose.Position.x - x) < CLICK(2) && pushableItem->Pose.Position.x > x)
				pushableItem->Pose.Position.x = x;

			break;

		case SOUTH:
			z = pushable.MoveZ - displaceBox;

			if (abs(pushableItem->Pose.Position.z - z) < CLICK(2) && pushableItem->Pose.Position.z < z)
				pushableItem->Pose.Position.z = z;

			break;

		case WEST:
			x = pushable.MoveX - displaceBox;

			if (abs(pushableItem->Pose.Position.x - x) < CLICK(2) && pushableItem->Pose.Position.x < x)
				pushableItem->Pose.Position.x = x;

			break;

		default:
			break;
		}

		MoveStackXZ(itemNumber);

		if (laraItem->Animation.FrameNumber == (g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd - 1))
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPull(pushableItem, blockHeight, quadrant))
					laraItem->Animation.TargetState = LS_IDLE;
				else
				{
					pushableItem->Pose.Position.x = pushable.MoveX = pushableItem->Pose.Position.x & 0xFFFFFE00 | 0x200;
					pushableItem->Pose.Position.z = pushable.MoveZ = pushableItem->Pose.Position.z & 0xFFFFFE00 | 0x200;
					TestTriggers(pushableItem, true, pushableItem->Flags & IFLAG_ACTIVATION_MASK);
				}
			}
			else
				laraItem->Animation.TargetState = LS_IDLE;
		}

		break;

	case LA_PUSHABLE_PUSH_TO_STAND:
	case LA_PUSHABLE_PULL_TO_STAND:
		if (laraItem->Animation.FrameNumber == g_Level.Anims[LA_PUSHABLE_PUSH_TO_STAND].frameBase ||
			laraItem->Animation.FrameNumber == g_Level.Anims[LA_PUSHABLE_PULL_TO_STAND].frameBase)
		{
			pushableItem->Pose.Position.x = pushableItem->Pose.Position.x & 0xFFFFFE00 | 0x200;
			pushableItem->Pose.Position.z = pushableItem->Pose.Position.z & 0xFFFFFE00 | 0x200;

			MoveStackXZ(itemNumber);
			FindStack(itemNumber);
			AddBridgeStack(itemNumber);

			TestTriggers(pushableItem, true, pushableItem->Flags & IFLAG_ACTIVATION_MASK);
		}

		if (laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
		{
			RemoveActiveItem(itemNumber);
			pushableItem->Status = ITEM_NOT_ACTIVE;

			if (pushable.HasFloorCeiling)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(pushableItem, pushableItem->ItemFlags[0] + ANGLE(90.0f), false);
			}
		}

		break;
	}
}

void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* pushableItem = &g_Level.Items[itemNumber];
	auto& pushable = GetPushableInfo(pushableItem);
	auto* lara = GetLaraInfo(laraItem);

	int blockHeight = GetStackHeight(pushableItem);
	
	if ((!(TrInput & IN_ACTION) ||
		pushableItem->Status == ITEM_INVISIBLE ||
		pushableItem->TriggerFlags < 0 ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
		laraItem->Animation.IsAirborne ||
		lara->Control.HandStatus != HandStatus::Free) &&
		(!lara->Control.IsMoving || lara->InteractedItem != itemNumber))
	{
		if ((laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
			(laraItem->Animation.FrameNumber != g_Level.Anims[LA_PUSHABLE_GRAB].frameEnd) ||
			lara->NextCornerPos.Position.x != itemNumber))
		{
			if (!pushable.HasFloorCeiling)
				ObjectCollision(itemNumber, laraItem, coll);

			return;
		}

		bool isQuadrantDisabled = false;
		short quadrant = GetQuadrant(laraItem->Pose.Orientation.y);
		switch (quadrant)
		{
		case NORTH:
			isQuadrantDisabled = pushable.DisableN;
			break;

		case EAST:
			isQuadrantDisabled = pushable.DisableE;
			break;

		case SOUTH:
			isQuadrantDisabled = pushable.DisableS;
			break;

		case WEST:
			isQuadrantDisabled = pushable.DisableW;
			break;
		}

		if (isQuadrantDisabled)
			return;

		if (!CheckStackLimit(pushableItem))
			return;

		if (!TestBlockMovable(pushableItem, blockHeight))
			return;

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(pushableItem, blockHeight, quadrant) || pushable.DisablePush)
				return;

			laraItem->Animation.TargetState = LS_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(pushableItem, blockHeight, quadrant) || pushable.DisablePull)
				return;

			laraItem->Animation.TargetState = LS_PUSHABLE_PULL;
		}
		else
			return;

		pushableItem->Status = ITEM_ACTIVE;
		AddActiveItem(itemNumber);
		ResetLaraFlex(laraItem);
		
		pushable.MoveX = pushableItem->Pose.Position.x;
		pushable.MoveZ = pushableItem->Pose.Position.z;

		if (pushable.HasFloorCeiling)
		{
			//AlterFloorHeight(item, ((item->triggerFlags - 64) * 256));
			AdjustStopperFlag(pushableItem, pushableItem->ItemFlags[0], false);
		}
	}
	else
	{
		auto* bounds = GetBoundsAccurate(pushableItem);
		PushableBlockBounds.boundingBox.X1 = (bounds->X1 / 2) - 100;
		PushableBlockBounds.boundingBox.X2 = (bounds->X2 / 2) + 100;
		PushableBlockBounds.boundingBox.Z1 = bounds->Z1 - 200;
		PushableBlockBounds.boundingBox.Z2 = 0;

		short yOrient = pushableItem->Pose.Orientation.y;
		pushableItem->Pose.Orientation.y = (laraItem->Pose.Orientation.y + ANGLE(45.0f)) & 0xC000;

		if (TestLaraPosition(&PushableBlockBounds, pushableItem, laraItem))
		{
			int quadrant = GetQuadrant(pushableItem->Pose.Orientation.y);
			if (quadrant & 1)
				PushableBlockPos.z = bounds->X1 - CLICK(0.4f);
			else
				PushableBlockPos.z = bounds->Z1 - CLICK(0.4f);

			if (pushable.HasFloorCeiling)
			{					
				// For now, don't use auto-align function because it can collide with player's vault.
				laraItem->Pose.Orientation = pushableItem->Pose.Orientation;

				SetAnimation(laraItem, LA_PUSHABLE_GRAB);
				lara->Control.IsMoving = false;
				lara->Control.HandStatus = HandStatus::Busy;
				lara->NextCornerPos.Position.x = itemNumber;
				pushableItem->Pose.Orientation.y = yOrient;
			}
			else
			{
				if (MoveLaraPosition(&PushableBlockPos, pushableItem, laraItem))
				{
					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Busy;
					lara->NextCornerPos.Position.x = itemNumber;
					pushableItem->Pose.Orientation.y = yOrient;
				}
				else
				{
					lara->InteractedItem = itemNumber;
					pushableItem->Pose.Orientation.y = yOrient;
				}
			}
		}
		else
		{
			if (lara->Control.IsMoving && lara->InteractedItem == itemNumber)
			{
				lara->Control.IsMoving = false;
				lara->Control.HandStatus = HandStatus::Free;
			}

			pushableItem->Pose.Orientation.y = yOrient;
		}
	}
}

bool TestBlockMovable(ItemInfo* item, int blockHeight)
{
	auto collPoint = GetCollision(item);

	// Assess for wall.
	if (collPoint.BottomBlock->IsWall(collPoint.BottomBlock->SectorPlane(item->Pose.Position.x, item->Pose.Position.z)))
		return false;

	// Determine whether surface is level.
	if (collPoint.Position.Floor != item->Pose.Position.y)
		return false;

	return true;
}

bool TestBlockPush(ItemInfo* item, int blockHeight, unsigned short quadrant)
{
	const auto& pushable = GetPushableInfo(item);

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z;
	
	switch (quadrant)
	{
	case NORTH:
		z += SECTOR(1);
		break;

	case EAST:
		x += SECTOR(1);
		break;

	case SOUTH:
		z -= SECTOR(1);
		break;

	case WEST:
		x -= SECTOR(1);
		break;
	}

	auto pointColl = GetCollision(x, y - blockHeight, z, item->RoomNumber);

	auto* room = &g_Level.Rooms[pointColl.RoomNumber];
	if (GetSector(room, x - room->x, z - room->z)->Stopper)
		return false;

	if (pointColl.Position.FloorSlope || pointColl.Position.DiagonalStep ||
		pointColl.Block->FloorSlope(0) != Vector2::Zero || pointColl.Block->FloorSlope(1) != Vector2::Zero)
	{
		return false;
	}

	if (pushable.CanFall)
	{
		if (pointColl.Position.Floor < y)
			return false;
	}
	else
	{
		if (pointColl.Position.Floor != y)
			return false;
	}

	int ceiling = (y - blockHeight) + 100;

	if (GetCollision(x, ceiling, z, item->RoomNumber).Position.Ceiling > ceiling)
		return false;

	int prevX = item->Pose.Position.x;
	int prevZ = item->Pose.Position.z;
	item->Pose.Position.x = x;
	item->Pose.Position.z = z;
	GetCollidedObjects(item, CLICK(1), true, &CollidedItems[0], nullptr, true);
	item->Pose.Position.x = prevX;
	item->Pose.Position.z = prevZ;

	int i = 0;
	while (CollidedItems[i] != nullptr)
	{
		if (Objects[CollidedItems[i]->ObjectNumber].floor == 0)
			return false;
		else
		{
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // Index of CollidedItems[i].

			// Check whether floor function returns nullopt.
			auto collPos = CollidedItems[i]->Pose.Position;
			if (object.floor(collidedIndex, collPos.x, collPos.y, collPos.z) == std::nullopt)
				return false;
		}

		i++;
	}

	return true;
}

bool TestBlockPull(ItemInfo* pushableItem, int blockHeight, short quadrant)
{
	auto* laraItem = LaraItem;

	int xAdd = 0;
	int zAdd = 0;

	switch (quadrant)
	{
	case NORTH:
		zAdd = -SECTOR(1);
		break;

	case EAST:
		xAdd = -SECTOR(1);
		break;

	case SOUTH:
		zAdd = SECTOR(1);
		break;

	case WEST:
		xAdd = SECTOR(1);
		break;
	}

	int x = pushableItem->Pose.Position.x + xAdd;
	int y = pushableItem->Pose.Position.y;
	int z = pushableItem->Pose.Position.z + zAdd;

	short roomNumber = pushableItem->RoomNumber;
	auto* room = &g_Level.Rooms[roomNumber];
	if (GetSector(room, x - room->x, z - room->z)->Stopper)
		return false;

	auto pointColl = GetCollision(x, y - blockHeight, z, pushableItem->RoomNumber);

	if (pointColl.Position.Floor != y)
		return false;

	if (pointColl.Position.FloorSlope || pointColl.Position.DiagonalStep ||
		pointColl.Block->FloorSlope(0) != Vector2::Zero || pointColl.Block->FloorSlope(1) != Vector2::Zero)
		return false;

	int ceiling = (y - blockHeight) + 100;

	if (GetCollision(x, ceiling, z, pushableItem->RoomNumber).Position.Ceiling > ceiling)
		return false;

	int prevX = pushableItem->Pose.Position.x;
	int prevZ = pushableItem->Pose.Position.z;
	pushableItem->Pose.Position.x = x;
	pushableItem->Pose.Position.z = z;
	GetCollidedObjects(pushableItem, CLICK(1), true, &CollidedItems[0], nullptr, true);
	pushableItem->Pose.Position.x = prevX;
	pushableItem->Pose.Position.z = prevZ;

	int i = 0;
	while (CollidedItems[i] != nullptr)
	{
		if (Objects[CollidedItems[i]->ObjectNumber].floor == 0)
			return false;
		else
		{
			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data();

			auto collPos = CollidedItems[i]->Pose.Position;
			if (object.floor(collidedIndex, collPos.x, collPos.y, collPos.z) == std::nullopt)
				return false;
		}

		i++;
	}

	int xAddLara = 0, zAddLara = 0;
	switch (quadrant)
	{
	case NORTH:
		zAddLara = GetBestFrame(laraItem)->offsetZ;
		break;
	case EAST:
		xAddLara = GetBestFrame(laraItem)->offsetZ;
		break;
	case SOUTH:
		zAddLara = -GetBestFrame(laraItem)->offsetZ;
		break;
	case WEST:
		xAddLara = -GetBestFrame(laraItem)->offsetZ;
		break;
	}

	x = laraItem->Pose.Position.x + xAdd + xAddLara;
	z = laraItem->Pose.Position.z + zAdd + zAddLara;

	roomNumber = laraItem->RoomNumber;

	pointColl = GetCollision(x, y - LARA_HEIGHT, z, laraItem->RoomNumber);

	room = &g_Level.Rooms[roomNumber];
	if (GetSector(room, x - room->x, z - room->z)->Stopper)
		return false;

	if (pointColl.Position.Floor != y)
		return false;

	if (pointColl.Block->CeilingHeight(x, z) > y - LARA_HEIGHT)
		return false;

	prevX = laraItem->Pose.Position.x;
	prevZ = laraItem->Pose.Position.z;
	laraItem->Pose.Position.x = x;
	laraItem->Pose.Position.z = z;
	GetCollidedObjects(laraItem, CLICK(1), true, &CollidedItems[0], nullptr, true);
	laraItem->Pose.Position.x = prevX;
	laraItem->Pose.Position.z = prevZ;

	i = 0;
	while (CollidedItems[i] != nullptr)
	{
		// If collided item is not pushable in which player embedded.
		if (CollidedItems[i] != pushableItem)
		{
			if (Objects[CollidedItems[i]->ObjectNumber].floor == 0)
				return false;
			else
			{
				const auto& object = Objects[CollidedItems[i]->ObjectNumber];
				int collidedIndex = CollidedItems[i] - g_Level.Items.data();

				auto collPos  = CollidedItems[i]->Pose.Position;
				if (object.floor(collidedIndex, collPos.x, collPos.y, collPos.z) == std::nullopt)
					return false;
			}
		}

		i++;
	}

	return true;
}

void MoveStackXZ(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short probedRoomNumber = GetCollision(item).RoomNumber;
	if (probedRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probedRoomNumber);

	// Move pushable stack along with bottom pushable.
	auto* stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM)
	{
		int stackIndex = stackItem->ItemFlags[1];
		stackItem = &g_Level.Items[stackIndex];

		stackItem->Pose.Position.x = item->Pose.Position.x;
		stackItem->Pose.Position.z = item->Pose.Position.z;

		probedRoomNumber = GetCollision(item).RoomNumber;
		if (probedRoomNumber != stackItem->RoomNumber)
			ItemNewRoom(stackIndex, probedRoomNumber);
	}
}

void MoveStackY(short itemNumber, int y)
{
	auto* item = &g_Level.Items[itemNumber];

	short probedRoomNumber = GetCollision(item).RoomNumber;
	if (probedRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probedRoomNumber);

	// Move pushable stack along with bottom pushable.
	while (item->ItemFlags[1] != NO_ITEM)
	{
		int stackIndex = item->ItemFlags[1];
		item = &g_Level.Items[stackIndex];

		item->Pose.Position.y += y;

		probedRoomNumber = GetCollision(item).RoomNumber;
		if (probedRoomNumber != item->RoomNumber)
			ItemNewRoom(stackIndex, probedRoomNumber);
	}
}

void AddBridgeStack(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = GetPushableInfo(item);

	if (pushable.HasFloorCeiling)
		TEN::Floordata::AddBridge(itemNumber);

	int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto* stackItem = &g_Level.Items[stackIndex];

		if (pushable.HasFloorCeiling)
			TEN::Floordata::AddBridge(stackIndex);

		stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
	}
}

void RemoveBridgeStack(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = GetPushableInfo(item);

	if (pushable.HasFloorCeiling)
		TEN::Floordata::RemoveBridge(itemNumber);

	int stackIndex = g_Level.Items[itemNumber].ItemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto* stackItem = &g_Level.Items[stackIndex];

		if (pushable.HasFloorCeiling)
			TEN::Floordata::RemoveBridge(stackIndex);

		stackIndex = g_Level.Items[stackIndex].ItemFlags[1];
	}
}

// Unlink pushable from stack if linked.
void RemoveFromStack(short itemNumber)
{
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (i == itemNumber)
			continue;

		auto& itemBelow = g_Level.Items[i];

		int objectNumber = itemBelow.ObjectNumber;
		if (objectNumber >= ID_PUSHABLE_OBJECT1 && objectNumber <= ID_PUSHABLE_OBJECT10)
		{
			if (itemBelow.ItemFlags[1] == itemNumber)
				itemBelow.ItemFlags[1] = NO_ITEM;
		}
	}
}

int FindStack(short itemNumber)
{
	int stackTop = NO_ITEM;		// Index of heighest pushable in stack.
	int stackYmin = SECTOR(64); // Set starting height

	// Check for pushable directly below current one in the same sector.
	for (int i = 0; i < g_Level.NumItems; i++)
	{
		if (i == itemNumber)
			continue;

		const auto& itemBelow = g_Level.Items[i];

		int objectNumber = itemBelow.ObjectNumber;
		if (objectNumber >= ID_PUSHABLE_OBJECT1 &&
			objectNumber <= ID_PUSHABLE_OBJECT10)
		{
			const auto& item = g_Level.Items[itemNumber];

			if (itemBelow.Pose.Position.x == item.Pose.Position.x &&
				itemBelow.Pose.Position.z == item.Pose.Position.z)
			{
				// Set heighest pushable so far as top of stack.
				int belowY = itemBelow.Pose.Position.y;
				if (belowY > item.Pose.Position.y &&
					belowY < stackYmin)
				{
					stackTop = i;
					stackYmin = itemBelow.Pose.Position.y;
				}
			}
		}
	}

	if (stackTop != NO_ITEM)
		g_Level.Items[stackTop].ItemFlags[1] = itemNumber;

	return stackTop;
}

int GetStackHeight(ItemInfo* item)
{
	const auto& pushable = GetPushableInfo(item);

	int height = pushable.Height;

	auto* stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM)
	{
		stackItem = &g_Level.Items[stackItem->ItemFlags[1]];
		height += pushable.Height;
	}

	return height;
}

bool CheckStackLimit(ItemInfo* item)
{
	const auto& pushable = GetPushableInfo(item);

	int limit = pushable.StackLimit;
	
	int count = 1;
	auto* stackItem = item;
	while (stackItem->ItemFlags[1] != NO_ITEM)
	{
		stackItem = &g_Level.Items[stackItem->ItemFlags[1]];
		count++;

		if (count > limit)
			return false;
	}

	return true;
}

std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = GetPushableInfo(item);
	
	if (item.Status != ITEM_INVISIBLE && pushable.HasFloorCeiling)
	{
		int height = item.Pose.Position.y - (item.TriggerFlags & 0x1F) * CLICK(1);
		return std::optional{ height };
	}

	return std::nullopt;
}

std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	const auto& pushable = GetPushableInfo(item);

	if (item.Status != ITEM_INVISIBLE && pushable.HasFloorCeiling)
		return std::optional{item.Pose.Position.y};

	return std::nullopt;
}

int PushableBlockFloorBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];

	int height = item.Pose.Position.y - ((item.TriggerFlags & 0x1F) * CLICK(1));
	return height;
}

int PushableBlockCeilingBorder(short itemNumber)
{
	const auto& item = g_Level.Items[itemNumber];

	return item.Pose.Position.y;
}
