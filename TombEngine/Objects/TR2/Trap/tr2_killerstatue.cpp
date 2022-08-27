#include "framework.h"
#include "Objects/TR2/Trap/tr2_killerstatue.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::Traps
{
	constexpr auto KILLER_STATUE_DAMAGE = 20;

	const vector<int> KillerStatueDamageJoints = { 7 };

	enum KillerStatueState
	{
		KSTATUE_STATE_SWORD_UP = 1,
		KSTATUE_STATE_SWORD_DOWN = 2,
	};

	enum KillerStatueAnim
	{
		KSTATUE_ANIM_LIFT_SWORD = 0,
		KSTATUE_ANIM_SWORD_DOWN = 1,
		KSTATUE_ANIM_SWING_SWORD = 2,
		KSTATUE_ANIM_SWORD_UP = 3
	};

	void InitialiseKillerStatue(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		SetAnimation(item, KSTATUE_ANIM_SWORD_UP);
	}

	void KillerStatueControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item) && item->Animation.ActiveState == KSTATUE_STATE_SWORD_UP)
			item->Animation.TargetState = KSTATUE_STATE_SWORD_DOWN;
		else
			item->Animation.TargetState = KSTATUE_STATE_SWORD_UP;

		if (item->TestBits(JointBitType::Touch, KillerStatueDamageJoints) &&
			item->Animation.ActiveState == KSTATUE_STATE_SWORD_DOWN)
		{
			DoDamage(LaraItem, KILLER_STATUE_DAMAGE);

			auto pos = Vector3Int(
				LaraItem->Pose.Position.x + (GetRandomControl() - SECTOR(16)) / CLICK(1),
				LaraItem->Pose.Position.z + (GetRandomControl() - SECTOR(16)) / CLICK(1),
				LaraItem->Pose.Position.y - GetRandomControl() / 44
			);
			int angle = (GetRandomControl() - SECTOR(16)) / 8 + LaraItem->Pose.Orientation.y;
			DoBloodSplat(pos.x, pos.y, pos.z, LaraItem->Animation.Velocity.z, angle, LaraItem->RoomNumber);
		}

		AnimateItem(item);
	}
}
