#include "framework.h"
#include "tr4_spikyceiling.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"

void ControlSpikyCeiling(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED)
	{
		int y = item->Pose.Position.y + ((item->ItemFlags[0] == 1) ? 10 : 5);
		auto probe = GetCollision(item->Pose.Position.x, y, item->Pose.Position.z, item->RoomNumber);

		if (probe.Position.Floor < (y + SECTOR(1)))
		{
			item->Status = ITEM_DEACTIVATED;
			StopSoundEffect(147);
		}
		else
		{
			item->Pose.Position.y = y;

			if (probe.RoomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			SoundEffect(147, &item->Pose, 0);
		}
	}

	if (item->TouchBits)
	{
		LaraItem->HitPoints -= 20;
		LaraItem->HitStatus = true;

		DoLotsOfBlood(LaraItem->Pose.Position.x, item->Pose.Position.y + CLICK(3), LaraItem->Pose.Position.z, 4, item->Pose.Orientation.GetY(), LaraItem->RoomNumber, 3);
		item->TouchBits = 0;

		SoundEffect(56, &item->Pose, 0);
	}

	if (TriggerActive(item) && item->Status != ITEM_DEACTIVATED && item->ItemFlags[0] == 1)
		AnimateItem(item);
}
