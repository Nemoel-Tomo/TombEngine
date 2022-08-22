#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct Vector3Int;
struct Vector3Shrt;

using std::vector;

// I don't know what I'm doing. Just let it happen.

namespace TEN::Entities::Vehicles
{
	// Maybe not necessary.
	enum class VehicleType
	{
		// TR2:
		Skidoo,
		Speedboat,

		// TR3:
		BigGun,
		Kayak,
		Minecart,
		QuadBike,
		RubberBoat,
		UPV,

		// TR4:
		Jeep,
		Motorbike
	};

	struct VehicleAttributes
	{
		int Radius = 0;
		int Length = 0;
		int Width = 0;
		int Height = 0;
		
		int StepHeight = 0;
		int Bounce = 0;
		int Kick = 0;
		int MountDistance = 0;
		int DismountDistance = 0; //
	};

	struct VehicleControl
	{
		int Velocity = 0;
		int VerticalVelocity = 0;
		int LeftVerticalVelocity = 0;
		int RightVerticalVelocity = 0;

		Vector3Shrt TurnRate = Vector3Shrt();
		short MomentumAngle = 0;
		short ExtraRotation = 0;
	};

	class VehicleController
	{
	public:

	};

	class AbstractVehicle
	{
	public:
		VehicleAttributes  Attributes = {};
		VehicleControl	   Control = {};
		vector<Vector3Int> CollisionPoints = {};

		virtual void ModulateTurnRateX(short accelRate, short minTurnRate, short maxTurnRate, bool invert = true);
		virtual void ModulateTurnRateY(short accelRate, short minTurnRate, short maxTurnRate, bool invert = false);
		virtual void UndoTurnRateX(short decelRate);
		virtual void UndoTurnRateY(short decelRate);

	private:
		virtual short ModulateTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert);
		virtual short UndoTurnRate(short turnRate, short decelRate);
	};
}
