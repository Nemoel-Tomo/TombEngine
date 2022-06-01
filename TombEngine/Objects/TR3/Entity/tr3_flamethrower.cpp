#include "framework.h"
#include "Objects/TR3/Entity/tr3_flamethrower.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO FlamethrowerBite = { 0, 340, 64, 7 };

// TODO
enum FlamethrowerState
{

};

// TODO
enum FlamethrowerAnim
{

};

static void TriggerPilotFlame(int itemNumber)
{
	int dx = LaraItem->Pose.Position.x - g_Level.Items[itemNumber].Pose.Position.x;
	int dz = LaraItem->Pose.Position.z - g_Level.Items[itemNumber].Pose.Position.z;

	if (dx < -SECTOR(16) || dx > SECTOR(16) || dz < -SECTOR(16) || dz > SECTOR(16))
		return;

	auto* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 48 + (GetRandomControl() & 31);
	spark->sG = spark->sR;
	spark->sB = 192 + (GetRandomControl() & 63);

	spark->dR = 192 + (GetRandomControl() & 63);
	spark->dG = 128 + (GetRandomControl() & 63);
	spark->dB = 32;

	spark->colFadeSpeed = 12 + (GetRandomControl() & 3);
	spark->fadeToBlack = 4;
	spark->sLife = spark->life = (GetRandomControl() & 3) + 20;
	spark->transType = TransTypeEnum::COLADD;
	spark->extras = 0;
	spark->dynamic = -1;
	spark->x = (GetRandomControl() & 31) - 16;
	spark->y = (GetRandomControl() & 31) - 16;
	spark->z = (GetRandomControl() & 31) - 16;

	spark->xVel = (GetRandomControl() & 31) - 16;
	spark->yVel = -(GetRandomControl() & 3);
	spark->zVel = (GetRandomControl() & 31) - 16;

	spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
	spark->fxObj = itemNumber;
	spark->nodeNumber = 0;
	spark->friction = 4;
	spark->gravity = -(GetRandomControl() & 3) - 2;
	spark->maxYvel = -(GetRandomControl() & 3) - 4;
	//spark->def = Objects[EXPLOSION1].mesh_index;
	spark->scalar = 0;
	int size = (GetRandomControl() & 7) + 32;
	spark->size = size / 2;
	spark->dSize = size;
}

