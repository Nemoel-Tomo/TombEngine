#include "framework.h"
#include "Objects/TR4/Entity/tr4_von_croy.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::TR4
{
	constexpr auto VON_CROY_ATTACK_DAMAGE = 20;
	constexpr auto VON_CROY_ATTACK_RANGE = 0x64000;
	constexpr auto VON_CROY_MONKEYSWING_JUMP_HEIGHT = 1536;

	const auto VonCroyBite = BiteInfo(Vector3(0.0f, 35.0f, 130.0f), 18);
	const vector<int> VonCroyKnifeSwapJoints = { 7, 18 };
	const vector<int> VonCroyBookJoints = { 15 };

	enum VonCroyState
	{
		VON_CROY_STATE_IDLE = 1,
		VON_CROY_STATE_WALK = 2,
		VON_CROY_STATE_RUN = 3,
		VON_CROY_STATE_START_MONKEY = 4,
		VON_CROY_STATE_MONKEY = 5,
		VON_CROY_STATE_TOGGLE_KNIFE = 6,
		VON_CROY_STATE_LOOK_BEFORE_JUMP = 7,
		VON_CROY_STATE_TALK_1 = 8,
		VON_CROY_STATE_TALK_2 = 9,
		VON_CROY_STATE_TALK_3 = 10,
		VON_CROY_STATE_IGNITE = 11, // Not sure about this one as the animation that uses it (30) is the same as GUIDE's torch ignite. -- Kubsy 2022.06.18
		VON_CROY_STATE_STOP_LARA = 12,
		VON_CROY_STATE_CALL_LARA_1 = 13,
		VON_CROY_STATE_CALL_LARA_2 = 14,
		VON_CROY_STATE_JUMP_1_BLOCK = 15,
		VON_CROY_STATE_JUMP_2_BLOCKS = 16,
		VON_CROY_STATE_CLIMB_4_BLOCKS = 17,
		VON_CROY_STATE_CLIMB_3_BLOCKS = 18,
		VON_CROY_STATE_CLIMB_2_BLOCKS = 19,
		VON_CROY_STATE_ENABLE_TRAP = 20,
		VON_CROY_STATE_KNIFE_ATTACK_HIGH = 21,
		VON_CROY_STATE_LOOK_BACK_LEFT = 22,
		VON_CROY_STATE_JUMP_DOWN_2_CLICKS = 23,
		VON_CROY_STATE_JUMP_DOWN_3_CLICKS = 24,
		VON_CROY_STATE_JUMP_DOWN_4_CLICKS = 25,
		VON_CROY_STATE_STEP_DOWN_HIGH = 26,
		VON_CROY_STATE_GRAB_BORDER = 27,
		VON_CROY_STATE_SHIMMY_BORDER_RIGHT = 28,
		VON_CROY_STATE_SHIMMY_BORDER_LEFT = 29, //New one, not animations in original assets.
		VON_CROY_STATE_LADDER_CLIMB_UP = 30,
		VON_CROY_STATE_KNIFE_ATTACK_LOW = 31,
		VON_CROY_STATE_POINT = 32,
		VON_CROY_STATE_STANDING_JUMP_GRAB = 33,
		VON_CROY_STATE_JUMP_BACK = 34,
		VON_CROY_STATE_LOOK_BACK_RIGHT = 35,
		VON_CROY_STATE_ADJUST_POSITION_FRONT = 36,
		VON_CROY_STATE_ADJUST_POSITION_BACK = 37
	};

	enum VonCroyAnim
	{
		VON_CROY_ANIM_WALK_FORWARD = 0,
		VON_CROY_ANIM_RUN_FORWARD = 1,
		VON_CROY_ANIM_MONKEY_FORWARD = 2,
		VON_CROY_ANIM_USE_SWITCH = 3,
		VON_CROY_ANIM_IDLE = 4,
		VON_CROY_ANIM_ATTACK_HIGH = 5,
		VON_CROY_ANIM_MONKEY_TO_FORWARD = 6,
		VON_CROY_ANIM_MONKEY_IDLE = 7,
		VON_CROY_ANIM_MONKEY_IDLE_TO_FORWARD = 8,
		VON_CROY_ANIM_MONKEY_STOP = 9,
		VON_CROY_ANIM_MONKEY_FALL = 10,
		VON_CROY_ANIM_KNIFE_EQUIP_UNEQUIP = 11,
		VON_CROY_ANIM_GROUND_INSPECT = 12,
		VON_CROY_ANIM_IDLE_TO_WALK = 13,
		VON_CROY_ANIM_WALK_STOP = 14,
		VON_CROY_ANIM_IDLE_TO_RUN = 15,
		VON_CROY_ANIM_RUN_STOP = 16,
		VON_CROY_ANIM_WALK_TO_RUN = 17,
		VON_CROY_ANIM_RUN_TO_WALK = 18,
		VON_CROY_ANIM_TALKING1 = 19,
		VON_CROY_ANIM_TALKING2 = 20,
		VON_CROY_ANIM_TALKING3 = 21,
		VON_CROY_ANIM_IDLE_TO_JUMP = 22,
		VON_CROY_ANIM_JUMP_1_BLOCK = 23,
		VON_CROY_ANIM_JUMP_LAND = 24,
		VON_CROY_ANIM_JUMP_2_BLOCKS = 25,
		VON_CROY_ANIM_LEFT_TURN = 26,
		VON_CROY_ANIM_CLIMB_4_BLOCKS = 27,
		VON_CROY_ANIM_CLIMB_3_BLOCKS = 28,
		VON_CROY_ANIM_CLIMB_2_BLOCKS = 29,
		VON_CROY_ANIM_IDLE_TO_CROUCH = 30,
		VON_CROY_ANIM_LARA_INTERACT_COME_DISTANT = 31,
		VON_CROY_ANIM_LARA_INTERACT_COME_CLOSE = 32,
		VON_CROY_ANIM_LARA_INTERACT_STOP = 33,
		VON_CROY_ANIM_LARA_INTERACT_STOP_TO_COME = 34,
		VON_CROY_ANIM_CLIMB_DOWN_1_SECTOR = 35,
		VON_CROY_ANIM_CLIMB_DOWN_2_SECTORS = 36,
		VON_CROY_ANIM_JUMP_TO_HANG = 37,
		VON_CROY_ANIM_SHIMMY_TO_THE_RIGHT = 38,
		VON_CROY_ANIM_CLIMB = 39,
		VON_CROY_ANIM_DROP_FROM_HANG = 40,
		VON_CROY_ANIM_CLIMB_OFF_3_CLICKS = 41,
		VON_CROY_ANIM_CLIMB_OFF_2_CLICKS = 42,
		VON_CROY_ANIM_HANG = 43,
		VON_CROY_ANIM_ATTACK_LOW = 44,
		VON_CROY_ANIM_RUNNING_JUMP_RIGHT_FOOT = 45,
		VON_CROY_ANIM_RUNNING_JUMP_LEFT_FOOT = 46,
		VON_CROY_ANIM_START_POINT = 47,
		VON_CROY_ANIM_POINTING = 48,
		VON_CROY_ANIM_STOP_POINTING = 49, 
		VON_CROY_ANIM_RUNNING_JUMP = 50,
		VON_CROY_ANIM_RUNNING_JUMP_TO_GRAB = 51,
		VON_CROY_ANIM_CLIMB_UP_AFTER_JUMP = 52,
		VON_CROY_ANIM_STANDING_JUMP_BACK_START = 53,
		VON_CROY_ANIM_STANDING_JUMP_BACK = 54,
		VON_CROY_ANIM_STANDING_JUMP_BACK_END = 55,
		VON_CROY_ANIM_TURN_TO_THE_RIGHT = 56,
		VON_CROY_ANIM_ALIGN_FRONT = 57,
		VON_CROY_ANIM_ALIGN_BACK = 58,
		VON_CROY_ANIM_LAND_TO_RUN = 59
	};

	enum VonCroyJumps
	{
		NO_JUMP = 0,
		JUMP_1_SECTOR = 1,
		JUMP_2_SECTORS = 2,
		JUMP_3_SECTORS = 3
	};

	void InitialiseVonCroy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_KNIFE_EQUIP_UNEQUIP;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = VON_CROY_STATE_TOGGLE_KNIFE;
		item->Animation.ActiveState = VON_CROY_STATE_TOGGLE_KNIFE;
		item->SetBits(JointBitType::MeshSwap, VonCroyKnifeSwapJoints);

		item->HitPoints = NOT_TARGETABLE;
	}

	void VonCroyControl(short itemNumber)
	{
		// General Initialization
		// AI Initialization
		// Update AI data
		// Update laraAI data
		// Search target
		// TEN flags
		// States Behaviours
		//----------

		//General Initialization
		auto* item = &g_Level.Items[itemNumber];

		if (!CreatureActive(itemNumber))
			return;

		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short tilt = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		VonCroyJumps currentJump = VonCroyJumps::NO_JUMP;
		bool canMonkeySwing = false;

		// AI Initialization
		item->AIBits = FOLLOW;
		GetAITarget(creature);

		AI_INFO AI;
		AI_INFO laraAI;

		int dx;
		int dy;
		int dz;
				
		CollisionResult probe;

		// Update AI data
		if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_DOWN_2_SECTORS ||
			item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_UP_AFTER_JUMP)
		{
			//Update room.
			short oldRoom = item->RoomNumber;
			item->Pose.Position.x += dx;
			item->Pose.Position.z += dz;

			GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &item->RoomNumber);

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + VON_CROY_ANIM_RUN_FORWARD)
				CreateZone(item);

			CreatureAIInfo(item, &AI);

			item->RoomNumber = oldRoom;
			item->Pose.Position.x -= dx;
			item->Pose.Position.z -= dz;
		}
		else
			CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, true);
		CreatureMood(item, &AI, true);

		// Update laraAI data
		if (creature->Enemy->IsLara())
			memcpy(&laraAI, &AI, sizeof(AI_INFO));
		else
		{
			dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

			laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;

			laraAI.ahead = true;
			if (laraAI.angle <= -ANGLE(90.0f) || laraAI.angle >= ANGLE(90.0f))
				laraAI.ahead = false;

			int distance = 0;
			if (dx > SECTOR(31.25f) || dx < -SECTOR(31.25f) ||
				dz > SECTOR(31.25f) || dz < -SECTOR(31.25f))
			{
				laraAI.distance = INT_MAX;
			}
			else
				laraAI.distance = pow(dx, 2) + pow(dz, 2);

			/* Does it really need this?
			dx = abs(dx);
			dz = abs(dz);

			int dy = item->Pose.Position.y - LaraItem->Pose.Position.y;

			if (dx <= dz)
				laraAI.xAngle = phd_atan(dz + (dx / 2), dy);
			else
				laraAI.xAngle = phd_atan(dx + (dz / 2), dy);
			*/
		}

		angle = CreatureTurn(item, creature->MaxTurn);

		//Search target
		ItemInfo* foundEnemy = nullptr;
		
		if (true)
		{
			int distance = 0;
			int minDistance = INT_MAX;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreatureInfo = ActiveCreatures[i];

				if (currentCreatureInfo->ItemNumber == NO_ITEM ||
					currentCreatureInfo->ItemNumber == itemNumber)
				{
					continue;
				}

				auto* currentItem = &g_Level.Items[currentCreatureInfo->ItemNumber];
				
				if (currentItem->ObjectNumber != ID_VON_CROY &&
					abs(currentItem->Pose.Position.y - item->Pose.Position.y) <= 512)
				{
					dx = currentItem->Pose.Position.x - item->Pose.Position.x;
					dy = currentItem->Pose.Position.y - item->Pose.Position.y;
					dz = currentItem->Pose.Position.z - item->Pose.Position.z;

					if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
						distance = 0x7FFFFFFF;
					else
						distance = pow(dx, 2) + pow(dz, 2);

					if (distance < minDistance &&
						distance < pow(SECTOR(2), 2) &&
						(abs(dy) < CLICK(1) ||
							laraAI.distance < pow(SECTOR(2), 2) ||
							currentItem->ObjectNumber == ID_DOG)) // Here to add more entities as target.
					{
						foundEnemy = currentItem;
						minDistance = distance;
					}
				}
			}
		}
		auto* enemy = creature->Enemy;
		if (foundEnemy)
		{
			creature->Enemy = enemy;
			enemy = foundEnemy;
		}


		// TEN Flags
		bool flagNewBehaviour = ((item->ItemFlags[2] & (1 << 0)) != 0);
		bool flagIgnoreLaraDistance = ((item->ItemFlags[2] & (1 << 1)) != 0);
		bool flagRunDefault = ((item->ItemFlags[2] & (1 << 2)) != 0);
		bool flagRetryNodeSearch = ((item->ItemFlags[2] & (1 << 3)) != 0);
		bool flagJump = ((item->ItemFlags[2] & (1 << 4)) != 0);
		bool flagTalk = ((item->ItemFlags[2] & (1 << 5)) != 0);
		bool flagBook = ((item->ItemFlags[2] & (1 << 6)) != 0);
		bool flagLookLara = ((item->ItemFlags[2] & (1 << 7)) != 0);
		
		short goalNode = (flagNewBehaviour) ? item->ItemFlags[4] : Lara.Location;

		if (flagRetryNodeSearch)
		{
			item->ItemFlags[2] &= ~(1 << 3); // Turn off 3rd bit for flagRetryNodeSearch.
			creature->ReachedGoal = false;
			creature->Enemy = nullptr;
		}


		// States Behaviours
		switch (item->Animation.ActiveState)
		{
		case VON_CROY_STATE_IDLE:
			creature->LOT.IsJumping = false;
			creature->LOT.IsMonkeying = false;
			creature->MaxTurn = 0;
			creature->Flags = 0;
			joint3 = AI.angle / 2;

			if (laraAI.ahead)
			{
				joint0 = laraAI.angle / 2;
				joint1 = laraAI.xAngle / 2;
				joint2 = laraAI.angle / 2;
			}
			else if (AI.ahead)
			{
				joint0 = AI.angle / 2;
				joint1 = AI.xAngle / 2;
				joint2 = AI.angle / 2;
			}

			if (goalNode < item->ItemFlags[3])
			{
				item->Animation.TargetState = VON_CROY_STATE_IDLE;
				break;
			}

			if (creature->ReachedGoal)
			{
				if (!enemy->Flags)
				{
					//Prepare the data for the next node search.
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					break;
				}

				if (AI.distance <= pow(CLICK(0.5f), 2))
				{
					switch (enemy->Flags)
					{
						case 0x02: //Codebit: 1						Shimmy Right
							/*item->current_anim_state = 29;
							item->anim_number = objects[VON_CROY].anim_index + 37;
							item->frame_number = anims[item->anim_number].frame_base;
							item->pos.x_pos = oEnemy->pos.x_pos;
							item->pos.y_pos = oEnemy->pos.y_pos;
							item->pos.z_pos = oEnemy->pos.z_pos;
							item->pos.x_rot = oEnemy->pos.x_rot;
							item->pos.y_rot = oEnemy->pos.y_rot;
							item->pos.z_rot = oEnemy->pos.z_rot;
							ifl3 = 1;
							VonCroy->reached_goal = 0;
							VonCroy->enemy = 0;
							item->item_flags[3] += ifl3;
							item->ai_bits = FOLLOW;*/
							break;
						case 0x04: //Codebit: 2						Climb > 4 clicks
							/*item->current_anim_state = 26;
							item->anim_number = objects[VON_CROY].anim_index + 36;
							item->frame_number = anims[item->anim_number].frame_base;
							VonCroy->LOT.is_jumping = 1;
							item->pos.x_pos = oEnemy->pos.x_pos;
							item->pos.y_pos = oEnemy->pos.y_pos;
							item->pos.z_pos = oEnemy->pos.z_pos;
							item->pos.x_rot = oEnemy->pos.x_rot;
							item->pos.y_rot = oEnemy->pos.y_rot;
							item->pos.z_rot = oEnemy->pos.z_rot;
							ifl3 = 1;
							VonCroy->reached_goal = 0;
							VonCroy->enemy = 0;
							item->item_flags[3] += ifl3;
							item->ai_bits = FOLLOW;*/
							break;
						case 0x06: //Codebit: 1 + 2					??
							/*if (lara.location > item->item_flags[3])
							{
								ifl3 = 1;
								VonCroy->reached_goal = 0;
								VonCroy->enemy = 0;
								item->item_flags[3] += ifl3;
								item->ai_bits = FOLLOW;
							}
							else if (item->meshswap_meshbits & 0x40080)
								item->goal_anim_state = 6;
							else
								item->goal_anim_state = 31;*/
							break;
						case 0x08: //Codebit: 3						Use Switch Animation
							/*if (lara.location > item->item_flags[3])
							{
								ifl3 = 1;
								VonCroy->reached_goal = 0;
								VonCroy->enemy = 0;
								item->item_flags[3] += ifl3;
								item->ai_bits = FOLLOW;
							}

							item->goal_anim_state = 20;*/
							break;
						case 0x20: //Codebit: 5						Force to kill nearby enemies
							/*ifl3 = -1;

							if (oEnemy)
							{
								room_number = oEnemy->room_number;
								floor = GetFloor(oEnemy->pos.x_pos, oEnemy->pos.y_pos, oEnemy->pos.z_pos, &room_number);
								GetHeight(floor, oEnemy->pos.x_pos, oEnemy->pos.y_pos, oEnemy->pos.z_pos);
								TestTriggers(trigger_index, 1, 0);
								ifl3 = 1;
							}

							VonCroy->reached_goal = 0;
							VonCroy->enemy = 0;
							item->item_flags[3] += ifl3;
							item->ai_bits = FOLLOW;*/
							break;
						case 0x0A: //Codebit: 1 + 3					Check pit before jump down
							/*if (lara.location > item->item_flags[3])
							{
								ifl3 = 1;
								VonCroy->reached_goal = 0;
								VonCroy->enemy = 0;
								item->item_flags[3] += ifl3;
								item->ai_bits = FOLLOW;
							}

							item->goal_anim_state = 7;*/
							break;
						case 0x22: //Codebit: 1 + 5					??
							/*if (lara.location > item->item_flags[3])
							{
								ifl3 = 2;
								VonCroy->reached_goal = 0;
								VonCroy->enemy = 0;
								item->item_flags[3] += ifl3;
								item->ai_bits = FOLLOW;
							}

							item->goal_anim_state = 32;*/
							break;
						case 0x24: //Codebit: 2 + 5					Wave Lara when she gets closer than 3 sectors.
							/*if (lara.location > item->item_flags[3])
							{
								ifl3 = 1;
								VonCroy->reached_goal = 0;
								VonCroy->enemy = 0;
								item->item_flags[3] += ifl3;
								item->ai_bits = FOLLOW;
							}

							item->goal_anim_state = 11;*/
							break;
						case 0x28: //Codebit: 3 + 5					Jump 3 sectors
							/*if (item->item_flags[2] == 6)
								item->goal_anim_state = 3;
							else
							{
								item->goal_anim_state = 34;
								item->pos.x_pos = oEnemy->pos.x_pos;
								item->pos.y_pos = oEnemy->pos.y_pos;
								item->pos.z_pos = oEnemy->pos.z_pos;
								item->pos.x_rot = oEnemy->pos.x_rot;
								item->pos.y_rot = oEnemy->pos.y_rot;
								item->pos.z_rot = oEnemy->pos.z_rot;
							}*/
							break;
						case 0x3E: //Codebit: 1 + 2 + 3 + 4 + 5		Dissapear
							break;
					}
				}
				else
				{
					item->Animation.RequiredState = VON_CROY_STATE_ADJUST_POSITION_BACK - (AI.ahead != 0);
					creature->MaxTurn = 0;
				}
			}
			else if (currentJump > VonCroyJumps::NO_JUMP)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_IDLE_TO_JUMP;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				switch (currentJump)
				{
					case VonCroyJumps::JUMP_1_SECTOR:
						item->Animation.TargetState = VON_CROY_STATE_JUMP_1_BLOCK;
						break;
					case VonCroyJumps::JUMP_2_SECTORS:
						item->Animation.TargetState = VON_CROY_STATE_JUMP_2_BLOCKS;
						break;
					case VonCroyJumps::JUMP_3_SECTORS:
						item->Animation.TargetState = VON_CROY_STATE_JUMP_2_BLOCKS;
					break;
				}
				creature->MaxTurn = 0;
				creature->LOT.IsJumping = true;
			}
			else if (canMonkeySwing)
			{
				probe = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
				if (probe.Position.Ceiling == (probe.Position.Floor - VON_CROY_MONKEYSWING_JUMP_HEIGHT))
				{
					if (item->TestBits(JointBitType::MeshSwap, VonCroyKnifeSwapJoints))
						item->Animation.TargetState = VON_CROY_STATE_TOGGLE_KNIFE;
					else
						item->Animation.TargetState = VON_CROY_STATE_START_MONKEY;
					break;
				}
			}
			else if (!creature->Enemy->IsLara() || AI.distance > VON_CROY_ATTACK_RANGE)
			{
				if (flagRunDefault && AI.distance > pow(SECTOR(3), 2))
					item->Animation.TargetState = VON_CROY_STATE_RUN;
				else
					item->Animation.TargetState = VON_CROY_STATE_WALK;
			}
			break;

		case VON_CROY_STATE_WALK:
			creature->MaxTurn = ANGLE(7.0f);
			creature->LOT.IsJumping = false;
			creature->LOT.IsMonkeying = false;
			creature->Flags = 0;

			if (laraAI.ahead)
				joint3 = laraAI.angle;
			else if (AI.ahead)
				joint3 = AI.angle;

			if (goalNode < item->ItemFlags[3])
			{
				item->Animation.TargetState = VON_CROY_STATE_IDLE;
			}
			else if (currentJump > VonCroyJumps::NO_JUMP)
			{
				creature->MaxTurn = 0;
			}
			else if (canMonkeySwing)
			{
				item->Animation.TargetState = VON_CROY_STATE_IDLE;
			}
			else if (!creature->ReachedGoal)
			{
				if (enemy->Flags == 0x20) //Bit 5, force kill enemies before continue path.
				{
					/*ifl3 = -1;

					if (oEnemy)
					{
						room_number = oEnemy->room_number;
						floor = GetFloor(oEnemy->pos.x_pos, oEnemy->pos.y_pos, oEnemy->pos.z_pos, &room_number);
						GetHeight(floor, oEnemy->pos.x_pos, oEnemy->pos.y_pos, oEnemy->pos.z_pos);
						TestTriggers(trigger_index, 1, 0);
						ifl3 = 1;
					}

					VonCroy->reached_goal = 0;
					VonCroy->enemy = 0;
					item->item_flags[3] += ifl3;
					item->ai_bits = FOLLOW;*/
				}else{
					if (AI.distance < VON_CROY_ATTACK_RANGE)
					{
						creature->ReachedGoal = true;
						item->Animation.TargetState = VON_CROY_STATE_IDLE;
					}
					else if (AI.distance > 0x900000)
						item->Animation.TargetState = VON_CROY_STATE_RUN;
				}					
			}
			else
				item->Animation.TargetState = VON_CROY_STATE_IDLE;
			break;

		case VON_CROY_STATE_RUN:
			if (AI.ahead)
				joint3 = AI.angle;

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				creature->LOT.IsJumping = false;
				creature->MaxTurn = 1456;
			}

			angle >>= 1;

			if (currentJump > VonCroyJumps::NO_JUMP)
			{
				creature->MaxTurn = 0;
				item->Animation.TargetState = VON_CROY_STATE_JUMP_2_BLOCKS;
			}
			else if (goalNode < item->ItemFlags[3] || creature->JumpAhead)
				item->Animation.TargetState = VON_CROY_STATE_IDLE;
			else if (canMonkeySwing)
				item->Animation.TargetState = VON_CROY_STATE_IDLE;
			else if (creature->ReachedGoal)
			{
				if (enemy->Flags == 0x20)  //bit 5
				{
					/*ifl3 = -1;

					if (oEnemy)
					{
						room_number = oEnemy->room_number;
						floor = GetFloor(oEnemy->pos.x_pos, oEnemy->pos.y_pos, oEnemy->pos.z_pos, &room_number);
						GetHeight(floor, oEnemy->pos.x_pos, oEnemy->pos.y_pos, oEnemy->pos.z_pos);
						TestTriggers(trigger_index, 1, 0);
						ifl3 = 1;
					}

					VonCroy->reached_goal = 0;
					VonCroy->enemy = 0;
					item->item_flags[3] += ifl3;
					item->ai_bits = FOLLOW;*/
				}
				else if (AI.distance < 0x4000)
				{
					if (enemy->Flags == 0x28) //bit 3+5
					{
						creature->MaxTurn = 0;
						item->Pose.Orientation.y = enemy->Pose.Orientation.y;
						item->Animation.TargetState = VON_CROY_STATE_JUMP_2_BLOCKS;
						currentJump = VonCroyJumps::JUMP_2_SECTORS;
					}
				}
				else
					item->Animation.TargetState = VON_CROY_STATE_IDLE;
			}
			else if (AI.distance < 0x64000)
			{
				if (enemy->Flags != 0x20 && enemy->Flags != 0x28)
					item->Animation.TargetState = VON_CROY_STATE_IDLE;
			}

			break;

		case VON_CROY_STATE_START_MONKEY:
			creature->MaxTurn = 0;

			if (item->BoxNumber == creature->LOT.TargetBox || !creature->MonkeySwingAhead)
			{
				probe = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, probe.RoomNumber);
				if (probe.Position.Ceiling == probe.Position.Floor - VON_CROY_MONKEYSWING_JUMP_HEIGHT)
					item->Animation.TargetState = VON_CROY_STATE_IDLE;
				}
				else
					item->Animation.TargetState = VON_CROY_STATE_MONKEY;

			break;

		case VON_CROY_STATE_MONKEY:
			creature->LOT.IsJumping = true;
			creature->LOT.IsMonkeying = true;
			creature->MaxTurn = ANGLE(6.0f);
						
			if (item->BoxNumber == creature->LOT.TargetBox || !creature->MonkeySwingAhead)
			{
				probe = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, probe.RoomNumber);
				if (probe.Position.Ceiling == (probe.Position.Floor - SECTOR(1.5f)))
					item->Animation.TargetState = VON_CROY_STATE_START_MONKEY;
			}
			break;

		case VON_CROY_STATE_TOGGLE_KNIFE:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 28)
			{
				if (!item->TestBits(JointBitType::MeshSwap, VonCroyKnifeSwapJoints))
					item->SetBits(JointBitType::MeshSwap, VonCroyKnifeSwapJoints);
				else
					item->ClearBits(JointBitType::MeshSwap, VonCroyKnifeSwapJoints);
			}
			break;

		case VON_CROY_STATE_LOOK_BEFORE_JUMP:
			/*if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				item->Pose = enemy->Pose;

				if (item->ItemFlags[2] == VON_CROY_FLAG_JUMP)
				{
					creature->MaxTurn = 0;
					item->Animation.ActiveState = VON_CROY_STATE_JUMP_1_BLOCK;
					item->Animation.TargetState = VON_CROY_STATE_JUMP_2_BLOCKS;
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_IDLE_TO_JUMP;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					creature->LOT.IsJumping = true;
				}

				item->AIBits = FOLLOW;
				creature->ReachedGoal = false;
				creature->Enemy = nullptr;
				creature->LocationAI++;
			}*/
			break;

		case VON_CROY_STATE_JUMP_1_BLOCK:
			if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_JUMP_1_BLOCK)
				item->Animation.TargetState = VON_CROY_STATE_RUN;
			break;

		case VON_CROY_STATE_JUMP_2_BLOCKS:
			if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_JUMP_2_BLOCKS
				|| item->Animation.FrameNumber > (g_Level.Anims[item->Animation.AnimNumber].frameBase + 7))
				creature->LOT.IsJumping = true;
			else if (creature->JumpAhead)
				item->Animation.TargetState = VON_CROY_STATE_JUMP_1_BLOCK;

			if (currentJump > VonCroyJumps::NO_JUMP)
				item->Animation.TargetState = VON_CROY_STATE_STANDING_JUMP_GRAB;
			break;

		case VON_CROY_STATE_ENABLE_TRAP:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				item->Pose = enemy->Pose;
			else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 120)
			{
				TestTriggers(
					creature->AITarget->Pose.Position.x,
					creature->AITarget->Pose.Position.y,
					creature->AITarget->Pose.Position.z,
					creature->AITarget->RoomNumber,
					true);

				creature->ReachedGoal = false;
				creature->Enemy = nullptr;
				item->AIBits = FOLLOW;
			}

			break;

		case VON_CROY_STATE_KNIFE_ATTACK_HIGH:
			/*creature->MaxTurn = 0;
			ClampRotation(&item->Pose, AI.angle, ANGLE(6.0f));

			if (AI.ahead)
			{
				joint2 = AI.angle / 2;
				joint1 = AI.xAngle / 2;
				joint0 = joint2;
			}

			if (!creature->Flags && enemy != nullptr)
			{
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 20 &&
					item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 45)
				{
					if (abs(item->Pose.Position.x - enemy->Pose.Position.x) < CLICK(2) &&
						abs(item->Pose.Position.y - enemy->Pose.Position.y) < CLICK(2) &&
						abs(item->Pose.Position.z - enemy->Pose.Position.z) < CLICK(2))
					{
						DoDamage(enemy, 40);
						CreatureEffect2(item, VonCroyBite, 2, -1, DoBloodSplat);
						creature->Flags = 1;

						if (enemy->HitPoints <= 0)
							item->AIBits = FOLLOW;
					}
				}
			}*/
			break;

		case VON_CROY_STATE_LOOK_BACK_LEFT:
		case VON_CROY_STATE_LOOK_BACK_RIGHT:
			creature->MaxTurn = 0;

			if (flagLookLara)
				ClampRotation(&item->Pose, laraAI.angle, ANGLE(2.8f));
			else
				ClampRotation(&item->Pose, enemy->Pose.Orientation.y - item->Pose.Orientation.y, ANGLE(2.8f));

			break;

		case VON_CROY_STATE_GRAB_BORDER:
			/*creature->MaxTurn = 0;
			creature->LOT.IsJumping = true;
						
			item->Animation.TargetState = VON_CROY_STATE_LADDER_CLIMB_UP;
			creature->ReachedGoal = false;
			creature->Enemy = nullptr;
			item->AIBits = FOLLOW;
			creature->LocationAI++;*/

			break;

		case VON_CROY_STATE_SHIMMY_BORDER_RIGHT:
		case VON_CROY_STATE_SHIMMY_BORDER_LEFT:
			/*creature->MaxTurn = 0;
			creature->LOT.IsJumping = true;*/
			break;

		case VON_CROY_STATE_KNIFE_ATTACK_LOW:
			if (AI.ahead)
			{
				joint2 = AI.angle / 2;
				joint1 = AI.xAngle / 2;
				joint0 = joint2;
			}

			creature->MaxTurn = 0;
			if (abs(AI.angle) >= ANGLE(6.0f))
			{
				if (AI.angle < 0)
					item->Pose.Orientation.y += ANGLE(6.0f);
				else
					item->Pose.Orientation.y -= ANGLE(6.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (!creature->Flags)
			{
				if (enemy)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 15 &&
						item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 26)
					{
						if (creature->Enemy && !creature->Enemy->IsLara() && AI.distance < VON_CROY_ATTACK_RANGE)
						{
							DoDamage(enemy, VON_CROY_ATTACK_DAMAGE);
							CreatureEffect2(item, VonCroyBite, 2, -1, DoBloodSplat);
							creature->Flags = 1;
						}
						if (enemy->HitPoints <= 0)
							item->AIBits = FOLLOW;
					}
				}
			}
			break;

		case VON_CROY_STATE_POINT:
			creature->MaxTurn = 0;
			ClampRotation(&item->Pose, AI.angle / 2, ANGLE(6.0f));

			if (AI.ahead)
			{
				joint0 = joint2;
				joint1 = AI.xAngle;
				joint2 = AI.angle / 2;
			}

			if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_START_POINT)
			{
				if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase)
					break;
			}
			else
			{
				item->Animation.TargetState = VON_CROY_STATE_IDLE;

				if (TestProbability(0.97f))
					break;
			}

			item->AIBits = FOLLOW;
			creature->ReachedGoal = false;
			creature->Enemy = nullptr;
			creature->LocationAI++;
			break;

		case VON_CROY_STATE_STANDING_JUMP_GRAB:
			if (item->Animation.AnimNumber != Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_UP_AFTER_JUMP
				|| item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
			}

			item->Animation.TargetState = VON_CROY_STATE_WALK;
			break;

		case VON_CROY_STATE_JUMP_BACK:
			currentJump = VonCroyJumps::JUMP_1_SECTOR;
			break;

		case VON_CROY_STATE_ADJUST_POSITION_FRONT:
		case VON_CROY_STATE_ADJUST_POSITION_BACK:
			creature->MaxTurn = 0;
			MoveCreature3DPos(&item->Pose, &enemy->Pose, 15, enemy->Pose.Orientation.y - item->Pose.Orientation.y, 512);
			break;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureJoint(item, 3, joint3);

		if ((item->Animation.ActiveState < VON_CROY_STATE_JUMP_1_BLOCK)
			&& (item->Animation.ActiveState != VON_CROY_STATE_MONKEY))
		{
			switch (CreatureVault(itemNumber, angle, 2, 260))
			{
			case VON_CROY_STATE_WALK:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_2_BLOCKS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_CLIMB_2_BLOCKS;
				creature->MaxTurn = 0;
				break;

			case VON_CROY_STATE_RUN:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_3_BLOCKS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_CLIMB_3_BLOCKS;
				creature->MaxTurn = 0;
				break;

			case VON_CROY_STATE_START_MONKEY:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_4_BLOCKS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_CLIMB_4_BLOCKS;
				creature->MaxTurn = 0;
				break;

			case VON_CROY_STATE_LOOK_BEFORE_JUMP:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_JUMP_TO_HANG;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_GRAB_BORDER;
				creature->MaxTurn = 0;
				break;

			case -7:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_DOWN_2_SECTORS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_STEP_DOWN_HIGH;
				creature->MaxTurn = 0;
				break;

			case -4:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_DOWN_1_SECTOR;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_JUMP_DOWN_4_CLICKS;
				creature->MaxTurn = 0;
				break;

			case -3:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_OFF_3_CLICKS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_JUMP_DOWN_3_CLICKS;
				creature->MaxTurn = 0;
				break;

			case -2:
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + VON_CROY_ANIM_CLIMB_OFF_2_CLICKS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = VON_CROY_STATE_JUMP_DOWN_2_CLICKS;
				creature->MaxTurn = 0;
				break;
			}
		}
		else
			CreatureAnimation(itemNumber, angle, 0);
	}
}
