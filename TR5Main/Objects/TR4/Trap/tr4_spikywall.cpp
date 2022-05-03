#include "framework.h"
#include "tr4_spikywall.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/effects/effects.h"

void ControlSpikyWall(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	// Move wall.
	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED)
	{
		int x = item->Pose.Position.x + sin(item->Pose.Orientation.GetY());
		int z = item->Pose.Position.z + cos(item->Pose.Orientation.GetY());
		auto probe = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber);

		if (probe.Position.Floor != item->Pose.Position.y)
		{
			item->Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		else
		{
			item->Pose.Position.x = x;
			item->Pose.Position.z = z;

			if (probe.RoomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item->Pose, 0);
		}
	}

	if (item->TouchBits)
	{
		LaraItem->HitPoints -= 15;
		LaraItem->HitStatus = true;

		DoLotsOfBlood(LaraItem->Pose.Position.x, LaraItem->Pose.Position.y - CLICK(2), LaraItem->Pose.Position.z, 4, item->Pose.Orientation.GetY(), LaraItem->RoomNumber, 3);
		item->TouchBits = 0;

		SoundEffect(56, &item->Pose, 0);
	}
}
