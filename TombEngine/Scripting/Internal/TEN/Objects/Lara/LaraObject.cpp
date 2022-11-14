#include "framework.h"
#include "LaraObject.h"

#include "lara_fx.h"
#include "lara_helpers.h"
#include "lara_struct.h"
#include "ReservedScriptNames.h"

/***
Class for extra Lara-only functions.
Do not try to create an object of this type; use the built-in *Lara* variable instead.

In addition, LaraObject inherits all the functions of @{Objects.Moveable|Moveable}.

@tenclass Objects.LaraObject
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ ScriptReserved_LaraObject };

/// Set Lara on fire
// @function LaraObject:SetOnFire
// @bool fire true to set lara on fire, false to extinguish her
void LaraObject::SetOnFire(bool onFire)
{
	//todo add support for other BurnTypes -squidshire 11/11/2022
	auto* lara = GetLaraInfo(m_item);
	if (onFire && lara->BurnType == BurnType::None )
	{
		TEN::Effects::Lara::LaraBurn(m_item);
		lara->BurnType = BurnType::Normal;
	}
	else if(!onFire)
	{
		lara->BurnType = BurnType::None;
	}
}

/// Get whether Lara is on fire or not
// @function LaraObject:GetOnFire
// @treturn bool fire true if lara is on fire, false otherwise
bool LaraObject::GetOnFire() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->BurnType != BurnType::None;
}

/// Set Poison with potency of poision
// @function LaraObject:SetPoison
// @tparam Potency Value 
// @usage
// Lara:SetPoison(10)
// Max Value: 64
void LaraObject::SetPoison(int potency = 0)
{
	auto* lara = GetLaraInfo(m_item);

	lara->PoisonPotency = potency;
}

/// Get Poison potency of Lara
// @function LaraObject:GetPoison
// @usage
// Lara:GetPoison()
int LaraObject::GetPoison()
{
	auto* lara = GetLaraInfo(m_item);

	return lara->PoisonPotency;
}

/// Remove Poison
// @function Moveable:RemovePoison
// @usage
// Lara:RemovePoison()
void LaraObject::RemovePoison()
{
	auto* lara = GetLaraInfo(m_item);

	lara->PoisonPotency = 0;
}

/// Set Air of Lara
// @function LaraObject:SetAir
// @tparam Air Value 
// @usage
// Lara:SetAir(100)
// Max Value: 1800
void LaraObject::SetAir(int air = 0)
{
	auto* lara = GetLaraInfo(m_item);

	lara->Air = air;
}

/// Get Air value of Lara
// @function LaraObject:GetAir
// @usage
// Lara:GetAir()
int LaraObject::GetAir()
{
	auto* lara = GetLaraInfo(m_item);

	return lara->Air;
}

void LaraObject::Register(sol::table& parent)
{
	parent.new_usertype<LaraObject>(LUA_CLASS_NAME,
			ScriptReserved_SetOnFire, &LaraObject::SetOnFire,
			ScriptReserved_GetOnFire, &LaraObject::GetOnFire,
			ScriptReserved_SetPoison, &LaraObject::SetPoison,
			ScriptReserved_GetPoison, &LaraObject::GetPoison,
			ScriptReserved_RemovePoison, &LaraObject::RemovePoison,
			ScriptReserved_SetAir, &LaraObject::SetAir,
			ScriptReserved_GetAir, &LaraObject::GetAir,
			sol::base_classes, sol::bases<Moveable>()
		);
}

