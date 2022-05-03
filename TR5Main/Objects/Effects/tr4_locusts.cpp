#include "framework.h"
#include "Objects/Effects/tr4_locusts.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4 
{
	LOCUST_INFO Locusts[MAX_LOCUSTS];

	int CreateLocust(void)
	{
		for (int i = 0; i < MAX_LOCUSTS; i++)
		{
			auto* locust = &Locusts[i];
			if (!locust->on)
				return i;
		}

		return NO_ITEM;
	}

	void SpawnLocust(ITEM_INFO* item)
	{
		Vector3Int start, end;
		float angles[2];

	   short locustNumber = CreateLocust();
		if (locustNumber != NO_ITEM)
		{
			auto* locust = &Locusts[locustNumber];

			// Emitter.
			if (item->ObjectNumber == ID_LOCUSTS_EMITTER)
			{
				end.x = item->Pose.Position.x;
				end.y = item->Pose.Position.y;
				end.z = item->Pose.Position.z;
				angles[0] = item->Pose.Orientation.GetY() - Angle::DegToRad(180.0f);
				angles[1] = 0;
			}
			// Mutant.
			else
			{
				start.x = 0;
				start.y = -96;
				start.z = 144;
				GetJointAbsPosition(item, &start, 9);
				end.x = 0;
				end.y = -128;
				end.z = 288;
				GetJointAbsPosition(item, &end, 9);
				phd_GetVectorAngles(end.x - start.x, end.y - start.y, end.z - start.z, angles);
			}

			// NOTE: this is not present in original TR4 code
			//target = GetCreatureInfo(item)->enemy;

			locust->on = true;
			//locust->target = target != nullptr ? target : nullptr;
			locust->pos.Position = end;
			locust->pos.Orientation.SetX((GetRandomControl() & 0x3FF) + angles[1] - 0x200);
			locust->pos.Orientation.SetY((GetRandomControl() & 0x7FF) + angles[0] - 0x400);
			locust->roomNumber = item->RoomNumber;
			locust->randomRotation = (GetRandomControl() & 0x1F) + 0x10;
			locust->escapeXrot = ((GetRandomControl() & 0x7) + 0xF) * 20;
			locust->escapeYrot = (GetRandomControl() & 0x1FF);
			locust->counter = 0;
		}
	}

	void InitialiseLocust(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Pose.Orientation.GetY() > 0)
		{
			if (item->Pose.Orientation.GetY() == Angle::DegToRad(90.0f))
				item->Pose.Position.x += CLICK(2);
		}
		else if (item->Pose.Orientation.GetY() < 0)
		{
			if (item->Pose.Orientation.GetY() == Angle::DegToRad(-180.0f))
				item->Pose.Position.z -= CLICK(2);
			else if (item->Pose.Orientation.GetY() == Angle::DegToRad(-90.0f))
				item->Pose.Position.x -= CLICK(2);
		}
		else
			item->Pose.Position.z += CLICK(2);
	}

	void LocustControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			if (item->TriggerFlags)
			{
				SpawnLocust(item);
				item->TriggerFlags--;
			}
			else
				KillItem(itemNumber);
		}
	}

	void UpdateLocusts(void)
	{
		float angles[2];

		for (int i = 0; i < MAX_LOCUSTS; i++)
		{
			auto* locust = &Locusts[i];
			if (locust->on)
			{
				// NOTE: not present in original TR4 code
				//if (LaraItem == nullptr)
				//    LaraItem = LaraItem;

				if ((Lara.Control.KeepLow || LaraItem->HitPoints <= 0) &&
					locust->counter >= 90 &&
					!(GetRandomControl() & 7))
				{
					locust->counter = 90;
				}

				locust->counter--;
				if (locust->counter == 0)
				{
					locust->on = false;
					break;
				}

				if (!(GetRandomControl() & 7))
				{
					locust->escapeYrot = GetRandomControl() % 640 + 128;
					locust->escapeXrot = (GetRandomControl() & 0x7F) - 64;
					locust->escapeZrot = (GetRandomControl() & 0x7F) - 64;
				}

				phd_GetVectorAngles(
					LaraItem->Pose.Position.x + 8 * locust->escapeXrot - locust->pos.Position.x,
					LaraItem->Pose.Position.y - locust->escapeYrot - locust->pos.Position.y,
					LaraItem->Pose.Position.z + 8 * locust->escapeZrot - locust->pos.Position.z,
					angles);

				int distance = SQUARE(LaraItem->Pose.Position.z - locust->pos.Position.z) + SQUARE(LaraItem->Pose.Position.x - locust->pos.Position.x);
				int square = int(sqrt(distance)) / 8;
				if (square <= 128)
				{
					if (square < 48)
						square = 48;
				}
				else
					square = 128;

				if (locust->randomRotation < square)
					locust->randomRotation += 1;
				else if (locust->randomRotation > square)
					locust->randomRotation -= 1;

				if (locust->counter > 90)
				{
					short resultYrot, resultXrot;
					int shiftYrot, shiftXrot;
					int random = locust->randomRotation * 128;
					resultYrot = angles[0] - locust->pos.Orientation.GetY();

					if (abs(resultYrot) > Angle::DegToRad(180.0f))
						resultYrot = locust->pos.Orientation.GetY() - angles[0];

					resultXrot = angles[1] - locust->pos.Orientation.GetX();

					if (abs(resultXrot) > Angle::DegToRad(180.0f))
						resultXrot = locust->pos.Orientation.GetX() - angles[0];

					shiftYrot = resultYrot / 8;
					shiftXrot = resultXrot / 8;

					if (shiftYrot > random || shiftYrot < -random)
						shiftYrot = -random;

					if (shiftXrot > random || shiftXrot < -random)
						shiftXrot = -random;

					locust->pos.Orientation.SetY(locust->pos.Orientation.GetY() + shiftYrot);
					locust->pos.Orientation.SetX(locust->pos.Orientation.GetX() + shiftXrot);
				}

				locust->pos.Position.x += locust->randomRotation * cos(locust->pos.Orientation.GetX()) * sin(locust->pos.Orientation.GetY());
				locust->pos.Position.y += locust->randomRotation * sin(-locust->pos.Orientation.GetX());
				locust->pos.Position.z += locust->randomRotation * cos(locust->pos.Orientation.GetX()) * cos(locust->pos.Orientation.GetY());
				if (ItemNearTarget(&locust->pos, LaraItem, CLICK(1) / 2))
				{
					TriggerBlood(locust->pos.Position.x, locust->pos.Position.y, locust->pos.Position.z, 2 * GetRandomControl(), 2);
					if (LaraItem->HitPoints > 0)
						LaraItem->HitPoints -= LOCUST_LARA_DAMAGE;
					// NOTE: not present in original TR4 code
					//else
					//    LaraItem->HitPoints -= LOCUST_ENTITY_DAMAGE;
				}

				if (locust->counter > 0)
					SoundEffect(SFX_TR4_LOCUSTS_LOOP, &locust->pos, NULL);
			}
		}
	}

	void DrawLocust(void)
	{
		// TODO: no render for the locusts !
	}
}
