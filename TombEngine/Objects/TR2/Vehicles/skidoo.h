#pragma once
#include "Game/Lara/lara.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Objects/Utils/Vehicle.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	constexpr auto SKIDOO_HEIGHT = LARA_HEIGHT;
	constexpr auto SKIDOO_RADIUS = 500;
	constexpr auto SKIDOO_FRONT = 550;
	constexpr auto SKIDOO_SIDE = 260;
	constexpr auto SKIDOO_SLIP = 100;
	constexpr auto SKIDOO_SLIP_SIDE = 50;

	constexpr auto SKIDOO_VELOCITY_ACCEL = 10;
	constexpr auto SKIDOO_VELOCITY_DECEL = 2;
	constexpr auto SKIDOO_VELOCITY_BRAKE_DECEL = 5;
	constexpr auto SKIDOO_REVERSE_VELOCITY_ACCEL = 5;

	constexpr auto SKIDOO_SLOW_VELOCITY_MAX = 50;
	constexpr auto SKIDOO_NORMAL_VELOCITY_MAX = 100;
	constexpr auto SKIDOO_FAST_VELOCITY_MAX = 150;
	constexpr auto SKIDOO_TURN_VELOCITY_MAX = 15;
	constexpr auto SKIDOO_REVERSE_VELOCITY_MAX = 30;

	constexpr auto SKIDOO_STEP_HEIGHT = CLICK(1);
	constexpr auto SKIDOO_BOUNCE_MIN = (SKIDOO_NORMAL_VELOCITY_MAX / 2) / 256;
	constexpr auto SKIDOO_KICK_MAX = -80;
	constexpr auto SKIDOO_MOUNT_DISTANCE = CLICK(2);
	constexpr auto SKIDOO_DISMOUNT_DISTANCE = 295;
	constexpr auto SKIDOO_DAMAGE_START = 140;
	constexpr auto SKIDOO_DAMAGE_LENGTH = 14;

	#define SKIDOO_TURN_RATE_ACCEL			ANGLE(2.5f)
	#define SKIDOO_TURN_RATE_DECEL			ANGLE(2.0f)
	#define SKIDOO_TURN_RATE_MAX			ANGLE(6.0f)
	#define SKIDOO_MOMENTUM_TURN_RATE_ACCEL	ANGLE(3.0f)
	#define SKIDOO_MOMENTUM_TURN_RATE_MAX	ANGLE(150.0f)

	enum SkidooState
	{
		SKIDOO_STATE_DRIVE = 0,
		SKIDOO_STATE_MOUNT = 1,
		SKIDOO_STATE_LEFT = 2,
		SKIDOO_STATE_RIGHT = 3,
		SKIDOO_STATE_FALL = 4,
		SKIDOO_STATE_IMPACT = 5,
		SKIDOO_STATE_MOUNT_LEFT = 6,
		SKIDOO_STATE_DISMOUNT_LEFT = 7,
		SKIDOO_STATE_IDLE = 8,
		SKIDOO_STATE_DISMOUNT_RIGHT = 9,
		SKIDOO_STATE_DISMOUNT_JUMP = 10,
		SKIDOO_STATE_IDLE_DEATH = 11,
		SKIDOO_STATE_DRIVE_DEATH = 12
	};

	enum SkidooAnim
	{
		SKIDOO_ANIM_DRIVE = 0,
		SKIDOO_ANIM_MOUNT_RIGHT = 1,
		SKIDOO_ANIM_TURN_LEFT_START = 2,
		SKIDOO_ANIM_TURN_LEFT_CONTINUE = 3,
		SKIDOO_ANIM_TURN_LEFT_END = 4,
		SKIDOO_ANIM_TURN_RIGHT_START = 5,
		SKIDOO_ANIM_TURN_RIGHT_CONTINUE = 6,
		SKIDOO_ANIM_TURN_RIGHT_END = 7,
		SKIDOO_ANIM_LEAP_START = 8,
		SKIDOO_ANIM_LEAP_END = 9,
		SKIDOO_ANIM_LEAP_CONTINUE = 10,
		SKIDOO_ANIM_IMPACT_LEFT = 11,
		SKIDOO_ANIM_IMPACT_RIGHT = 12,
		SKIDOO_ANIM_IMPACT_FRONT = 13,
		SKIDOO_ANIM_IMPACT_BACK = 14,
		SKIDOO_ANIM_IDLE = 15,
		SKIDOO_ANIM_DISMOUNT_RIGHT = 16,
		SKIDOO_ANIM_UNK = 17, // TODO
		SKIDOO_ANIM_MOUNT_LEFT = 18,
		SKIDOO_ANIM_DISMOUNT_LEFT = 19,
		SKIDOO_ANIM_FALL_OFF = 20, // DISMOUNT_JUMP?
		SKIDOO_ANIM_IDLE_DEATH = 21,
		SKIDOO_ANIM_FALL_DEATH = 22
	};

	const vector<VehicleMountType> SkidooMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Left,
		VehicleMountType::Right
	};

	struct SkidooAttributes
	{
		int   Pitch		 = 0;
		int	  FlashTimer = 0;
		short TrackMesh	 = 0;
		bool  Armed		 = false;
	};

	class SkidooObject : public AbstractVehicle
	{
	public:
		SkidooAttributes Specific = {};

		SkidooObject();
		void InitializeSkidoo(short itemNumber);
	};

	SkidooObject* GetSkidooInfo(ItemInfo* skidooItem);
	void InitialiseSkidoo(short itemNumber);

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll);
	VehicleImpactDirection SkidooDynamics(ItemInfo* skidooItem, ItemInfo* laraItem);
	bool SkidooUserControl(ItemInfo* skidooItem, ItemInfo* laraItem, int height, int* pitch);
	void AnimateSkidoo(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection, bool isDead);

	void SkidooPlayerCollision(short skidooItemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoSkidooMount(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestSkidooDismount(ItemInfo* skidooItem, int direction);
	bool DoSkidooDismount(ItemInfo* skidooItem, ItemInfo* laraItem);
	void DoSkidooImpact(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection);

	void HandleSkidooGuns(ItemInfo* skidooItem, ItemInfo* laraItem);
	void TriggerSkidooSnowEffect(ItemInfo* skidooItem);
}