static void TriggerFlamethrowerFlame(int x, int y, int z, int xv, int yv, int zv, int fxNumber)
{
	auto* spark = &Sparks[GetFreeSpark()];

	spark->on = true;
	spark->sR = 48 + (GetRandomControl() & 31);
	spark->sG = spark->sR;
	spark->sB = 192 + (GetRandomControl() & 63);

	spark->dR = 192 + (GetRandomControl() & 63);
	spark->dG = 128 + (GetRandomControl() & 63);
	spark->dB = 32;

	if (xv || yv || zv)
	{
		spark->colFadeSpeed = 6;
		spark->fadeToBlack = 2;
		spark->sLife = spark->life = (GetRandomControl() & 1) + 12;
	}
	else
	{
		spark->colFadeSpeed = 8;
		spark->fadeToBlack = 16;
		spark->sLife = spark->life = (GetRandomControl() & 3) + 20;
	}

	spark->transType = TransTypeEnum::COLADD;

	spark->extras = 0;
	spark->dynamic = -1;

	spark->x = x + ((GetRandomControl() & 31) - 16);
	spark->y = y;
	spark->z = z + ((GetRandomControl() & 31) - 16);

	spark->xVel = ((GetRandomControl() & 15) - 16) + xv;
	spark->yVel = yv;
	spark->zVel = ((GetRandomControl() & 15) - 16) + zv;
	spark->friction = 0;

	if (GetRandomControl() & 1)
	{
		if (fxNumber >= 0)
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
		else
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

		spark->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			spark->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			spark->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
	{
		if (fxNumber >= 0)
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
		else
			spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;
	}

	spark->fxObj = fxNumber;
	spark->gravity = 0;
	spark->maxYvel = 0;

	int size = (GetRandomControl() & 31) + 64;

	if (xv || yv || zv)
	{
		spark->size = size / 32;
		if (fxNumber == -2)
			spark->scalar = 2;
		else
			spark->scalar = 3;
	}
	else
	{
		spark->size = size / 16;
		spark->scalar = 4;
	}

	spark->dSize = size / 2;
}

static short TriggerFlameThrower(ItemInfo* item, BITE_INFO* bite, short speed)
{
	short effectNumber = CreateNewEffect(item->RoomNumber);
	if (effectNumber != NO_ITEM)
	{
		auto* fx = &EffectList[effectNumber];

		auto pos1 = Vector3Int(bite->x, bite->y, bite->z);
		GetJointAbsPosition(item, &pos1, bite->meshNum);

		auto pos2 = Vector3Int(bite->x, bite->y / 2, bite->z);
		GetJointAbsPosition(item, &pos2, bite->meshNum);

		auto angles = GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z);

		fx->pos.Position = pos1;
		fx->pos.Orientation = angles;
		fx->RoomNumber = item->RoomNumber;
		fx->speed = speed * 4;
		fx->counter = 20;
		fx->flag1 = 0;

		TriggerFlamethrowerFlame(0, 0, 0, 0, 0, 0, effectNumber);

		int velocity;
		int xv, yv, zv;
		
		for (int i = 0; i < 2; i++)
		{
			speed = (GetRandomControl() % (speed * 4)) + 32;
			velocity = speed * phd_cos(fx->pos.Orientation.x);

			xv = velocity * phd_sin(fx->pos.Orientation.y);
			yv = -speed * phd_sin(fx->pos.Orientation.x);
			zv = velocity * phd_cos(fx->pos.Orientation.y);

			TriggerFlamethrowerFlame(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, xv * 32, yv * 32, zv * 32, -1);
		}

		velocity = (speed * 2) * phd_cos(fx->pos.Orientation.x);
		zv = velocity * phd_cos(fx->pos.Orientation.y);
		xv = velocity * phd_sin(fx->pos.Orientation.y);
		yv = -(speed * 2) * phd_sin(fx->pos.Orientation.x);

		TriggerFlamethrowerFlame(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, xv * 32, yv * 32, zv * 32, -2);
	}

	return effectNumber;
}

void FlameThrowerControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short torsoX = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;
	short head = 0;

	Vector3Int pos = { FlamethrowerBite.x, FlamethrowerBite.y, FlamethrowerBite.z };
	GetJointAbsPosition(item, &pos, FlamethrowerBite.meshNum);

	int random = GetRandomControl();
	if (item->Animation.ActiveState != 6 && item->Animation.ActiveState != 11)
	{
		TriggerDynamicLight(pos.x, pos.y, pos.z, (random & 3) + 6, 24 - ((random / 16) & 3), 16 - ((random / 64) & 3), random & 3); 
		TriggerPilotFlame(itemNumber);
	}
	else
		TriggerDynamicLight(pos.x, pos.y, pos.z, (random & 3) + 10, 31 - ((random / 16) & 3), 24 - ((random / 64) & 3), random & 7);  

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 7)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 7;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;
		else
		{
			creature->Enemy = NULL;

			ItemInfo* target = NULL;
			int minDistance = INT_MAX;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreature = ActiveCreatures[i];
				if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
					continue;

				target = &g_Level.Items[currentCreature->ItemNumber];
				if (target->ObjectNumber == ID_LARA || target->HitPoints <= 0)
					continue;

				int x = target->Pose.Position.x - item->Pose.Position.x;
				int z = target->Pose.Position.z - item->Pose.Position.z;

				int distance = pow(x, 2) + pow(z, 2);
				if (distance < minDistance)
				{
					creature->Enemy = target;
					minDistance = distance;
				}
			}
		}

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		AI_INFO laraAI;
		if (creature->Enemy == LaraItem)
		{
			laraAI.angle = AI.angle;
			laraAI.distance = AI.distance;

			if (!creature->HurtByLara)
				creature->Enemy = NULL;
		}
		else
		{
			int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			
			laraAI.angle = phd_atan(dz, dz) - item->Pose.Orientation.y; 
			laraAI.distance = pow(dx, 2) + pow(dz, 2);
			
			AI.xAngle -= 0x800;
		}

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		auto* realEnemy = creature->Enemy; 

		if (item->HitStatus || laraAI.distance < pow(SECTOR(1), 2) || TargetVisible(item, &laraAI))
		{
			if (!creature->Alerted)
				SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);

			AlertAllGuards(itemNumber);
		}
		
		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->MaxTurn = 0;
			creature->Flags = 0;
			head = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);

				if (!(GetRandomControl() & 0xFF))
					item->Animation.TargetState = 4;

				break;
			}
			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 2;
			else if (Targetable(item, &AI) && (realEnemy != LaraItem || creature->HurtByLara))
			{
				if (AI.distance < pow(SECTOR(4), 2))
					item->Animation.TargetState = 10;
				else
					item->Animation.TargetState = 2;
			}
			else if (creature->Mood == MoodType::Bored && AI.ahead && !(GetRandomControl() & 0xFF))
				item->Animation.TargetState = 4;
			else if (creature->Mood == MoodType::Attack || !(GetRandomControl() & 0xFF))
				item->Animation.TargetState = 2;
			
			break;

		case 4:
			head = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);

				if (!(GetRandomControl() & 0xFF))
					item->Animation.TargetState = 1;

				break;
			}
			else if ((Targetable(item, &AI) &&
				AI.distance < pow(SECTOR(4), 2) &&
				(realEnemy != LaraItem || creature->HurtByLara) ||
				creature->Mood != MoodType::Bored ||
				!(GetRandomControl() & 0xFF)))
			{
				item->Animation.TargetState = 1;
			}

			break;

		case 2:
			creature->Flags = 0;
			creature->MaxTurn = ANGLE(5.0f);
			head = laraAI.angle;

			if (item->AIBits & GUARD)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 1;
				item->Animation.TargetState = 1;
			}
			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 2;
			else if (Targetable(item, &AI) && 
				(realEnemy != LaraItem || creature->HurtByLara))
			{
				if (AI.distance < pow(SECTOR(4), 2))
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 9;
			}
			else if (creature->Mood == MoodType::Bored && AI.ahead)
				item->Animation.TargetState = 1;
			else
				item->Animation.TargetState = 2;

			break;

		case 10:
			creature->Flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->HurtByLara))
				{
					item->Animation.TargetState = 11;
				}
				else
					item->Animation.TargetState = 1;
			}

			break;

		case 9:
			creature->Flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->HurtByLara))
				{
					item->Animation.TargetState = 6;
				}
				else
					item->Animation.TargetState = 2;
			}

			break;

		case 11:
			if (creature->Flags < 40)
				creature->Flags += (creature->Flags / 4) + 1;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->HurtByLara))
				{
					item->Animation.TargetState = 11;
				}
				else
					item->Animation.TargetState = 1;
			}
			else
				item->Animation.TargetState = 1;

			if (creature->Flags < 40)
				TriggerFlameThrower(item, &FlamethrowerBite, creature->Flags);
			else
			{
				TriggerFlameThrower(item, &FlamethrowerBite, (GetRandomControl() & 31) + 12);
				if (realEnemy)
				{
					/*code*/
				}
			}

			SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
			break;
			
		case 6:
			if (creature->Flags < 40)
				creature->Flags += (creature->Flags / 4) + 1;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (Targetable(item, &AI) &&
					AI.distance < pow(SECTOR(4), 2) &&
					(realEnemy != LaraItem || creature->HurtByLara))
				{
					item->Animation.TargetState = 6;
				}
				else
					item->Animation.TargetState = 2;
			}
			else
				item->Animation.TargetState = 2;

			if (creature->Flags < 40)
				TriggerFlameThrower(item, &FlamethrowerBite, creature->Flags);
			else
			{
				TriggerFlameThrower(item, &FlamethrowerBite, (GetRandomControl() & 31) + 12);
				if (realEnemy)
				{
					/*code*/
				}
			}

			SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	CreatureAnimation(itemNumber, angle, 0);
}
