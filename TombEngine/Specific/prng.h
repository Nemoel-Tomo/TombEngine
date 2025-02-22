#pragma once
#include <limits>

namespace TEN::Math::Random
{
	int32_t GenerateInt(int32_t low = 0, int32_t high = std::numeric_limits<int16_t>::max());
	float	GenerateFloat(float low = 0.0f, float high = 1.0f);

	bool TestProbability(float probability);
}
