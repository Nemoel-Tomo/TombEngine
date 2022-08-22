#include "framework.h"
#include "Objects/Utils/Vehicle.h"

#include "Objects/Utils/VehicleHelpers.h"
#include "Specific/input.h"

using namespace TEN::Input;

namespace TEN::Entities::Vehicles
{
	// -------
	// PUBLIC:
	// -------

	void AbstractVehicle::ModulateTurnRateX(short accelRate, short minTurnRate, short maxTurnRate, bool invert)
	{
		this->Control.TurnRate.x = ModulateTurnRate(Control.TurnRate.x, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveVertical], invert);
	}

	void AbstractVehicle::ModulateTurnRateY(short accelRate, short minTurnRate, short maxTurnRate, bool invert)
	{
		this->Control.TurnRate.y = ModulateTurnRate(Control.TurnRate.y, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveHorizontal], invert);
	}

	void AbstractVehicle::UndoTurnRateX(short decelRate)
	{
		this->Control.TurnRate.x = UndoTurnRate(Control.TurnRate.x, decelRate);
	}

	void AbstractVehicle::UndoTurnRateY(short decelRate)
	{
		this->Control.TurnRate.y = UndoTurnRate(Control.TurnRate.y, decelRate);
	}

	// --------
	// PRIVATE:
	// --------

	// TODO: Vehicle turn rates *MUST* be affected by vehicle speed for more tactile modulation.
	short AbstractVehicle::ModulateTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert)
	{
		axisCoeff *= invert ? -1 : 1;
		int sign = std::copysign(1, axisCoeff);

		short minTurnRateNormalized = minTurnRate * abs(axisCoeff);
		short maxTurnRateNormalized = maxTurnRate * abs(axisCoeff);

		short newTurnRate = (turnRate + (accelRate * sign)) * sign;
		newTurnRate = std::clamp(newTurnRate, minTurnRateNormalized, maxTurnRateNormalized);
		return (newTurnRate * sign);
	}

	short AbstractVehicle::UndoTurnRate(short turnRate, short decelRate)
	{
		int sign = std::copysign(1, turnRate);

		if (abs(turnRate) <= decelRate)
			return 0;
		else if (abs(turnRate) > decelRate)
			return (turnRate + (decelRate * -sign));
	}
}
