#include "framework.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS SwitchBounds = 
	{
		0, 0,
		0, 0,
		0, 0,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR SwitchPos = { 0, 0, 0 };

	void SwitchControl(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		switchItem->Flags |= 0x3E00;

		if (!TriggerActive(switchItem) && !(switchItem->Flags & ONESHOT))
		{
			if (switchItem->ObjectNumber == ID_JUMP_SWITCH)
			{
				switchItem->TargetState = SWITCH_OFF;
				switchItem->Timer = 0;
				AnimateItem(switchItem);
			}
			else
			{
				switchItem->TargetState = SWITCH_ON;
				switchItem->Timer = 0;
			}
		}

		AnimateItem(switchItem);
	}

	void SwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			laraItem->ActiveState == LS_IDLE &&
			laraItem->AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->Status == ITEM_NOT_ACTIVE &&
			!(switchItem->Flags & 0x100) &&
			switchItem->TriggerFlags >= 0 ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			auto* bounds = GetBoundsAccurate(switchItem);

			if (switchItem->TriggerFlags == 3 && switchItem->ActiveState == SWITCH_ON ||
				switchItem->TriggerFlags >= 5 && switchItem->TriggerFlags <= 7 &&
				switchItem->ActiveState == SWITCH_OFF)
			{
				return;
			}

			SwitchBounds.boundingBox.X1 = bounds->X1 - 256;
			SwitchBounds.boundingBox.X2 = bounds->X2 + 256;

			if (switchItem->TriggerFlags)
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 512;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 512;

				if (switchItem->TriggerFlags == 3)
					SwitchPos.z = bounds->Z1 - 256;
				else
					SwitchPos.z = bounds->Z1 - 128;
			}
			else
			{
				SwitchBounds.boundingBox.Z1 = bounds->Z1 - 200;
				SwitchBounds.boundingBox.Z2 = bounds->Z2 + 200;
				SwitchPos.z = bounds->Z1 - 64;
			}

			if (TestLaraPosition(&SwitchBounds, switchItem, laraItem))
			{
				if (MoveLaraPosition(&SwitchPos, switchItem, laraItem))
				{
					if (switchItem->ActiveState == SWITCH_ON) /* Switch down */
					{
						if (switchItem->TriggerFlags)
						{
							laraItem->AnimNumber = LA_HOLESWITCH_ACTIVATE;
							laraItem->ActiveState = LS_HOLE;
						}
						else
						{
							laraItem->ActiveState = LS_SWITCH_UP;
							laraItem->AnimNumber = LA_WALLSWITCH_DOWN;
						}
						
						switchItem->TargetState = SWITCH_OFF;
					}
					else /* Switch up */
					{
						if (switchItem->TriggerFlags)
						{
							if (switchItem->TriggerFlags == 3)
								laraItem->AnimNumber = LA_BUTTON_LARGE_PUSH;
							else
							{
								laraItem->AnimNumber = LA_HOLESWITCH_ACTIVATE;
								laraItem->ActiveState = LS_HOLE;
							}
						}
						else
						{
							laraItem->ActiveState = LS_SWITCH_DOWN;
							laraItem->AnimNumber = LA_WALLSWITCH_UP;
						}

						switchItem->TargetState = SWITCH_ON;
					}

					ResetLaraFlex(laraItem);
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;

					AddActiveItem(itemNumber);
					switchItem->Status = ITEM_ACTIVE;
					AnimateItem(switchItem);
				}
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}

			return;
		}

		if (laraItem->ActiveState != LS_SWITCH_DOWN && laraItem->ActiveState != LS_SWITCH_UP)
			ObjectCollision(itemNumber, laraItem, coll);
	}
}
