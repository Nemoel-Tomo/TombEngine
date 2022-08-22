#include "framework.h"
#include "Objects/TR2/Vehicles/skidoo.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/effects/simple_particle.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Objects/Utils/Vehicle.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Sound/sound.h"

using namespace TEN::Input;
using std::vector;

namespace TEN::Entities::Vehicles
{
	//------------------

	SkidooObject::SkidooObject()
	{
		this->Attributes = VehicleAttributes
		{
			SKIDOO_RADIUS,
			SKIDOO_FRONT * 2,
			SKIDOO_SIDE * 2,
			SKIDOO_HEIGHT,
			SKIDOO_STEP_HEIGHT,
			SKIDOO_BOUNCE_MIN,
			SKIDOO_KICK_MAX,
			SKIDOO_MOUNT_DISTANCE,
			SKIDOO_DISMOUNT_DISTANCE
		};
	}

	void SkidooObject::InitializeSkidoo(short itemNumber)
	{
		auto* skidooItem = &g_Level.Items[itemNumber];
		//skidooItem->Data = Skidoo();
		auto* skidoo = GetSkidooInfo(skidooItem);

		if (skidooItem->Status != ITEM_ACTIVE)
		{
			AddActiveItem(itemNumber);
			skidooItem->Status = ITEM_ACTIVE;
		}
		
		if (skidooItem->ObjectNumber == ID_SNOWMOBILE_GUN)
			skidoo->Specific.Armed = true;

		skidoo->Control.MomentumAngle = skidooItem->Pose.Orientation.y;
	}

	SkidooObject SkidooGlobal;

	//------------------

	SkidooObject* GetSkidooInfo(ItemInfo* skidooItem)
	{
		return &SkidooGlobal;
	}

	void InitialiseSkidoo(short itemNumber)
	{
		auto* skidooItem = &g_Level.Items[itemNumber];
		//skidooItem->Data = Skidoo();
		auto* skidoo = GetSkidooInfo(skidooItem);

		if (skidooItem->Status != ITEM_ACTIVE)
		{
			AddActiveItem(itemNumber);
			skidooItem->Status = ITEM_ACTIVE;
		}

		if (skidooItem->ObjectNumber == ID_SNOWMOBILE_GUN)
			skidoo->Specific.Armed = true;

		skidoo->Control.MomentumAngle = skidooItem->Pose.Orientation.y;
	}

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* skidooItem = &g_Level.Items[lara->Vehicle];
		auto* skidoo = GetSkidooInfo(skidooItem);

