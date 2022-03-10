#include "framework.h"
#include "tr4_sas.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/people.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/input.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_item.h"

namespace TEN::Entities::TR4
{
	BITE_INFO SASGunBite = { 0, 300, 64, 7 };

	PHD_VECTOR SASDragBodyPosition = { 0, 0, -460 };
	OBJECT_COLLISION_BOUNDS SASDragBodyBounds =
	{
		-256, 256,
		-64, 100,
		-200, -460,
		ANGLE(-10.0f), ANGLE(10.0f),
		ANGLE(-30.0f), ANGLE(30.0f),
		0, 0
	};

	enum SASState
	{
		SAS_STATE_NONE = 0,
		SAS_STATE_IDLE = 1,
		SAS_STATE_WALK = 2,
		SAS_STATE_RUN = 3,
		SAS_STATE_WAIT = 4,
		SAS_STATE_SIGHT_SHOOT = 5,
		SAS_STATE_WALK_SHOOT = 6,
		SAS_STATE_DEATH = 7,
		SAS_STATE_SIGHT_AIM = 8,
		SAS_STATE_WALK_AIM = 9,
		SAS_STATE_HOLD_AIM = 10,
		SAS_STATE_HOLD_SHOOT = 11,
		SAS_STATE_KNEEL_AIM = 12,
		SAS_STATE_KNEEL_SHOOT = 13,
		SAS_STATE_KNEEL_STOP = 14,
		SAS_STATE_HOLD_PREPARE_GRENADE = 15,
		SAS_STATE_HOLD_SHOOT_GRENADE = 16,
		SAS_STATE_BLIND = 17
	};

	enum SASAnim
	{
		SAS_ANIM_WALK = 0,
		SAS_ANIM_RUN = 1,
		SAS_ANIM_SIGHT_SHOOT = 2,
		SAS_ANIM_STAND_TO_SIGHT_AIM = 3,
		SAS_ANIM_WALK_TO_WALK_AIM = 4,
		SAS_ANIM_RUN_TO_WALK = 5,
		SAS_ANIM_WALK_SHOOT = 6,
		SAS_ANIM_SIGHT_AIM_TO_STAND = 7,
		SAS_ANIM_WALK_AIM_TO_STAND = 8,
		SAS_ANIM_STAND_TO_RUN = 9,
		SAS_ANIM_STAND_TO_WAIT = 10,
		SAS_ANIM_STAND_TO_WALK = 11,
		SAS_ANIM_STAND = 12,
		SAS_ANIM_WAIT_TO_STAND = 13,
		SAS_ANIM_WAIT = 14,
		SAS_ANIM_WAIT_TO_SIGHT_AIM = 15,
		SAS_ANIM_WALK_TO_RUN = 16,
		SAS_ANIM_WALK_TO_STAND = 17,
		SAS_ANIM_WALK_AIM_TO_WALK = 18,
		SAS_ANIM_DEATH = 19,
		SAS_ANIM_STAND_TO_HOLD_AIM = 20,
		SAS_ANIM_HOLD_SHOOT = 21,
		SAS_ANIM_HOLD_AIM_TO_STAND = 22,
		SAS_ANIM_HOLD_PREPARE_GRENADE = 23,
		SAS_ANIM_HOLD_SHOOT_GRENADE = 24,
		SAS_ANIM_STAND_TO_KNEEL_AIM = 25,
		SAS_ANIM_KNEEL_SHOOT = 26,
		SAS_ANIM_KNEEL_AIM_TO_STAND = 27,
		SAS_ANIM_BLIND = 28,
		SAS_ANIM_BLIND_TO_STAND = 29
	};

	void InitialiseSas(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->AnimNumber = Objects[item->ObjectNumber].animIndex + SAS_ANIM_STAND;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = SAS_STATE_IDLE;
		item->ActiveState = SAS_STATE_IDLE;
	}

