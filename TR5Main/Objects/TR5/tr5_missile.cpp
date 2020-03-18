#include "../oldobjects.h"
#include "../../Game/items.h"
#include "../../Game/Box.h"
#include "../../Game/people.h"
#include "../../Game/sphere.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effect2.h"
#include "../../Game/draw.h"
#include "../../Game/effects.h"
#include "../../Game/traps.h"

void MissileControl(short itemNumber)
{
	FX_INFO* fx = &Effects[itemNumber];
	if (fx->flag1 == 2)
	{
		fx->pos.zRot += 16 * fx->speed;

		if (fx->speed > 64)
			fx->speed -= 4;

		if (fx->pos.xRot > -12288)
		{
			if (fx->fallspeed < 512)
				fx->fallspeed += 36;
			fx->pos.xRot -= fx->fallspeed;
		}
	}
	else
	{
		short angles[2];
		phd_GetVectorAngles(
			LaraItem->pos.xPos - fx->pos.xPos,
			LaraItem->pos.yPos - fx->pos.yPos - 256,
			LaraItem->pos.zPos - fx->pos.zPos,
			angles);

		int dh;
		if (fx->flag1)
		{
			dh = fx->flag1 != 1 ? 768 : 384;
		}
		else
		{
			if (fx->counter)
				fx->counter--;
			dh = 256;
		}

		if (fx->speed < 192)
		{
			if (fx->flag1 == 0 || fx->flag1 == 1)
				fx->speed++;

			int dy = abs(angles[0] - fx->pos.yRot) >> 3;
			int dx = abs(angles[1] - fx->pos.xRot) >> 3;

			if (dy <= dh)
			{
				if (dy < -dh)
					dy = -dh;
			}
			else
			{
				dy = dh;
			}
			
			if (dx <= dh)
			{
				if (dx < -dh)
					dx = -dh;
			}
			else
			{
				dx = dh;
			}

			fx->pos.yRot += dy;
			fx->pos.xRot += dx;
		}
		
		fx->pos.zRot += 16 * fx->speed;

		if (!fx->flag1)
			fx->pos.zRot += 16 * fx->speed;
	}

	int x = fx->pos.xPos;
	int y = fx->pos.yPos;
	int z = fx->pos.zPos;

	int c = fx->speed * COS(fx->pos.xRot) >> W2V_SHIFT;

	fx->pos.xPos += c * SIN(fx->pos.yRot) >> W2V_SHIFT;
	fx->pos.yPos += fx->speed * SIN(-fx->pos.xRot) >> W2V_SHIFT;
	fx->pos.zPos += c * COS(fx->pos.yRot) >> W2V_SHIFT;

	short roomNumber = fx->roomNumber;
	FLOOR_INFO* floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);
	int fh = GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
	int ch = GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
	
	if (fx->pos.yPos >= fh || fx->pos.yPos <= ch)
	{
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;

		if (fx->flag1)
		{
			if (fx->flag1 == 1)
			{
				TriggerExplosionSparks(x, y, z, 3, -2, 2, fx->roomNumber);
				fx->pos.yPos -= 64;
				//TriggerShockwave((PHD_VECTOR*)fx, 16777264, 64, 402686016, 0x10000);
				fx->pos.yPos -= 128;
				//TriggerShockwave((PHD_VECTOR*)fx, 15728688, 48, 402686016, 0x10000);
			}
			else if (fx->flag1 == 2)
			{
				//ExplodeFX(fx, 0, 32);
				SoundEffect(251, &fx->pos, 0);
			}
		}
		else
		{
			TriggerExplosionSparks(x, y, z, 3, -2, 0, fx->roomNumber);
			//TriggerShockwave(fx, 15728688, 48, 411066368, 0x20000);
		}
		
		KillEffect(itemNumber);
	}
	else if (ItemNearLara((PHD_3DPOS*)fx, 200))
	{
		LaraItem->hitStatus = true;
		
		if (fx->flag1)
		{
			if (fx->flag1 == 1)
			{
				TriggerExplosionSparks(x, y, z, 3, -2, 2, fx->roomNumber);
				fx->pos.yPos -= 64;
				//TriggerShockwave(fx, 16777264, 64, 402686016, 0x10000);
				fx->pos.yPos -= 128;
				//TriggerShockwave(fx, 15728688, 48, 402686016, 0x10000);
				LaraItem->hitPoints -= 200;			
				KillEffect(itemNumber);
			}
			else
			{
				if (fx->flag1 == 2)
				{
					//ExplodeFX(fx, 0, 32);
					LaraItem->hitPoints -= 50;
					DoBloodSplat(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, (GetRandomControl() & 3) + 2, LaraItem->pos.yRot, LaraItem->roomNumber);
					SoundEffect(251, &fx->pos, 0);
					SoundEffect(31, &LaraItem->pos, 0);
				}
				
				KillEffect(itemNumber);
			}
		}
		else
		{
			TriggerExplosionSparks(x, y, z, 3, -2, 0, fx->roomNumber);
			//TriggerShockwave(fx, 15728688, 48, 411066368, 0);
			if (LaraItem->hitPoints >= 500)
				LaraItem->hitPoints -= 300;
			else
				LaraBurn();
			KillEffect(itemNumber);
		}
	}
	else
	{
		if (roomNumber != fx->roomNumber)
			EffectNewRoom(itemNumber, roomNumber);

		if (GlobalCounter & 1)
		{
			PHD_VECTOR pos;
			pos.x = x;
			pos.y = y;
			pos.z = z;

			int xv = x - fx->pos.xPos;
			int yv = y - fx->pos.yPos;
			int zv = z - fx->pos.zPos;

			if (fx->flag1)
			{
				if (fx->flag1 == 1)
					TriggerRomanStatueMissileSparks(&pos, itemNumber);
			}
			else
			{
				TriggerHydraMissileSparks(&pos, 4 * xv, 4 * yv, 4 * zv);
				TriggerHydraMissileSparks((PHD_VECTOR*)&fx, 4 * xv, 4 * yv, 4 * zv);
			}
		}
	}
}

