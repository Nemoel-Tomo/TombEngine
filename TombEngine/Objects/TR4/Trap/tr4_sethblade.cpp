#include "framework.h"
#include "tr4_sethblade.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	constexpr auto SETH_BLADE_DAMAGE = 1000;

	enum SethBladeState 
	{
		SETHBLADE_STATE_NONE = 0,
		SETHBLADE_STATE_ACTIVE = 1,
		SETHBLADE_STATE_IDLE = 2,
	};

	enum SethBladeAnim 
	{
		SETHBLADE_ANIM_ACTIVATE = 0,
		SETHBLADE_ANIM_IDLE = 1,
	};

	void InitialiseSethBlade(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SETHBLADE_ANIM_IDLE;
		item->Animation.TargetState = SETHBLADE_STATE_IDLE;
		item->Animation.ActiveState = SETHBLADE_STATE_IDLE;

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

		if (item->TriggerFlags == 0)
			item->TriggerFlags = -1;

		item->ItemFlags[2] = abs(item->TriggerFlags);
		item->ItemFlags[3] = SETH_BLADE_DAMAGE;
	}

	void SethBladeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		*((int*)&item->ItemFlags) = 0;

		if (!TriggerActive(item))
			return;

		if (item->Animation.ActiveState == SETHBLADE_STATE_IDLE)
		{
			if (item->ItemFlags[2] > 1)
				item->ItemFlags[2]--;
			else if (item->ItemFlags[2] == 1)
			{
				item->Animation.TargetState = SETHBLADE_STATE_ACTIVE;
				if (item->TriggerFlags > 0)
					item->ItemFlags[2] = item->TriggerFlags;
				else
					item->ItemFlags[2] = 0;
			}
		}
		else
		{
			int frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			if (frameNumber >= 0 && frameNumber <= 6)
				*((int*)&item->ItemFlags) = -1;
			else if (frameNumber >= 7 && frameNumber <= 15)
				*((int*)&item->ItemFlags) = 448;
			else
				*((int*)&item->ItemFlags) = 0;
		}

		AnimateItem(item);
	}
}