	void SasControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = (CreatureInfo*)item->Data;
		auto* enemy = creature->Enemy;

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		// Handle SAS firing.
		if (creature->FiredWeapon)
		{
			PHD_VECTOR pos = { SASGunBite.x, SASGunBite.y, SASGunBite.z };
			GetJointAbsPosition(item, &pos, SASGunBite.meshNum);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 24, 16, 4);
			creature->FiredWeapon--;
		}

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance = 0;
			int angle = 0;
			if (creature->Enemy == LaraItem)
			{
				angle = AI.angle;
				distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
				int ang = phd_atan(dz, dx) - item->Position.yRot;
				distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, creature->Enemy != LaraItem);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && AI.bite)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, creature->Enemy != LaraItem);
			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus)
				AlertAllGuards(itemNumber);

			int angle1 = 0;
			int angle2 = 0;

			switch (item->ActiveState)
			{
			case SAS_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				joint2 = angle;

				if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + SAS_ANIM_WALK_TO_STAND)
				{
					if (abs(AI.angle) >= ANGLE(10.0f))
					{
						if (AI.angle >= 0)
							item->Position.yRot += ANGLE(10.0f);
						else
							item->Position.yRot -= ANGLE(10.0f);
					}
					else
						item->Position.yRot += AI.angle;
				}
				else if (item->AIBits & MODIFY || Lara.Vehicle != NO_ITEM)
				{
					if (abs(AI.angle) >= ANGLE(2.0f))
					{
						if (AI.angle >= 0)
							item->Position.yRot += ANGLE(2.0f);
						else
							item->Position.yRot -= ANGLE(2.0f);
					}
					else
						item->Position.yRot += AI.angle;
				}

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);

					if (!GetRandomControl())
					{
						if (item->ActiveState == SAS_STATE_IDLE)
						{
							item->TargetState = SAS_STATE_WAIT;
							break;
						}

						item->TargetState = SAS_STATE_IDLE;
					}
				}
				else if (!(item->AIBits & PATROL1) ||
					item->AIBits & MODIFY ||
					Lara.Vehicle != NO_ITEM)
				{
					if (Targetable(item, &AI))
					{
						if (AI.distance < pow(SECTOR(3), 2) ||
							AI.zoneNumber != AI.enemyZone)
						{
							if (GetRandomControl() & 1)
								item->TargetState = SAS_STATE_SIGHT_AIM;
							else if (GetRandomControl() & 1)
								item->TargetState = SAS_STATE_HOLD_AIM;
							else
								item->TargetState = SAS_STATE_KNEEL_AIM;
						}
						else if (!(item->AIBits & MODIFY))
							item->TargetState = SAS_STATE_WALK;
					}
					else
					{
						if (item->AIBits & MODIFY)
							item->TargetState = SAS_STATE_IDLE;
						else
						{
							if (creature->Mood == MoodType::Escape)
								item->TargetState = SAS_STATE_RUN;
							else
							{
								if ((creature->Alerted ||
									creature->Mood != MoodType::Bored) &&
									(!(item->AIBits & FOLLOW) ||
										!creature->ReachedGoal &&
										distance <= pow(SECTOR(2), 2)))
								{
									if (creature->Mood == MoodType::Bored ||
										AI.distance <= pow(SECTOR(2), 2))
									{
										item->TargetState = SAS_STATE_WALK;
										break;
									}
									item->TargetState = SAS_STATE_RUN;
								}
								else
									item->TargetState = SAS_STATE_IDLE;
							}
						}
					}
				}
				else
				{
					item->TargetState = SAS_STATE_WALK;
					joint2 = 0;
				}

				break;

			case SAS_STATE_WAIT:
				creature->Flags = 0;
				creature->MaxTurn = 0;
				joint2 = angle;

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);

					if (!GetRandomControl())
						item->TargetState = SAS_STATE_IDLE;
				}
				else if (Targetable(item, &AI) ||
					creature->Mood != MoodType::Bored ||
					!AI.ahead ||
					item->AIBits & MODIFY ||
					Lara.Vehicle != NO_ITEM)
				{
					item->TargetState = SAS_STATE_IDLE;
				}

				break;

			case SAS_STATE_WALK:
				creature->MaxTurn = ANGLE(5.0f);
				creature->Flags = 0;
				joint2 = angle;

				if (item->AIBits & PATROL1)
					item->TargetState = SAS_STATE_WALK;
				else if (Lara.Vehicle == NO_ITEM ||
					!(item->AIBits & MODIFY) &&
					item->AIBits)
				{
					if (creature->Mood == MoodType::Escape)
						item->TargetState = SAS_STATE_RUN;
					else
					{
						if (item->AIBits & GUARD ||
							item->AIBits & FOLLOW &&
							(creature->ReachedGoal ||
								distance > pow(SECTOR(2), 2)))
						{
							item->TargetState = SAS_STATE_IDLE;
							break;
						}
						if (Targetable(item, &AI))
						{
							if (AI.distance < pow(SECTOR(3), 2) ||
								AI.enemyZone != AI.zoneNumber)
							{
								item->TargetState = SAS_STATE_IDLE;
								break;
							}

							item->TargetState = SAS_STATE_WALK_AIM;
						}
						else if (creature->Mood != MoodType::Bored)
						{
							if (AI.distance > pow(SECTOR(2), 2))
								item->TargetState = SAS_STATE_RUN;
						}
						else if (AI.ahead)
						{
							item->TargetState = SAS_STATE_IDLE;
							break;
						}
					}
				}
				else
					item->TargetState = SAS_STATE_IDLE;
				
				break;

			case SAS_STATE_RUN:
				creature->MaxTurn = ANGLE(10.0f);
				tilt = angle / 2;

				if (AI.ahead)
					joint2 = AI.angle;

				if (Lara.Vehicle != NO_ITEM)
				{
					if (item->AIBits & MODIFY || !item->AIBits)
					{
						item->TargetState = SAS_STATE_WAIT;
						break;
					}
				}

				if (item->AIBits & GUARD ||
					item->AIBits & FOLLOW &&
					(creature->ReachedGoal ||
						distance > pow(SECTOR(2), 2)))
				{
					item->TargetState = SAS_STATE_WALK;
					break;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->TargetState = SAS_STATE_WALK;
					else
					{
						if (creature->Mood != MoodType::Bored ||
							creature->Mood == MoodType::Stalk &&
							item->AIBits & FOLLOW &&
							AI.distance < pow(SECTOR(2), 2))
						{
							item->TargetState = SAS_STATE_WALK;
						}
					}
				}

				break;

			case SAS_STATE_SIGHT_AIM:
			case SAS_STATE_HOLD_AIM:
			case SAS_STATE_KNEEL_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;

					if (Targetable(item, &AI))
					{
						if (item->ActiveState == SAS_STATE_SIGHT_AIM)
							item->TargetState = SAS_STATE_SIGHT_SHOOT;
						else if (item->ActiveState == SAS_STATE_KNEEL_AIM)
							item->TargetState = SAS_STATE_KNEEL_SHOOT;
						else if (GetRandomControl() & 1)
							item->TargetState = SAS_STATE_HOLD_SHOOT;
						else
							item->TargetState = SAS_STATE_HOLD_PREPARE_GRENADE;
					}
					else
						item->TargetState = SAS_STATE_IDLE;
				}

				break;

			case SAS_STATE_WALK_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;

					if (Targetable(item, &AI))
						item->TargetState = SAS_STATE_WALK_SHOOT;
					else
						item->TargetState = SAS_STATE_WALK;
				}

				break;

			case SAS_STATE_HOLD_PREPARE_GRENADE:
				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;
				}

				break;

			case SAS_STATE_HOLD_SHOOT_GRENADE:
				if (AI.ahead)
				{
					angle1 = AI.angle;
					angle2 = AI.xAngle;
					joint0 = AI.angle;
					joint1 = AI.xAngle;

					if (AI.distance > pow(SECTOR(3), 2))
					{
						angle2 = sqrt(AI.distance) + AI.xAngle - ANGLE(5.6f);
						joint1 = angle2;
					}
				}
				else
				{
					angle1 = 0;
					angle2 = 0;
				}

				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 20)
				{
					if (!creature->Enemy->Velocity)
					{
						angle1 += (GetRandomControl() & 0x1FF) - 256;
						angle2 += (GetRandomControl() & 0x1FF) - 256;
						joint0 = angle1;
						joint1 = angle2;
					}

					SasFireGrenade(item, angle2, angle1);

					if (Targetable(item, &AI))
						item->TargetState = SAS_STATE_HOLD_PREPARE_GRENADE;
				}

				break;

			case SAS_STATE_HOLD_SHOOT:
			case SAS_STATE_KNEEL_SHOOT:
			case SAS_STATE_SIGHT_SHOOT:
			case SAS_STATE_WALK_SHOOT:
				if (item->ActiveState == SAS_STATE_HOLD_SHOOT ||
					item->ActiveState == SAS_STATE_KNEEL_SHOOT)
				{
					if (item->TargetState != SAS_STATE_IDLE &&
						item->TargetState != SAS_STATE_KNEEL_STOP &&
						(creature->Mood == MoodType::Escape ||
							!Targetable(item, &AI)))
					{
						if (item->ActiveState == SAS_STATE_HOLD_SHOOT)
							item->TargetState = SAS_STATE_IDLE;
						else
							item->TargetState = SAS_STATE_KNEEL_STOP;
					}
				}

				if (AI.ahead)
				{
					joint0 = AI.angle;
					joint1 = AI.xAngle;
				}

				if (creature->Flags)
					creature->Flags -= 1;
				else
				{
					ShotLara(item, &AI, &SASGunBite, joint0, 15);
					creature->Flags = 5;
					creature->FiredWeapon = 3;
				}

				break;

			case SAS_STATE_BLIND:
				if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
					item->TargetState = SAS_STATE_WAIT;

				break;

			default:
				break;
			}

			if (WeaponEnemyTimer > 100 &&
				item->ActiveState != SAS_STATE_BLIND)
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + SAS_ANIM_BLIND;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + (GetRandomControl() & 7);
				item->ActiveState = SAS_STATE_BLIND;
				creature->MaxTurn = 0;
			}
		}
		else
		{
			if (item->ActiveState != SAS_STATE_DEATH)
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + SAS_ANIM_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = SAS_STATE_DEATH;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		CreatureAnimation(itemNumber, angle, 0);
	}

	void SasFireGrenade(ITEM_INFO* item, short angle1, short angle2)
	{
		short itemNumber = CreateItem();
		if (itemNumber != NO_ITEM)
		{
			auto* grenadeItem = &g_Level.Items[itemNumber];

			grenadeItem->Shade = -15856;
			grenadeItem->ObjectNumber = ID_GRENADE;
			grenadeItem->RoomNumber = item->RoomNumber;

			PHD_VECTOR pos = { SASGunBite.x, SASGunBite.y, SASGunBite.z };
			GetJointAbsPosition(item, &pos, SASGunBite.meshNum);

			grenadeItem->Position.xPos = pos.x;
			grenadeItem->Position.yPos = pos.y;
			grenadeItem->Position.zPos = pos.z;

			auto probe = GetCollisionResult(pos.x, pos.y, pos.z, grenadeItem->RoomNumber);
			grenadeItem->RoomNumber = probe.RoomNumber;

			if (probe.Position.Floor < grenadeItem->Position.yPos)
			{
				grenadeItem->Position.xPos = item->Position.xPos;
				grenadeItem->Position.yPos = probe.Position.Floor;
				grenadeItem->Position.zPos = item->Position.zPos;
				grenadeItem->RoomNumber = item->RoomNumber;
			}

			SmokeCountL = 32;
			SmokeWeapon = (LaraWeaponType)5; // TODO: 5 is the HK. Did TEN's enum get shuffled around? @Sezz 2022.03.09

			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, (LaraWeaponType)5, 32);

			InitialiseItem(itemNumber);

			grenadeItem->Position.xRot = angle1 + item->Position.xRot;
			grenadeItem->Position.yRot = angle2 + item->Position.yRot;
			grenadeItem->Position.zRot = 0;

			if (GetRandomControl() & 3)
				grenadeItem->ItemFlags[0] = (int)GrenadeType::Normal;
			else
				grenadeItem->ItemFlags[0] = (int)GrenadeType::Super;

			grenadeItem->ItemFlags[2] = 1;
			grenadeItem->Velocity = 128;
			grenadeItem->ActiveState = grenadeItem->Position.xRot;
			grenadeItem->VerticalVelocity = -128 * phd_sin(grenadeItem->Position.xRot);
			grenadeItem->TargetState = grenadeItem->Position.yRot;
			grenadeItem->RequiredState = 0;
			grenadeItem->HitPoints = 120;

			AddActiveItem(itemNumber);
		}
	}

	void InitialiseInjuredSas(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->TargetState = item->ActiveState = 1;
		}
		else
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
			item->TargetState = item->ActiveState = 4;
		}

		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	}

	void InjuredSasControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->ActiveState == 1)
		{
			if (!(GetRandomControl() & 0x7F))
			{
				item->TargetState = 2;
				AnimateItem(item);
			}
			else if (!(byte)GetRandomControl())
				item->TargetState = 3;
		}
		else if (item->ActiveState == 4 && !(GetRandomControl() & 0x7F))
		{
			item->TargetState = 5;
			AnimateItem(item);
		}
		else
			AnimateItem(item);
	}

	void SasDragBlokeCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if ((!(TrInput & IN_ACTION) ||
			laraItem->Airborne ||
			laraItem->ActiveState != LS_IDLE ||
			laraItem->AnimNumber != LA_STAND_IDLE ||
			Lara.Control.HandStatus != HandStatus::Free ||
			item->Flags & 0x3E00) &&
			(!(Lara.Control.IsMoving) || Lara.InteractedItem != itemNumber))
		{
			if (item->Status == ITEM_ACTIVE)
			{
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				{
					int x = laraItem->Position.xPos - 512 * phd_sin(laraItem->Position.yRot);
					int y = laraItem->Position.yPos;
					int z = laraItem->Position.zPos - 512 * phd_cos(laraItem->Position.yRot);

					TestTriggers(x, y, z, laraItem->RoomNumber, true);

					RemoveActiveItem(itemNumber);
					item->Status = ITEM_NOT_ACTIVE;
				}
			}

			ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			if (TestLaraPosition(&SASDragBodyBounds, item, laraItem))
			{
				if (MoveLaraPosition(&SASDragBodyPosition, item, laraItem))
				{
					laraItem->AnimNumber = LA_DRAG_BODY;
					laraItem->ActiveState = LS_MISC_CONTROL;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					laraItem->Position.yRot = item->Position.yRot;
					ResetLaraFlex(laraItem);
					Lara.Control.IsMoving = false;
					Lara.Control.HandStatus = HandStatus::Busy;
					item->Flags |= 0x3E00;
					item->Status = ITEM_ACTIVE;
					AddActiveItem(itemNumber);
				}
				else
					Lara.InteractedItem = itemNumber;
			}
		}
	}
}
