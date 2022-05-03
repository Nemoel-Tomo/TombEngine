#include "framework.h"
#include "tr4_smallscorpion.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO SmallScorpionBiteInfo1 = { 0, 0, 0, 0 };
BITE_INFO SmallScorpionBiteInfo2 = { 0, 0, 0, 23 };

enum SmallScorionState
{
	SSCORPION_STATE_IDLE = 1,
	SSCORPION_STATE_WALK = 2,
	SSCORPION_STATE_RUN = 3,
	SSCORPION_STATE_ATTACK_1 = 4,
	SSCORPION_STATE_ATTACK_2 = 5,
	SSCORPION_STATE_DEATH_1 = 6,
	SSCORPION_STATE_DEATH_2 = 7
};

// TODO
enum SmallScorpionAnim
{

};

void InitialiseSmallScorpion(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = SSCORPION_STATE_IDLE;
	item->Animation.ActiveState = SSCORPION_STATE_IDLE;
}

void SmallScorpionControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	float angle = 0;
	float head = 0;
	float neck = 0;
	float tilt = 0;
	float joint0 = 0;
	float joint1 = 0;
	float joint2 = 0;
	float joint3 = 0;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->Animation.ActiveState != SSCORPION_STATE_DEATH_1 &&
			item->Animation.ActiveState != SSCORPION_STATE_DEATH_2)
		{
			item->Animation.AnimNumber = Objects[ID_SMALL_SCORPION].animIndex + 5;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = SSCORPION_STATE_DEATH_1;
		}
	}
	else
	{
		int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
		int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
		int laraDistance = dx * dx + dz * dz;

		if (item->AIBits & GUARD)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case SSCORPION_STATE_IDLE:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (AI.distance > pow(341, 2))
				item->Animation.TargetState = SSCORPION_STATE_WALK;
			else if (AI.bite)
			{
				creature->MaxTurn = Angle::DegToRad(6.0f);
				if (GetRandomControl() & 1)
					item->Animation.TargetState = SSCORPION_STATE_ATTACK_1;
				else
					item->Animation.TargetState = SSCORPION_STATE_ATTACK_2;
			}
			else if (!AI.ahead)
				item->Animation.TargetState = SSCORPION_STATE_RUN;
			
			break;

		case SSCORPION_STATE_WALK:
			creature->MaxTurn = Angle::DegToRad(6.0f);

			if (AI.distance >= pow(341, 2))
			{
				if (AI.distance > pow(213, 2))
					item->Animation.TargetState = SSCORPION_STATE_RUN;
			}
			else
				item->Animation.TargetState = SSCORPION_STATE_IDLE;
			
			break;

		case SSCORPION_STATE_RUN:
			creature->MaxTurn = Angle::DegToRad(8.0f);

			if (AI.distance < pow(341, 2))
				item->Animation.TargetState = SSCORPION_STATE_IDLE;
			
			break;

		case SSCORPION_STATE_ATTACK_1:
		case SSCORPION_STATE_ATTACK_2:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= Angle::DegToRad(6.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + Angle::DegToRad(6.0f));
				else
					item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() - Angle::DegToRad(6.0f));
			}
			else
				item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + AI.angle);
			
			if (!creature->Flags)
			{
				if (item->TouchBits & 0x1B00100)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 20 &&
						item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 32)
					{
						Lara.PoisonPotency += 2;
						LaraItem->HitPoints -= 20;
						LaraItem->HitStatus = true;

						float rotation;
						BITE_INFO* biteInfo;
						if (item->Animation.ActiveState == SSCORPION_STATE_ATTACK_1)
						{
							rotation = item->Pose.Orientation.GetY() + Angle::DegToRad(-180.0f);
							biteInfo = &SmallScorpionBiteInfo1;
						}
						else
						{
							rotation = item->Pose.Orientation.GetY() + Angle::DegToRad(-180.0f);
							biteInfo = &SmallScorpionBiteInfo2;
						}

						CreatureEffect2(item, biteInfo, 3, rotation, DoBloodSplat);
						creature->Flags = 1;
					}
				}
			}

			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