		Vector3Int frontLeft, frontRight;
		auto collide = SkidooDynamics(skidooItem, laraItem);
		auto heightFrontLeft = GetVehicleHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, true, &frontLeft);
		auto heightFrontRight = GetVehicleHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, true, &frontRight);

		auto probe = GetCollision(skidooItem);

		TestTriggers(skidooItem, true);
		TestTriggers(skidooItem, false);

		bool isDead = false;
		int drive = 0;

		if (laraItem->HitPoints <= 0)
		{
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
			isDead = true;
		}
		else if (laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_FALL)
		{
			isDead = true;
			collide = VehicleImpactDirection::None;
		}

		int height = probe.Position.Floor;
		int pitch = 0;

		if (skidooItem->Flags & IFLAG_INVISIBLE)
		{
			drive = 0;
			collide = VehicleImpactDirection::None;
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case SKIDOO_STATE_MOUNT_LEFT:  // Unused?
			case SKIDOO_STATE_MOUNT_RIGHT: // Used for both?
			case SKIDOO_STATE_DISMOUNT_LEFT:
			case SKIDOO_STATE_DISMOUNT_RIGHT:
			case SKIDOO_STATE_DISMOUNT_FALL:
				drive = -1;
				collide = VehicleImpactDirection::None;

				break;

			default:
				drive = SkidooUserControl(skidooItem, laraItem, height, &pitch);
				break;
			}
		}

		if (drive > 0)
		{
			skidoo->Specific.TrackMesh = ((skidoo->Specific.TrackMesh & 3) == 1) ? 2 : 1;
			skidoo->Specific.Pitch += (pitch - skidoo->Specific.Pitch) / 4;

			auto pitch = std::clamp(0.5f + (float)abs(skidoo->Specific.Pitch) / (float)SKIDOO_NORMAL_VELOCITY_MAX, 0.6f, 1.4f);
			SoundEffect(skidoo->Specific.Pitch ? SFX_TR2_VEHICLE_SNOWMOBILE_MOVING : SFX_TR2_VEHICLE_SNOWMOBILE_ACCELERATE, &skidooItem->Pose, SoundEnvironment::Land, pitch);
		}
		else
		{
			skidoo->Specific.TrackMesh = 0;
			if (!drive)
				SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &skidooItem->Pose);
			skidoo->Specific.Pitch = 0;
		}
		skidooItem->Floor = height;

		skidoo->Control.LeftVerticalVelocity = DoVehicleDynamics(heightFrontLeft, skidoo->Control.LeftVerticalVelocity, SKIDOO_BOUNCE_MIN, SKIDOO_KICK_MAX, &frontLeft.y);
		skidoo->Control.RightVerticalVelocity = DoVehicleDynamics(heightFrontRight, skidoo->Control.RightVerticalVelocity, SKIDOO_BOUNCE_MIN, SKIDOO_KICK_MAX, &frontRight.y);
		skidooItem->Animation.VerticalVelocity = DoVehicleDynamics(height, skidooItem->Animation.VerticalVelocity, SKIDOO_BOUNCE_MIN, SKIDOO_KICK_MAX, &skidooItem->Pose.Position.y);
		skidooItem->Animation.Velocity = DoVehicleWaterMovement(skidooItem, laraItem, skidooItem->Animation.Velocity, SKIDOO_RADIUS, &skidoo->Control.TurnRate.y);

		height = (frontLeft.y + frontRight.y) / 2;
		short xRot = phd_atan(SKIDOO_FRONT, skidooItem->Pose.Position.y - height);
		short zRot = phd_atan(SKIDOO_SIDE, height - frontLeft.y);

		skidooItem->Pose.Orientation.x += (xRot - skidooItem->Pose.Orientation.x) / 2;
		skidooItem->Pose.Orientation.z += (zRot - skidooItem->Pose.Orientation.z) / 2;

		if (skidooItem->Flags & IFLAG_INVISIBLE)
		{
			if (probe.RoomNumber != skidooItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			AnimateItem(laraItem);

			if (skidooItem->Pose.Position.y == skidooItem->Floor)
				ExplodeVehicle(laraItem, skidooItem);

			return false;
		}

		AnimateSkidoo(skidooItem, laraItem, collide, isDead);

		if (probe.RoomNumber != skidooItem->RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		if (laraItem->Animation.ActiveState != SKIDOO_STATE_DRIVE_DEATH)
		{
			laraItem->Pose.Position = skidooItem->Pose.Position;
			laraItem->Pose.Orientation.y = skidooItem->Pose.Orientation.y;

			if (drive >= 0)
			{
				laraItem->Pose.Orientation.x = skidooItem->Pose.Orientation.x;
				laraItem->Pose.Orientation.z = skidooItem->Pose.Orientation.z;
			}
			else
				laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;
		}
		else
			laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;

		AnimateItem(laraItem);

		if (!isDead && drive >= 0 && skidoo->Specific.Armed)
			HandleSkidooGuns(skidooItem, laraItem);

		if (!isDead)
		{
			skidooItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex);
			skidooItem->Animation.FrameNumber = g_Level.Anims[skidooItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
		}
		else
		{
			skidooItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE].animIndex + SKIDOO_ANIM_IDLE;
			skidooItem->Animation.FrameNumber = g_Level.Anims[skidooItem->Animation.AnimNumber].frameBase;
		}

		if (skidooItem->Animation.Velocity.z && skidooItem->Floor == skidooItem->Pose.Position.y)
		{
			TriggerSkidooSnowEffect(skidooItem);

			if (skidooItem->Animation.Velocity.z < 50)
				TriggerSkidooSnowEffect(skidooItem);
		}

		return DoSkidooDismount(skidooItem, laraItem);
	}

	VehicleImpactDirection SkidooDynamics(ItemInfo* skidooItem, ItemInfo* laraItem)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);

		// Get point/room collision at vehicle corners.
		auto prevPointFrontLeft = GetVehicleCollision(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, true);
		auto prevPointFrontRight = GetVehicleCollision(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, true);
		auto prevPointBackLeft = GetVehicleCollision(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, true);
		auto prevPointBackRight = GetVehicleCollision(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, true);
		auto prevPos = skidooItem->Pose.Position;
			
		// Apply rotations and determine angle of momentum.
		if (skidooItem->Pose.Position.y > (skidooItem->Floor - SKIDOO_STEP_HEIGHT))
		{
			if (skidoo->Control.TurnRate.y < -SKIDOO_TURN_RATE_DECEL)
				skidoo->Control.TurnRate.y += SKIDOO_TURN_RATE_DECEL;
			else if (skidoo->Control.TurnRate.y > SKIDOO_TURN_RATE_DECEL)
				skidoo->Control.TurnRate.y -= SKIDOO_TURN_RATE_DECEL;
			else
				skidoo->Control.TurnRate.y = 0;
			skidooItem->Pose.Orientation.y += skidoo->Control.TurnRate.y + skidoo->Control.ExtraRotation;

			short rotation = skidooItem->Pose.Orientation.y - skidoo->Control.MomentumAngle;
			if (rotation < -SKIDOO_MOMENTUM_TURN_RATE_ACCEL)
			{
				if (rotation < -SKIDOO_MOMENTUM_TURN_RATE_MAX)
				{
					rotation = -SKIDOO_MOMENTUM_TURN_RATE_MAX;
					skidoo->Control.MomentumAngle = skidooItem->Pose.Orientation.y - rotation;
				}
				else
					skidoo->Control.MomentumAngle -= SKIDOO_MOMENTUM_TURN_RATE_ACCEL;
			}
			else if (rotation > SKIDOO_MOMENTUM_TURN_RATE_ACCEL)
			{
				if (rotation > SKIDOO_MOMENTUM_TURN_RATE_MAX)
				{
					rotation = SKIDOO_MOMENTUM_TURN_RATE_MAX;
					skidoo->Control.MomentumAngle = skidooItem->Pose.Orientation.y - rotation;
				}
				else
					skidoo->Control.MomentumAngle += SKIDOO_MOMENTUM_TURN_RATE_ACCEL;
			}
			else
				skidoo->Control.MomentumAngle = skidooItem->Pose.Orientation.y;
		}
		else
			skidooItem->Pose.Orientation.y += skidoo->Control.TurnRate.y + skidoo->Control.ExtraRotation;

		// Translate vehicle according to momentum angle.
		TranslateItem(skidooItem, skidoo->Control.MomentumAngle, skidooItem->Animation.Velocity);

		// Apply slip. TODO: Determine what "slip" is exactly.
		int slip = SKIDOO_SLIP * phd_sin(skidooItem->Pose.Orientation.x);
		if (abs(slip) > (SKIDOO_SLIP / 2))
		{
			skidooItem->Pose.Position.x -= slip * phd_sin(skidooItem->Pose.Orientation.y);
			skidooItem->Pose.Position.z -= slip * phd_cos(skidooItem->Pose.Orientation.y);
		}

		slip = SKIDOO_SLIP_SIDE * phd_sin(skidooItem->Pose.Orientation.z);
		if (abs(slip) > (SKIDOO_SLIP_SIDE / 2))
		{
			skidooItem->Pose.Position.x += slip * phd_cos(skidooItem->Pose.Orientation.y);
			skidooItem->Pose.Position.z -= slip * phd_sin(skidooItem->Pose.Orientation.y);
		}

		// Store old 2D position to determine movement delta later.
		auto moved = Vector3Int(skidooItem->Pose.Position.x, 0, skidooItem->Pose.Position.z);

		// Process entity collision.
		if (!(skidooItem->Flags & IFLAG_INVISIBLE))
			DoVehicleCollision(skidooItem, SKIDOO_RADIUS);

		if (!(TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT)))
		{
			auto leftProbe = GetCollision(skidooItem, skidooItem->Pose.Position.y, SKIDOO_FRONT, -SKIDOO_SIDE, -SKIDOO_HEIGHT);
			auto rightProbe = GetCollision(skidooItem, skidooItem->Pose.Position.y, SKIDOO_FRONT, SKIDOO_SIDE, -SKIDOO_HEIGHT);

			if (leftProbe.Position.Floor < (skidooItem->Pose.Position.y - SKIDOO_HEIGHT))
				skidooItem->Pose.Orientation.y -= ANGLE(2.0f);
			else if (rightProbe.Position.Floor < (skidooItem->Pose.Position.y - SKIDOO_HEIGHT))
				skidooItem->Pose.Orientation.y += ANGLE(2.0f);
		}

		// Apply shifts.
		short extraRot = 0;
		CalculateVehicleShift(skidooItem, &extraRot, prevPointBackLeft, SKIDOO_HEIGHT, -SKIDOO_FRONT, -SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);
		CalculateVehicleShift(skidooItem, &extraRot, prevPointBackRight, SKIDOO_HEIGHT, -SKIDOO_FRONT, SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);
		CalculateVehicleShift(skidooItem, &extraRot, prevPointFrontLeft, SKIDOO_HEIGHT, SKIDOO_FRONT, -SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);
		CalculateVehicleShift(skidooItem, &extraRot, prevPointFrontRight, SKIDOO_HEIGHT, SKIDOO_FRONT, SKIDOO_SIDE, SKIDOO_STEP_HEIGHT, false);

		auto probe = GetCollision(skidooItem);
		if (probe.Position.Floor < (skidooItem->Pose.Position.y - SKIDOO_STEP_HEIGHT) ||
			abs(probe.Position.Ceiling - probe.Position.Floor) <= SKIDOO_HEIGHT)
		{
			DoVehicleShift(skidooItem, skidooItem->Pose.Position, prevPos);
		}

		skidoo->Control.ExtraRotation = extraRot;

		// Determine whether wall impact occurred and affect vehicle accordingly.
		auto impactDirection = GetVehicleImpactDirection(skidooItem, moved);
		if (impactDirection != VehicleImpactDirection::None)
		{
			int newVelocity = (skidooItem->Pose.Position.z - prevPos.z) * phd_cos(skidoo->Control.MomentumAngle) + (skidooItem->Pose.Position.x - prevPos.x) * phd_sin(skidoo->Control.MomentumAngle);
			if (skidooItem->Animation.Velocity > (SKIDOO_NORMAL_VELOCITY_MAX + SKIDOO_VELOCITY_ACCEL) &&
				newVelocity < (skidooItem->Animation.Velocity - 10))
			{
				DoDamage(laraItem, (skidooItem->Animation.Velocity - newVelocity) / 2);
			}

			if (skidooItem->Animation.Velocity > 0 && newVelocity < skidooItem->Animation.Velocity)
				skidooItem->Animation.Velocity = (newVelocity < 0) ? 0 : newVelocity;
			else if (skidooItem->Animation.Velocity < 0 && newVelocity > skidooItem->Animation.Velocity)
				skidooItem->Animation.Velocity = (newVelocity > 0) ? 0 : newVelocity;

			if (skidooItem->Animation.Velocity < SKIDOO_REVERSE_VELOCITY_MAX)
				skidooItem->Animation.Velocity = SKIDOO_REVERSE_VELOCITY_MAX;*/
		}

		return impactDirection;
	}

	bool SkidooUserControl(ItemInfo* skidooItem, ItemInfo* laraItem, int height, int* pitch)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);

		bool drive = false;

		if (skidooItem->Pose.Position.y >= (height - SKIDOO_STEP_HEIGHT))
		{
			*pitch = skidooItem->Animation.Velocity.z + (height - skidooItem->Pose.Position.y);

			if (TrInput & IN_LOOK && skidooItem->Animation.Velocity.z == 0)
				LookUpDown(laraItem);

			if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT))
				ModulateVehicleTurnRateY(&skidoo->Control.TurnRate.y, SKIDOO_TURN_RATE_ACCEL, -SKIDOO_TURN_RATE_MAX, SKIDOO_TURN_RATE_MAX);

			if (TrInput & VEHICLE_IN_REVERSE)
			{
				if (skidooItem->Animation.Velocity.z > 0)
					skidooItem->Animation.Velocity.z -= SKIDOO_VELOCITY_BRAKE_DECEL;
				else
				{
					drive = true;

					if (skidooItem->Animation.Velocity.z > -SKIDOO_REVERSE_VELOCITY_MAX)
						skidooItem->Animation.Velocity.z -= SKIDOO_REVERSE_VELOCITY_ACCEL;
				}
			}
			else if (TrInput & VEHICLE_IN_ACCELERATE)
			{
				drive = true;

				float maxVelocity;
				if (TrInput & VEHICLE_IN_SLOW)
					maxVelocity = SKIDOO_SLOW_VELOCITY_MAX;
				else if (TrInput & VEHICLE_IN_SPEED)
					maxVelocity = SKIDOO_FAST_VELOCITY_MAX;
				else
					maxVelocity = SKIDOO_NORMAL_VELOCITY_MAX;

				if (skidooItem->Animation.Velocity.z < maxVelocity)
					skidooItem->Animation.Velocity.z += (SKIDOO_VELOCITY_ACCEL / 2) + (SKIDOO_VELOCITY_ACCEL * (skidooItem->Animation.Velocity.z / (2 * maxVelocity)));
				else if (skidooItem->Animation.Velocity.z > (maxVelocity + SKIDOO_VELOCITY_DECEL))
					skidooItem->Animation.Velocity.z -= SKIDOO_VELOCITY_DECEL;
			}
			else if (TrInput & (VEHICLE_IN_LEFT | VEHICLE_IN_RIGHT) &&
				skidooItem->Animation.Velocity.z >= 0 &&
				skidooItem->Animation.Velocity.z < SKIDOO_TURN_VELOCITY_MAX)
			{
				drive = true;
				skidooItem->Animation.Velocity.z = SKIDOO_TURN_VELOCITY_MAX;
			}
			else if (skidooItem->Animation.Velocity.z > SKIDOO_VELOCITY_DECEL)
			{
				skidooItem->Animation.Velocity.z -= SKIDOO_VELOCITY_DECEL;

				if ((GetRandomControl() & 0x7f) < skidooItem->Animation.Velocity.z)
					drive = true;
			}
			else
				skidooItem->Animation.Velocity.z = 0;
		}
		else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
		{
			*pitch = skidoo->Specific.Pitch + 50;
			drive = true;
		}

		return drive;
	}

	void AnimateSkidoo(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection, bool isDead)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);

		if (!isDead &&
			laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
			skidooItem->Animation.Velocity.y > 0 &&
			skidooItem->Pose.Position.y != skidooItem->Floor)
		{
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_LEAP_START;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = SKIDOO_STATE_FALL;
			laraItem->Animation.TargetState = SKIDOO_STATE_FALL;
		}
		else if (!isDead && impactDirection != VehicleImpactDirection::None &&
			laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
			abs(skidooItem->Animation.Velocity.z) >= SKIDOO_REVERSE_VELOCITY_MAX)
		{
			DoSkidooImpact(skidooItem, laraItem, impactDirection);
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case SKIDOO_STATE_IDLE:

				if (isDead)
				{
					laraItem->Animation.TargetState = SKIDOO_STATE_IDLE_DEATH;
					break;
				}

				laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;

				if (TrInput & VEHICLE_IN_DISMOUNT)
				{
					if (TrInput & VEHICLE_IN_RIGHT &&
						TestSkidooDismount(skidooItem, SKIDOO_STATE_DISMOUNT_RIGHT))
					{
						laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_RIGHT;
						skidooItem->Animation.Velocity.z = 0;
					}
					else if (TrInput & VEHICLE_IN_LEFT &&
						TestSkidooDismount(skidooItem, SKIDOO_STATE_DISMOUNT_LEFT))
					{
						laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_LEFT;
						skidooItem->Animation.Velocity.z = 0;
					}
				}
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					if (skidooItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					if (skidooItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
				}
				else if (TrInput & (VEHICLE_IN_ACCELERATE | VEHICLE_IN_REVERSE))
					laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;

				break;

			case SKIDOO_STATE_DRIVE:
				if (!skidooItem->Animation.Velocity.z)
					laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;

				if (isDead)
					laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE_DEATH;
				else if (TrInput & VEHICLE_IN_LEFT)
				{
					if (skidooItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
				}
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					if (skidooItem->Animation.Velocity.z >= 0)
						laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
					else
						laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
				}

				break;

			case SKIDOO_STATE_LEFT:
				if (skidooItem->Animation.Velocity.z >= 0)
				{
					if (!(TrInput & VEHICLE_IN_LEFT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}
				else
				{
					if (!(TrInput & VEHICLE_IN_RIGHT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_RIGHT:
				if (skidooItem->Animation.Velocity.z >= 0)
				{
					if (!(TrInput & VEHICLE_IN_RIGHT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}
				else
				{
					if (!(TrInput & VEHICLE_IN_LEFT))
						laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
				}

				break;

			case SKIDOO_STATE_FALL:
				if (skidooItem->Animation.VerticalVelocity <= 0 ||
					skidoo->Control.LeftVerticalVelocity <= 0 ||
					skidoo->Control.RightVerticalVelocity <= 0)
				{
					laraItem->Animation.TargetState = SKIDOO_STATE_DRIVE;
					SoundEffect(SFX_TR2_VEHICLE_IMPACT3, &skidooItem->Pose);
				}
				else if (skidooItem->Animation.Velocity.y > (SKIDOO_DAMAGE_START + SKIDOO_DAMAGE_LENGTH))
					laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_FALL;

				break;
			}
		}
	}

	void SkidooPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* skidooItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
			return;

		auto mountType = GetVehicleMountType(skidooItem, laraItem, coll, SkidooMountTypes, SKIDOO_MOUNT_DISTANCE);
		if (mountType == VehicleMountType::None)
			ObjectCollision(itemNumber, laraItem, coll);
		else
		{
			lara->Vehicle = itemNumber;
			DoSkidooMount(skidooItem, laraItem, mountType);
		}
	}

	void DoSkidooMount(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleMountType mountType)
	{
		auto* lara = GetLaraInfo(laraItem);

		switch (mountType)
		{
		case VehicleMountType::LevelStart:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_IDLE;
			laraItem->Animation.ActiveState = SKIDOO_STATE_IDLE;
			laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;
			break;

		case VehicleMountType::Left:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_LEFT;
			laraItem->Animation.ActiveState = SKIDOO_STATE_MOUNT_RIGHT;
			laraItem->Animation.TargetState = SKIDOO_STATE_MOUNT_RIGHT;
			break;

		default:
		case VehicleMountType::Right:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_RIGHT;
			laraItem->Animation.ActiveState = SKIDOO_STATE_MOUNT_RIGHT;
			laraItem->Animation.TargetState = SKIDOO_STATE_MOUNT_RIGHT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

		DoVehicleFlareDiscard(laraItem);
		laraItem->Pose.Position = skidooItem->Pose.Position;
		laraItem->Pose.Orientation = Vector3Shrt(0, skidooItem->Pose.Orientation.y, 0);
		lara->Control.HandStatus = HandStatus::Busy;
		skidooItem->Collidable = true;
	}

	bool TestSkidooDismount(ItemInfo* skidooItem, int direction)
	{
		short angle;
		if (direction == SKIDOO_STATE_DISMOUNT_LEFT)
			angle = skidooItem->Pose.Orientation.y + ANGLE(90.0f);
		else
			angle = skidooItem->Pose.Orientation.y - ANGLE(90.0f);

		auto probe = GetCollision(skidooItem, angle, -SKIDOO_DISMOUNT_DISTANCE);

		if ((probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT) ||
			abs(probe.Position.Floor - skidooItem->Pose.Position.y) > CLICK(2) ||
			((probe.Position.Ceiling - skidooItem->Pose.Position.y) > -LARA_HEIGHT ||
				(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT))
		{
			return false;
		}

		return true;
	}

	bool DoSkidooDismount(ItemInfo* skidooItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (lara->Vehicle != NO_ITEM)
		{
			if ((laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_RIGHT || laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT) &&
				laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				if (laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT)
					laraItem->Pose.Orientation.y += ANGLE(90.0f);
				else
					laraItem->Pose.Orientation.y -= ANGLE(90.0f);

				SetAnimation(laraItem, LA_STAND_IDLE);
				TranslateItem(laraItem, laraItem->Pose.Orientation.y, -SKIDOO_DISMOUNT_DISTANCE);
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Vehicle = NO_ITEM;
			}
			else if (laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_FALL &&
				(skidooItem->Pose.Position.y == skidooItem->Floor || TestLastFrame(laraItem)))
			{
				SetAnimation(laraItem, LA_FREEFALL);

				if (skidooItem->Pose.Position.y == skidooItem->Floor)
				{
					laraItem->Animation.TargetState = LS_DEATH;
					laraItem->Animation.Velocity.z = 0;
					laraItem->Animation.Velocity.y = SKIDOO_DAMAGE_START + SKIDOO_DAMAGE_LENGTH;
					ExplodeVehicle(laraItem, skidooItem);
				}
				else
				{
					laraItem->Animation.TargetState = LS_FREEFALL;
					laraItem->Pose.Position.y -= 200;
					laraItem->Animation.Velocity = skidooItem->Animation.Velocity;
					laraItem->Animation.Velocity.y = skidooItem->Animation.Velocity.y;
					SoundEffect(SFX_TR4_LARA_FALL, &laraItem->Pose);
				}

				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				laraItem->Animation.IsAirborne = true;
				lara->Control.MoveAngle = skidooItem->Pose.Orientation.y;
				lara->Control.HandStatus = HandStatus::Free;
				skidooItem->Collidable = false;
				skidooItem->Flags |= IFLAG_INVISIBLE;

				return false;
			}

			return true;
		}
		else
			return true;
	}

	void DoSkidooImpact(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection)
	{
		if (laraItem->Animation.ActiveState == SKIDOO_STATE_IMPACT)
			return;

		switch (impactDirection)
		{
		default:
		case VehicleImpactDirection::Front:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_IMPACT_FRONT;
			break;

		case VehicleImpactDirection::Back:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_IMPACT_BACK;
			break;

		case VehicleImpactDirection::Left:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_IMPACT_LEFT;
			break;

		case VehicleImpactDirection::Right:
			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_IMPACT_RIGHT;
			break;
		}
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = SKIDOO_STATE_IMPACT;
		laraItem->Animation.TargetState = SKIDOO_STATE_IMPACT;

		if (impactDirection == VehicleImpactDirection::Front)
			SoundEffect(SFX_TR2_VEHICLE_IMPACT1, &skidooItem->Pose);
		else
			SoundEffect(SFX_TR2_VEHICLE_IMPACT2, &skidooItem->Pose);
	}

	void HandleSkidooGuns(ItemInfo* skidooItem, ItemInfo* laraItem)
	{
		auto* skidoo = GetSkidooInfo(skidooItem);
		auto* lara = GetLaraInfo(laraItem);
		auto* weapon = &Weapons[(int)LaraWeaponType::Snowmobile];

		LaraGetNewTarget(laraItem, weapon);
		AimWeapon(laraItem, weapon, &lara->RightArm);

		if (TrInput & VEHICLE_IN_FIRE && !skidooItem->ItemFlags[0])
		{
			auto angles = Vector3Shrt(
				lara->RightArm.Orientation.x,
				lara->RightArm.Orientation.y + laraItem->Pose.Orientation.y,
				0
			);

			if ((int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles) +
				(int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles))
			{
				skidoo->Specific.FlashTimer = 2;
				SoundEffect(weapon->SampleNum, &laraItem->Pose);
				skidooItem->ItemFlags[0] = 4;
			}
		}

		if (skidooItem->ItemFlags[0])
			skidooItem->ItemFlags[0]--;
	}

	void TriggerSkidooSnowEffect(ItemInfo* skidooItem)
	{
		auto material = GetCollision(skidooItem).BottomBlock->Material;
		if (material == FLOOR_MATERIAL::Ice || material == FLOOR_MATERIAL::Snow)
			TEN::Effects::TriggerSnowmobileSnow(skidooItem);
	}
}
