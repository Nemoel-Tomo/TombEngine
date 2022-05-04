#include "framework.h"
#include "Game/savegame.h"

#include <filesystem>
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/control/lot.h"
#include "Game/effects/lara_fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/spotcam.h"
#include "Game/room.h"
#include "Objects/Generic/Object/rope.h"
#include "Objects/Generic/Switches/fullblock_switch.h"
#include "Objects/Generic/Traps/traps.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/savegame/flatbuffers/ten_savegame_generated.h"


using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Switches;
using namespace TEN::Entities::TR4;
using namespace TEN::Entities::Generic;
using namespace TEN::Floordata;
using namespace flatbuffers;

namespace Save = TEN::Save;

const std::string SAVEGAME_PATH = "Save//";

GameStats Statistics;
SaveGameHeader SavegameInfos[SAVEGAME_MAX];

FileStream* SaveGame::m_stream;
std::vector<LuaVariable> SaveGame::m_luaVariables;
int SaveGame::LastSaveGame;

void LoadSavegameInfos()
{
	for (int i = 0; i < SAVEGAME_MAX; i++)
		SavegameInfos[i].Present = false;

	if (!std::filesystem::exists(SAVEGAME_PATH))
		return;

	// try to load the savegame
	for (int i = 0; i < SAVEGAME_MAX; i++)
	{
		auto fileName = SAVEGAME_PATH + "savegame." + std::to_string(i);
		auto savegamePtr = fopen(fileName.c_str(), "rb");

		if (savegamePtr == NULL)
			continue;

		fclose(savegamePtr);

		SavegameInfos[i].Present = true;
		SaveGame::LoadHeader(i, &SavegameInfos[i]);

		fclose(savegamePtr);
	}

	return;
}

bool SaveGame::Save(int slot)
{
	auto fileName = std::string(SAVEGAME_PATH) + "savegame." + std::to_string(slot);

	ITEM_INFO itemToSerialize{};
	FlatBufferBuilder fbb{};

	std::vector<flatbuffers::Offset< Save::Item>> serializedItems{};

	// Savegame header
	auto levelNameOffset = fbb.CreateString(g_GameFlow->GetString(g_GameFlow->GetLevel(CurrentLevel)->NameStringKey.c_str()));

	Save::SaveGameHeaderBuilder sghb{ fbb };
	sghb.add_level_name(levelNameOffset);
	sghb.add_days((GameTimer / 30) / 8640);
	sghb.add_hours(((GameTimer / 30) % 86400) / 3600);
	sghb.add_minutes(((GameTimer / 30) / 60) % 6);
	sghb.add_seconds((GameTimer / 30) % 60);
	sghb.add_level(CurrentLevel);
	sghb.add_timer(GameTimer);
	sghb.add_count(++LastSaveGame);
	auto headerOffset = sghb.Finish();

	Save::SaveGameStatisticsBuilder sgLevelStatisticsBuilder{ fbb };
	sgLevelStatisticsBuilder.add_ammo_hits(Statistics.Level.AmmoHits);
	sgLevelStatisticsBuilder.add_ammo_used(Statistics.Level.AmmoUsed);
	sgLevelStatisticsBuilder.add_kills(Statistics.Level.Kills);
	sgLevelStatisticsBuilder.add_medipacks_used(Statistics.Level.HealthUsed);
	sgLevelStatisticsBuilder.add_distance(Statistics.Level.Distance);
	sgLevelStatisticsBuilder.add_secrets(Statistics.Level.Secrets);
	sgLevelStatisticsBuilder.add_timer(Statistics.Level.Timer);
	auto levelStatisticsOffset = sgLevelStatisticsBuilder.Finish();

	Save::SaveGameStatisticsBuilder sgGameStatisticsBuilder{ fbb };
	sgGameStatisticsBuilder.add_ammo_hits(Statistics.Game.AmmoHits);
	sgGameStatisticsBuilder.add_ammo_used(Statistics.Game.AmmoUsed);
	sgGameStatisticsBuilder.add_kills(Statistics.Game.Kills);
	sgGameStatisticsBuilder.add_medipacks_used(Statistics.Game.HealthUsed);
	sgGameStatisticsBuilder.add_distance(Statistics.Game.Distance);
	sgGameStatisticsBuilder.add_secrets(Statistics.Game.Secrets);
	sgGameStatisticsBuilder.add_timer(Statistics.Game.Timer);
	auto gameStatisticsOffset = sgGameStatisticsBuilder.Finish();

	// Lara
	std::vector<int> puzzles;
	for (int i = 0; i < NUM_PUZZLES; i++)
		puzzles.push_back(Lara.Inventory.Puzzles[i]);
	auto puzzlesOffset = fbb.CreateVector(puzzles);

	std::vector<int> puzzlesCombo;
	for (int i = 0; i < NUM_PUZZLES * 2; i++)
		puzzlesCombo.push_back(Lara.Inventory.PuzzlesCombo[i]);
	auto puzzlesComboOffset = fbb.CreateVector(puzzlesCombo);

	std::vector<int> keys;
	for (int i = 0; i < NUM_KEYS; i++)
		keys.push_back(Lara.Inventory.Keys[i]);
	auto keysOffset = fbb.CreateVector(keys);

	std::vector<int> keysCombo;
	for (int i = 0; i < NUM_KEYS * 2; i++)
		keysCombo.push_back(Lara.Inventory.KeysCombo[i]);
	auto keysComboOffset = fbb.CreateVector(keysCombo);

	std::vector<int> pickups;
	for (int i = 0; i < NUM_PICKUPS; i++)
		pickups.push_back(Lara.Inventory.Pickups[i]);
	auto pickupsOffset = fbb.CreateVector(pickups);

	std::vector<int> pickupsCombo;
	for (int i = 0; i < NUM_PICKUPS * 2; i++)
		pickupsCombo.push_back(Lara.Inventory.PickupsCombo[i]);
	auto pickupsComboOffset = fbb.CreateVector(pickupsCombo);

	std::vector<int> examines;
	for (int i = 0; i < NUM_EXAMINES; i++)
		examines.push_back(Lara.Inventory.Examines[i]);
	auto examinesOffset = fbb.CreateVector(examines);

	std::vector<int> examinesCombo;
	for (int i = 0; i < NUM_EXAMINES * 2; i++)
		examinesCombo.push_back(Lara.Inventory.ExaminesCombo[i]);
	auto examinesComboOffset = fbb.CreateVector(examinesCombo);

	std::vector<int> meshPtrs;
	for (int i = 0; i < 15; i++)
		meshPtrs.push_back(Lara.MeshPtrs[i]);
	auto meshPtrsOffset = fbb.CreateVector(meshPtrs);

	std::vector<byte> wet;
	for (int i = 0; i < 15; i++)
		wet.push_back(Lara.Wet[i] == 1);
	auto wetOffset = fbb.CreateVector(wet);

	Save::Vector3 nextCornerPos = Save::Vector3(Lara.NextCornerPos.Position.x, Lara.NextCornerPos.Position.y, Lara.NextCornerPos.Position.z);
	Save::Vector3 nextCornerRot = Save::Vector3(Lara.NextCornerPos.Orientation.GetX(), Lara.NextCornerPos.Orientation.GetY(), Lara.NextCornerPos.Orientation.GetZ());

	Save::Vector3 leftArmRotation = Save::Vector3(Lara.LeftArm.Orientation.GetX(), Lara.LeftArm.Orientation.GetY(), Lara.LeftArm.Orientation.GetZ());
	Save::Vector3 rightArmRotation = Save::Vector3(Lara.RightArm.Orientation.GetX(), Lara.RightArm.Orientation.GetY(), Lara.RightArm.Orientation.GetZ());
	
	Save::Vector3 extraHeadRot = Save::Vector3(Lara.ExtraHeadRot.GetX(), Lara.ExtraHeadRot.GetY(), Lara.ExtraHeadRot.GetZ());
	Save::Vector3 extraTorsoRot = Save::Vector3(Lara.ExtraTorsoRot.GetX(), Lara.ExtraTorsoRot.GetY(), Lara.ExtraTorsoRot.GetZ());
	Save::Vector3 extraVelocity = Save::Vector3(Lara.ExtraVelocity.x, Lara.ExtraVelocity.y, Lara.ExtraVelocity.z);
	Save::Vector3 waterCurrentPull = Save::Vector3(Lara.WaterCurrentPull.x, Lara.WaterCurrentPull.y, Lara.WaterCurrentPull.z);

	std::vector<int> laraTargetAngles{};
	laraTargetAngles.push_back(Lara.TargetArmOrient.GetY());
	laraTargetAngles.push_back(Lara.TargetArmOrient.GetX());
	auto laraTargetAnglesOffset = fbb.CreateVector(laraTargetAngles);

	std::vector<int> subsuitVelocity{};
	subsuitVelocity.push_back(Lara.Control.Subsuit.Velocity[0]);
	subsuitVelocity.push_back(Lara.Control.Subsuit.Velocity[1]);
	auto subsuitVelocityOffset = fbb.CreateVector(subsuitVelocity);

	Save::HolsterInfoBuilder holsterInfo{ fbb };
	holsterInfo.add_back_holster((int)Lara.Control.Weapon.HolsterInfo.BackHolster);
	holsterInfo.add_left_holster((int)Lara.Control.Weapon.HolsterInfo.LeftHolster);
	holsterInfo.add_right_holster((int)Lara.Control.Weapon.HolsterInfo.RightHolster);
	auto holsterInfoOffset = holsterInfo.Finish();

	Save::ArmInfoBuilder leftArm{ fbb };
	leftArm.add_anim_number(Lara.LeftArm.AnimNumber);
	leftArm.add_flash_gun(Lara.LeftArm.FlashGun);
	leftArm.add_frame_base(Lara.LeftArm.FrameBase);
	leftArm.add_frame_number(Lara.LeftArm.FrameNumber);
	leftArm.add_locked(Lara.LeftArm.Locked);
	leftArm.add_rotation(&leftArmRotation);
	auto leftArmOffset = leftArm.Finish();

	Save::ArmInfoBuilder rightArm{ fbb };
	rightArm.add_anim_number(Lara.RightArm.AnimNumber);
	rightArm.add_flash_gun(Lara.RightArm.FlashGun);
	rightArm.add_frame_base(Lara.RightArm.FrameBase);
	rightArm.add_frame_number(Lara.RightArm.FrameNumber);
	rightArm.add_locked(Lara.RightArm.Locked);
	rightArm.add_rotation(&rightArmRotation);
	auto rightArmOffset = rightArm.Finish();

	Save::FlareDataBuilder flare{ fbb };
	flare.add_control_left(Lara.Flare.ControlLeft);
	flare.add_frame(Lara.Flare.Frame);
	flare.add_life(Lara.Flare.Life);
	auto flareOffset = flare.Finish();

	Save::LaraInventoryDataBuilder inventory{ fbb };
	inventory.add_beetle_life(Lara.Inventory.BeetleLife);
	inventory.add_big_waterskin(Lara.Inventory.BigWaterskin);
	inventory.add_examines(examinesOffset);
	inventory.add_examines_combo(examinesComboOffset);
	inventory.add_beetle_components(Lara.Inventory.BeetleComponents);
	inventory.add_has_binoculars(Lara.Inventory.HasBinoculars);
	inventory.add_has_crowbar(Lara.Inventory.HasCrowbar);
	inventory.add_has_lasersight(Lara.Inventory.HasLasersight);
	inventory.add_has_silencer(Lara.Inventory.HasSilencer);
	inventory.add_has_torch(Lara.Inventory.HasTorch);
	inventory.add_is_busy(Lara.Inventory.IsBusy);
	inventory.add_keys(keysOffset);
	inventory.add_keys_combo(keysComboOffset);
	inventory.add_old_busy(Lara.Inventory.OldBusy);
	inventory.add_puzzles(puzzlesOffset);
	inventory.add_puzzles_combo(puzzlesComboOffset);
	inventory.add_pickups(pickupsOffset);
	inventory.add_pickups_combo(pickupsComboOffset);
	inventory.add_small_waterskin(Lara.Inventory.SmallWaterskin);
	inventory.add_total_flares(Lara.Inventory.TotalFlares);
	inventory.add_total_small_medipacks(Lara.Inventory.TotalSmallMedipacks);
	inventory.add_total_large_medipacks(Lara.Inventory.TotalLargeMedipacks);
	inventory.add_total_secrets(Lara.Inventory.TotalSecrets);
	auto inventoryOffset = inventory.Finish();

	Save::LaraCountDataBuilder count{ fbb };
	count.add_death(Lara.Control.Count.Death);
	count.add_no_cheat(Lara.Control.Count.NoCheat);
	count.add_pose(Lara.Control.Count.Pose);
	count.add_position_adjust(Lara.Control.Count.PositionAdjust);
	count.add_run_jump(Lara.Control.Count.RunJump);
	auto countOffset = count.Finish();

	Save::WeaponControlDataBuilder weaponControl{ fbb };
	weaponControl.add_weapon_item(Lara.Control.Weapon.WeaponItem);
	weaponControl.add_has_fired(Lara.Control.Weapon.HasFired);
	weaponControl.add_fired(Lara.Control.Weapon.Fired);
	weaponControl.add_uzi_left(Lara.Control.Weapon.UziLeft);
	weaponControl.add_uzi_right(Lara.Control.Weapon.UziRight);
	weaponControl.add_gun_type((int)Lara.Control.Weapon.GunType);
	weaponControl.add_request_gun_type((int)Lara.Control.Weapon.RequestGunType);
	weaponControl.add_last_gun_type((int)Lara.Control.Weapon.LastGunType);
	weaponControl.add_holster_info(holsterInfoOffset);
	auto weaponControlOffset = weaponControl.Finish();

	Save::RopeControlDataBuilder ropeControl{ fbb };
	ropeControl.add_segment(Lara.Control.Rope.Segment);
	ropeControl.add_direction(Lara.Control.Rope.Direction);
	ropeControl.add_arc_front(Lara.Control.Rope.ArcFront);
	ropeControl.add_arc_back(Lara.Control.Rope.ArcBack);
	ropeControl.add_last_x(Lara.Control.Rope.LastX);
	ropeControl.add_max_x_forward(Lara.Control.Rope.MaxXForward);
	ropeControl.add_max_x_backward(Lara.Control.Rope.MaxXBackward);
	ropeControl.add_dframe(Lara.Control.Rope.DFrame);
	ropeControl.add_frame(Lara.Control.Rope.Frame);
	ropeControl.add_frame_rate(Lara.Control.Rope.FrameRate);
	ropeControl.add_y(Lara.Control.Rope.Y);
	ropeControl.add_ptr(Lara.Control.Rope.Ptr);
	ropeControl.add_offset(Lara.Control.Rope.Offset);
	ropeControl.add_down_vel(Lara.Control.Rope.DownVel);
	ropeControl.add_flag(Lara.Control.Rope.Flag);
	ropeControl.add_count(Lara.Control.Rope.Count);
	auto ropeControlOffset = ropeControl.Finish();

	Save::TightropeControlDataBuilder tightropeControl{ fbb };
	tightropeControl.add_balance(Lara.Control.Tightrope.Balance);
	tightropeControl.add_can_dismount(Lara.Control.Tightrope.CanDismount);
	tightropeControl.add_tightrope_item(Lara.Control.Tightrope.TightropeItem);
	tightropeControl.add_time_on_tightrope(Lara.Control.Tightrope.TimeOnTightrope);
	auto tightropeControlOffset = tightropeControl.Finish();

	Save::SubsuitControlDataBuilder subsuitControl{ fbb };
	subsuitControl.add_x_rot(Lara.Control.Subsuit.XRot);
	subsuitControl.add_d_x_rot(Lara.Control.Subsuit.DXRot);
	subsuitControl.add_velocity(subsuitVelocityOffset);
	subsuitControl.add_vertical_velocity(Lara.Control.Subsuit.VerticalVelocity);
	subsuitControl.add_x_rot_vel(Lara.Control.Subsuit.XRotVel);
	subsuitControl.add_hit_count(Lara.Control.Subsuit.HitCount);
	auto subsuitControlOffset = subsuitControl.Finish();

	Save::MinecartControlDataBuilder minecartControl{ fbb };
	minecartControl.add_left(Lara.Control.Minecart.Left);
	minecartControl.add_right(Lara.Control.Minecart.Right);
	auto minecartControlOffset = minecartControl.Finish();

	Save::LaraControlDataBuilder control{ fbb };
	control.add_move_angle(Lara.Control.MoveAngle);
	control.add_turn_rate(Lara.Control.TurnRate);
	control.add_calculated_jump_velocity(Lara.Control.CalculatedJumpVelocity);
	control.add_jump_direction((int)Lara.Control.JumpDirection);
	control.add_hand_status((int)Lara.Control.HandStatus);
	control.add_is_moving(Lara.Control.IsMoving);
	control.add_run_jump_queued(Lara.Control.RunJumpQueued);
	control.add_can_look(Lara.Control.CanLook);
	control.add_count(countOffset);
	control.add_keep_low(Lara.Control.KeepLow);
	control.add_is_low(Lara.Control.IsLow);
	control.add_can_climb_ladder(Lara.Control.CanClimbLadder);
	control.add_is_climbing_ladder(Lara.Control.IsClimbingLadder);
	control.add_can_monkey_swing(Lara.Control.CanMonkeySwing);
	control.add_locked(Lara.Control.Locked);
	control.add_minecart(minecartControlOffset);
	control.add_rope(ropeControlOffset);
	control.add_subsuit(subsuitControlOffset);
	control.add_tightrope(tightropeControlOffset);
	control.add_water_status((int)Lara.Control.WaterStatus);
	control.add_weapon(weaponControlOffset);
	auto controlOffset = control.Finish();

	std::vector<flatbuffers::Offset<Save::CarriedWeaponInfo>> carriedWeapons;
	for (int i = 0; i < (int)LaraWeaponType::NumWeapons; i++)
	{
		CarriedWeaponInfo* info = &Lara.Weapons[i];
		
		std::vector<flatbuffers::Offset<Save::AmmoInfo>> ammos;
		for (int j = 0; j < (int)WeaponAmmoType::NumAmmos; j++)
		{
			Save::AmmoInfoBuilder ammo{ fbb };
			ammo.add_count(info->Ammo[j].getCount());
			ammo.add_is_infinite(info->Ammo[j].hasInfinite());
			auto ammoOffset = ammo.Finish();
			ammos.push_back(ammoOffset);
		}
		auto ammosOffset = fbb.CreateVector(ammos);
		
		Save::CarriedWeaponInfoBuilder serializedInfo{ fbb };
		serializedInfo.add_ammo(ammosOffset);
		serializedInfo.add_has_lasersight(info->HasLasersight);
		serializedInfo.add_has_silencer(info->HasSilencer);
		serializedInfo.add_present(info->Present);
		serializedInfo.add_selected_ammo((int)info->SelectedAmmo);
		auto serializedInfoOffset = serializedInfo.Finish();

		carriedWeapons.push_back(serializedInfoOffset);
	}
	auto carriedWeaponsOffset = fbb.CreateVector(carriedWeapons);

	Save::LaraBuilder lara{ fbb };
	lara.add_air(Lara.Air);

	lara.add_burn_count(Lara.BurnCount);
	lara.add_burn_type((int)Lara.BurnType);
	lara.add_burn(Lara.Burn);
	lara.add_burn_blue(Lara.BurnBlue);
	lara.add_burn_smoke(Lara.BurnSmoke);
	lara.add_control(controlOffset);
	lara.add_next_corner_position(&nextCornerPos);
	lara.add_next_corner_rotation(&nextCornerRot);
	lara.add_extra_anim(Lara.ExtraAnim);
	lara.add_extra_head_rot(&extraHeadRot);
	lara.add_extra_torso_rot(&extraTorsoRot);
	lara.add_extra_velocity(&extraVelocity);
	lara.add_flare(flareOffset);
	lara.add_highest_location(Lara.HighestLocation);
	lara.add_hit_direction(Lara.HitDirection);
	lara.add_hit_frame(Lara.HitFrame);
	lara.add_interacted_item(Lara.InteractedItem);
	lara.add_inventory(inventoryOffset);
	lara.add_item_number(Lara.ItemNumber);
	lara.add_left_arm(leftArmOffset);
	lara.add_lit_torch(Lara.LitTorch);
	lara.add_location(Lara.Location);
	lara.add_location_pad(Lara.LocationPad);
	lara.add_mesh_ptrs(meshPtrsOffset);
	lara.add_poison_potency(Lara.PoisonPotency);
	lara.add_projected_floor_height(Lara.ProjectedFloorHeight);
	lara.add_right_arm(rightArmOffset);
	lara.add_spasm_effect_count(Lara.SpasmEffectCount);
	lara.add_sprint_energy(Lara.SprintEnergy);
	lara.add_target_facing_angle(Lara.TargetOrientation.GetY());
	lara.add_target_arm_angles(laraTargetAnglesOffset);
	lara.add_target_entity_number(Lara.TargetEntity - g_Level.Items.data());
	lara.add_vehicle(Lara.Vehicle);
	lara.add_water_current_active(Lara.WaterCurrentActive);
	lara.add_water_current_pull(&waterCurrentPull);
	lara.add_water_surface_dist(Lara.WaterSurfaceDist);
	lara.add_weapons(carriedWeaponsOffset);
	lara.add_wet(wetOffset);
	auto laraOffset = lara.Finish();

	for (auto& itemToSerialize : g_Level.Items) 
	{
		OBJECT_INFO* obj = &Objects[itemToSerialize.ObjectNumber];

		std::vector<int> itemFlags;
		for (int i = 0; i < 7; i++)
			itemFlags.push_back(itemToSerialize.ItemFlags[i]);
		auto itemFlagsOffset = fbb.CreateVector(itemFlags);
				
		flatbuffers::Offset<Save::Creature> creatureOffset;

		if (Objects[itemToSerialize.ObjectNumber].intelligent 
			&& itemToSerialize.Data.is<CreatureInfo>())
		{
			auto creature = GetCreatureInfo(&itemToSerialize);

			std::vector<int> jointRotations;
			for (int i = 0; i < 4; i++)
				jointRotations.push_back(creature->JointRotation[i]);
			auto jointRotationsOffset = fbb.CreateVector(jointRotations);

			Save::CreatureBuilder creatureBuilder{ fbb };

			creatureBuilder.add_alerted(creature->Alerted);
			creatureBuilder.add_can_jump(creature->LOT.CanJump);
			creatureBuilder.add_can_monkey(creature->LOT.CanMonkey);
			creatureBuilder.add_enemy(creature->Enemy - g_Level.Items.data());
			creatureBuilder.add_fired_weapon(creature->FiredWeapon);
			creatureBuilder.add_flags(creature->Flags);
			creatureBuilder.add_friendly(creature->Friendly);
			creatureBuilder.add_head_left(creature->HeadLeft);
			creatureBuilder.add_head_right(creature->HeadRight);
			creatureBuilder.add_hurt_by_lara(creature->HurtByLara);
			creatureBuilder.add_is_amphibious(creature->LOT.IsAmphibious);
			creatureBuilder.add_is_jumping(creature->LOT.IsJumping);
			creatureBuilder.add_is_monkeying(creature->LOT.IsMonkeying);
			creatureBuilder.add_joint_rotation(jointRotationsOffset);
			creatureBuilder.add_jump_ahead(creature->JumpAhead);
			creatureBuilder.add_location_ai(creature->LocationAI);
			creatureBuilder.add_maximum_turn(creature->MaxTurn);
			creatureBuilder.add_monkey_swing_ahead(creature->MonkeySwingAhead);
			creatureBuilder.add_mood((int)creature->Mood);
			creatureBuilder.add_patrol(creature->Patrol);
			creatureBuilder.add_poisoned(creature->Poisoned);
			creatureBuilder.add_reached_goal(creature->ReachedGoal);
			creatureBuilder.add_tosspad(creature->Tosspad);
			creatureOffset = creatureBuilder.Finish();
		} 

		Save::Position position = Save::Position(
			(int32_t)itemToSerialize.Pose.Position.x,
			(int32_t)itemToSerialize.Pose.Position.y,
			(int32_t)itemToSerialize.Pose.Position.z,
			(int32_t)itemToSerialize.Pose.Orientation.GetX(),
			(int32_t)itemToSerialize.Pose.Orientation.GetY(),
			(int32_t)itemToSerialize.Pose.Orientation.GetZ());

		Save::ItemBuilder serializedItem{ fbb };

		serializedItem.add_anim_number(itemToSerialize.Animation.AnimNumber - obj->animIndex);
		serializedItem.add_after_death(itemToSerialize.AfterDeath);
		serializedItem.add_box_number(itemToSerialize.BoxNumber);
		serializedItem.add_carried_item(itemToSerialize.CarriedItem);
		serializedItem.add_active_state(itemToSerialize.Animation.ActiveState);
		serializedItem.add_vertical_velocity(itemToSerialize.Animation.VerticalVelocity);
		serializedItem.add_flags(itemToSerialize.Flags);
		serializedItem.add_floor(itemToSerialize.Floor);
		serializedItem.add_frame_number(itemToSerialize.Animation.FrameNumber);
		serializedItem.add_target_state(itemToSerialize.Animation.TargetState);
		serializedItem.add_hit_points(itemToSerialize.HitPoints);
		serializedItem.add_item_flags(itemFlagsOffset);
		serializedItem.add_mesh_bits(itemToSerialize.MeshBits);
		serializedItem.add_object_id(itemToSerialize.ObjectNumber);
		serializedItem.add_position(&position);
		serializedItem.add_required_state(itemToSerialize.Animation.RequiredState);
		serializedItem.add_room_number(itemToSerialize.RoomNumber);
		serializedItem.add_velocity(itemToSerialize.Animation.Velocity);
		serializedItem.add_timer(itemToSerialize.Timer);
		serializedItem.add_touch_bits(itemToSerialize.TouchBits);
		serializedItem.add_trigger_flags(itemToSerialize.TriggerFlags);
		serializedItem.add_triggered((itemToSerialize.Flags & (TRIGGERED | CODE_BITS | ONESHOT)) != 0);
		serializedItem.add_active(itemToSerialize.Active);
		serializedItem.add_status(itemToSerialize.Status);
		serializedItem.add_airborne(itemToSerialize.Animation.Airborne);
		serializedItem.add_hit_stauts(itemToSerialize.HitStatus);
		serializedItem.add_ai_bits(itemToSerialize.AIBits);
		serializedItem.add_collidable(itemToSerialize.Collidable);
		serializedItem.add_looked_at(itemToSerialize.LookedAt);
		serializedItem.add_swap_mesh_flags(itemToSerialize.SwapMeshFlags);

		if (Objects[itemToSerialize.ObjectNumber].intelligent 
			&& itemToSerialize.Data.is<CreatureInfo>())
		{
			serializedItem.add_data_type(Save::ItemData::Creature);
			serializedItem.add_data(creatureOffset.Union());
		}
		else if (itemToSerialize.Data.is<short>())
		{
			short& data = itemToSerialize.Data;
			serializedItem.add_data_type(Save::ItemData::Short);
			serializedItem.add_data(data);
		}
		else if (itemToSerialize.Data.is<int>())
		{
			int& data = itemToSerialize.Data;
			serializedItem.add_data_type(Save::ItemData::Int);
			serializedItem.add_data(data);
		}

		auto serializedItemOffset = serializedItem.Finish();
		serializedItems.push_back(serializedItemOffset);
	}

	auto serializedItemsOffset = fbb.CreateVector(serializedItems);

	// Soundtrack playheads
	auto bgmTrackData = GetSoundTrackNameAndPosition(SOUNDTRACK_PLAYTYPE::BGM);
	auto oneshotTrackData = GetSoundTrackNameAndPosition(SOUNDTRACK_PLAYTYPE::OneShot);
	auto bgmTrackOffset = fbb.CreateString(bgmTrackData.first);
	auto oneshotTrackOffset = fbb.CreateString(oneshotTrackData.first);

	// Legacy soundtrack map
	std::vector<int> soundTrackMap;
	for (auto& track : SoundTracks) { soundTrackMap.push_back(track.Mask); }
	auto soundtrackMapOffset = fbb.CreateVector(soundTrackMap);

	// Flipmaps
	std::vector<int> flipMaps;
	for (int i = 0; i < MAX_FLIPMAP; i++)
		flipMaps.push_back(FlipMap[i] >> 8);
	auto flipMapsOffset = fbb.CreateVector(flipMaps);

	std::vector<int> flipStats;
	for (int i = 0; i < MAX_FLIPMAP; i++)
		flipStats.push_back(FlipStats[i]);
	auto flipStatsOffset = fbb.CreateVector(flipStats);

	// Cameras
	std::vector<flatbuffers::Offset<Save::FixedCamera>> cameras;
	for (int i = 0; i < g_Level.Cameras.size(); i++)
	{
		Save::FixedCameraBuilder camera{ fbb };
		camera.add_flags(g_Level.Cameras[i].flags);
		cameras.push_back(camera.Finish());
	}
	auto camerasOffset = fbb.CreateVector(cameras);

	// Sinks
	std::vector<flatbuffers::Offset<Save::Sink>> sinks;
	for (int i = 0; i < g_Level.Sinks.size(); i++)
	{
		Save::SinkBuilder sink{ fbb };
		sink.add_flags(g_Level.Sinks[i].strength);
		sinks.push_back(sink.Finish());
	}
	auto sinksOffset = fbb.CreateVector(sinks);

	// Flyby cameras
	std::vector<flatbuffers::Offset<Save::FlyByCamera>> flybyCameras;
	for (int i = 0; i < NumberSpotcams; i++)
	{
		Save::FlyByCameraBuilder flyby{ fbb };
		flyby.add_flags(SpotCam[i].flags);
		flybyCameras.push_back(flyby.Finish());
	}
	auto flybyCamerasOffset = fbb.CreateVector(flybyCameras);

	// Static meshes
	std::vector<flatbuffers::Offset<Save::StaticMeshInfo>> staticMeshes;
	for (int i = 0; i < g_Level.Rooms.size(); i++)
	{
		auto* room = &g_Level.Rooms[i];

		for (int j = 0; j < room->mesh.size(); j++)
		{
			Save::StaticMeshInfoBuilder staticMesh{ fbb };
			staticMesh.add_flags(room->mesh[j].flags);
			staticMesh.add_room_number(i);
			staticMeshes.push_back(staticMesh.Finish());
		}
	}
	auto staticMeshesOffset = fbb.CreateVector(staticMeshes);

	// Particle enemies
	std::vector<flatbuffers::Offset<Save::BatInfo>> bats;
	for (int i = 0; i < NUM_BATS; i++)
	{
		auto* bat = &Bats[i];

		Save::BatInfoBuilder batInfo{ fbb };

		batInfo.add_counter(bat->Counter);
		batInfo.add_on(bat->On);
		batInfo.add_room_number(bat->RoomNumber);
		batInfo.add_x(bat->Pose.Position.x);
		batInfo.add_y(bat->Pose.Position.y);
		batInfo.add_z(bat->Pose.Position.z);
		batInfo.add_x_rot(bat->Pose.Orientation.GetX());
		batInfo.add_y_rot(bat->Pose.Orientation.GetY());
		batInfo.add_z_rot(bat->Pose.Orientation.GetZ());

		bats.push_back(batInfo.Finish());
	}
	auto batsOffset = fbb.CreateVector(bats);

	std::vector<flatbuffers::Offset<Save::SpiderInfo>> spiders;
	for (int i = 0; i < NUM_SPIDERS; i++)
	{
		auto* spider = &Spiders[i];

		Save::SpiderInfoBuilder spiderInfo{ fbb };

		spiderInfo.add_flags(spider->Flags);
		spiderInfo.add_on(spider->On);
		spiderInfo.add_room_number(spider->RoomNumber);
		spiderInfo.add_x(spider->Pose.Position.x);
		spiderInfo.add_y(spider->Pose.Position.y);
		spiderInfo.add_z(spider->Pose.Position.z);
		spiderInfo.add_x_rot(spider->Pose.Orientation.GetX());
		spiderInfo.add_y_rot(spider->Pose.Orientation.GetY());
		spiderInfo.add_z_rot(spider->Pose.Orientation.GetZ());

		spiders.push_back(spiderInfo.Finish());
	}
	auto spidersOffset = fbb.CreateVector(spiders);

	std::vector<flatbuffers::Offset<Save::RatInfo>> rats;
	for (int i = 0; i < NUM_RATS; i++)
	{
		auto* rat = &Rats[i];

		Save::RatInfoBuilder ratInfo{ fbb };

		ratInfo.add_flags(rat->Flags);
		ratInfo.add_on(rat->On);
		ratInfo.add_room_number(rat->RoomNumber);
		ratInfo.add_x(rat->Pose.Position.x);
		ratInfo.add_y(rat->Pose.Position.y);
		ratInfo.add_z(rat->Pose.Position.z);
		ratInfo.add_x_rot(rat->Pose.Orientation.GetX());
		ratInfo.add_y_rot(rat->Pose.Orientation.GetY());
		ratInfo.add_z_rot(rat->Pose.Orientation.GetZ());

		rats.push_back(ratInfo.Finish());
	}
	auto ratsOffset = fbb.CreateVector(rats);

	std::vector<flatbuffers::Offset<Save::ScarabInfo>> scarabs;
	for (int i = 0; i < NUM_BATS; i++)
	{
		auto* beetle = &BeetleSwarm[i];

		Save::ScarabInfoBuilder scarabInfo{ fbb };

		scarabInfo.add_flags(beetle->Flags);
		scarabInfo.add_on(beetle->On);
		scarabInfo.add_room_number(beetle->RoomNumber);
		scarabInfo.add_x(beetle->Pose.Position.x);
		scarabInfo.add_y(beetle->Pose.Position.y);
		scarabInfo.add_z(beetle->Pose.Position.z);
		scarabInfo.add_x_rot(beetle->Pose.Orientation.GetX());
		scarabInfo.add_y_rot(beetle->Pose.Orientation.GetY());
		scarabInfo.add_z_rot(beetle->Pose.Orientation.GetZ());

		scarabs.push_back(scarabInfo.Finish());
	}
	auto scarabsOffset = fbb.CreateVector(scarabs);

	// Rope
	flatbuffers::Offset<Save::Rope> ropeOffset;
	flatbuffers::Offset<Save::Pendulum> pendulumOffset;
	flatbuffers::Offset<Save::Pendulum> alternatePendulumOffset;

	if (Lara.Control.Rope.Ptr != -1)
	{
		ROPE_STRUCT* rope = &Ropes[Lara.Control.Rope.Ptr];

		std::vector<const Save::Vector3*> segments;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			segments.push_back(&Save::Vector3(
				rope->segment[i].x, 
				rope->segment[i].y, 
				rope->segment[i].z));
		auto segmentsOffset = fbb.CreateVector(segments);

		std::vector<const Save::Vector3*> velocities;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			velocities.push_back(&Save::Vector3(
				rope->velocity[i].x,
				rope->velocity[i].y,
				rope->velocity[i].z));
		auto velocitiesOffset = fbb.CreateVector(velocities);

		std::vector<const Save::Vector3*> normalisedSegments;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			normalisedSegments.push_back(&Save::Vector3(
				rope->normalisedSegment[i].x,
				rope->normalisedSegment[i].y,
				rope->normalisedSegment[i].z));
		auto normalisedSegmentsOffset = fbb.CreateVector(normalisedSegments);

		std::vector<const Save::Vector3*> meshSegments;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			meshSegments.push_back(&Save::Vector3(
				rope->meshSegment[i].x,
				rope->meshSegment[i].y,
				rope->meshSegment[i].z));
		auto meshSegmentsOffset = fbb.CreateVector(meshSegments);

		std::vector<const Save::Vector3*> coords;
		for (int i = 0; i < ROPE_SEGMENTS; i++)
			coords.push_back(&Save::Vector3(
				rope->coords[i].x,
				rope->coords[i].y,
				rope->coords[i].z));
		auto coordsOffset = fbb.CreateVector(coords);

		Save::RopeBuilder ropeInfo{ fbb };

		ropeInfo.add_segments(segmentsOffset);
		ropeInfo.add_velocities(velocitiesOffset);
		ropeInfo.add_mesh_segments(meshSegmentsOffset);
		ropeInfo.add_normalised_segments(normalisedSegmentsOffset);
		ropeInfo.add_coords(coordsOffset);
		ropeInfo.add_coiled(rope->coiled);
		ropeInfo.add_position(&Save::Vector3(
			rope->position.x,
			rope->position.y,
			rope->position.z));
		ropeInfo.add_segment_length(rope->segmentLength);

		ropeOffset = ropeInfo.Finish();

		Save::PendulumBuilder pendulumInfo{ fbb };
		pendulumInfo.add_node(CurrentPendulum.node);
		pendulumInfo.add_position(&Save::Vector3(
			CurrentPendulum.position.x,
			CurrentPendulum.position.y,
			CurrentPendulum.position.z));
		pendulumInfo.add_velocity(&Save::Vector3(
			CurrentPendulum.velocity.x,
			CurrentPendulum.velocity.y,
			CurrentPendulum.velocity.z));
		pendulumOffset = pendulumInfo.Finish();

		Save::PendulumBuilder alternatePendulumInfo{ fbb };
		alternatePendulumInfo.add_node(AlternatePendulum.node);
		alternatePendulumInfo.add_position(&Save::Vector3(
			AlternatePendulum.position.x,
			AlternatePendulum.position.y,
			AlternatePendulum.position.z));
		alternatePendulumInfo.add_velocity(&Save::Vector3(
			AlternatePendulum.velocity.x,
			AlternatePendulum.velocity.y,
			AlternatePendulum.velocity.z));
		alternatePendulumOffset = alternatePendulumInfo.Finish();
	}

	Save::SaveGameBuilder sgb{ fbb };

	sgb.add_header(headerOffset);
	sgb.add_level(levelStatisticsOffset);
	sgb.add_game(gameStatisticsOffset);
	sgb.add_lara(laraOffset);
	sgb.add_items(serializedItemsOffset);
	sgb.add_ambient_track(bgmTrackOffset);
	sgb.add_ambient_position(bgmTrackData.second);
	sgb.add_oneshot_track(oneshotTrackOffset);
	sgb.add_oneshot_position(oneshotTrackData.second);
	sgb.add_cd_flags(soundtrackMapOffset);
	sgb.add_flip_maps(flipMapsOffset);
	sgb.add_flip_stats(flipStatsOffset);
	sgb.add_flip_effect(FlipEffect);
	sgb.add_flip_status(FlipStatus);
	sgb.add_flip_timer(0);
	sgb.add_static_meshes(staticMeshesOffset);
	sgb.add_fixed_cameras(camerasOffset);
	sgb.add_bats(batsOffset);
	sgb.add_rats(ratsOffset);
	sgb.add_spiders(spidersOffset);
	sgb.add_scarabs(scarabsOffset);
	sgb.add_sinks(sinksOffset);
	sgb.add_flyby_cameras(flybyCamerasOffset);

	if (Lara.Control.Rope.Ptr != -1)
	{
		sgb.add_rope(ropeOffset);
		sgb.add_pendulum(pendulumOffset);
		sgb.add_alternate_pendulum(alternatePendulumOffset);
	}

	auto sg = sgb.Finish();
	fbb.Finish(sg);

	auto bufferToSerialize = fbb.GetBufferPointer();
	auto bufferSize = fbb.GetSize();

	if (!std::filesystem::exists(SAVEGAME_PATH))
		std::filesystem::create_directory(SAVEGAME_PATH);

	std::ofstream fileOut{};
	fileOut.open(fileName, std::ios_base::binary | std::ios_base::out);
	fileOut.write((char*)bufferToSerialize, bufferSize);
	fileOut.close();

	return true;
}

bool SaveGame::Load(int slot)
{
	auto fileName = SAVEGAME_PATH + "savegame." + std::to_string(slot);

	std::ifstream file;
	file.open(fileName, std::ios_base::app | std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
	file.read(buffer.get(), length);
	file.close();

	const Save::SaveGame* s = Save::GetSaveGame(buffer.get());

	// Flipmaps
	for (int i = 0; i < s->flip_stats()->size(); i++)
	{
		if (s->flip_stats()->Get(i) != 0)
			DoFlipMap(i);

		FlipMap[i] = s->flip_maps()->Get(i) << 8;
	}

	// Effects
	FlipEffect = s->flip_effect();
	FlipStatus = s->flip_status();
	//FlipTimer = s->flip_timer();

	// Restore soundtracks
	PlaySoundTrack(s->ambient_track()->str(), SOUNDTRACK_PLAYTYPE::BGM, s->ambient_position());
	PlaySoundTrack(s->oneshot_track()->str(), SOUNDTRACK_PLAYTYPE::OneShot, s->oneshot_position());

	// Legacy soundtrack map
	for (int i = 0; i < s->cd_flags()->size(); i++)
	{
		// Safety check for cases when soundtrack map was externally modified and became smaller
		if (i >= SoundTracks.size())
			break;

		SoundTracks[i].Mask = s->cd_flags()->Get(i);
	}

	// Static objects
	for (int i = 0; i < s->static_meshes()->size(); i++)
	{
		auto staticMesh = s->static_meshes()->Get(i);
		auto room = &g_Level.Rooms[staticMesh->room_number()];
		if (i >= room->mesh.size())
			break;
		room->mesh[i].flags = staticMesh->flags();
		if (!room->mesh[i].flags)
		{
			short roomNumber = staticMesh->room_number();
			FLOOR_INFO* floor = GetFloor(room->mesh[i].pos.Position.x, room->mesh[i].pos.Position.y, room->mesh[i].pos.Position.z, &roomNumber);
			TestTriggers(room->mesh[i].pos.Position.x, room->mesh[i].pos.Position.y, room->mesh[i].pos.Position.z, staticMesh->room_number(), true, 0);
			floor->Stopper = false;
		}
	}

	// Cameras 
	for (int i = 0; i < s->fixed_cameras()->size(); i++)
	{
		if (i < g_Level.Cameras.size())
			g_Level.Cameras[i].flags = s->fixed_cameras()->Get(i)->flags();
	}

	// Sinks 
	for (int i = 0; i < s->sinks()->size(); i++)
	{
		if (i < g_Level.Sinks.size())
			g_Level.Sinks[i].strength = s->sinks()->Get(i)->flags();
	}

	// Flyby cameras 
	for (int i = 0; i < s->flyby_cameras()->size(); i++)
	{
		if (i < NumberSpotcams)
			SpotCam[i].flags = s->flyby_cameras()->Get(i)->flags();
	}

	ZeroMemory(&Lara, sizeof(LaraInfo));

	// Items
	for (int i = 0; i < s->items()->size(); i++)
	{
		const Save::Item* savedItem = s->items()->Get(i);

		short itemNumber = i;
		bool dynamicItem = false;

		if (i >= g_Level.NumItems)
		{
			// Items beyond items level space must be active
			if (!savedItem->active())
				continue;

			// Items beyond items level space must be initialised differently
			itemNumber = CreateItem();
			if (itemNumber == NO_ITEM)
				continue;
			dynamicItem = true;
		}

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[item->ObjectNumber];

		if (!dynamicItem)
		{
			// Kill immediately item if already killed and continue
			if (savedItem->flags() & IFLAG_KILLED)
			{
				if (obj->floor != nullptr)
					UpdateBridgeItem(itemNumber, true);

				KillItem(i);
				item->Status = ITEM_DEACTIVATED;
				item->Flags |= ONESHOT;
				continue;
			}

			// If not triggered, don't load remaining data
			if (item->ObjectNumber != ID_LARA && !(savedItem->flags() & (TRIGGERED | CODE_BITS | ONESHOT)))
				continue;
		}

		item->Pose.Position.x = savedItem->position()->x_pos();
		item->Pose.Position.y = savedItem->position()->y_pos();
		item->Pose.Position.z = savedItem->position()->z_pos();
		item->Pose.Orientation.SetX(savedItem->position()->x_rot());
		item->Pose.Orientation.SetY(savedItem->position()->y_rot());
		item->Pose.Orientation.SetZ(savedItem->position()->z_rot());

		short roomNumber = savedItem->room_number();

		if (dynamicItem)
		{
			item->RoomNumber = roomNumber;

			InitialiseItem(itemNumber);
			
			// InitialiseItem could overwrite position so restore it
			item->Pose.Position.x = savedItem->position()->x_pos();
			item->Pose.Position.y = savedItem->position()->y_pos();
			item->Pose.Position.z = savedItem->position()->z_pos();
			item->Pose.Orientation.SetX(savedItem->position()->x_rot());
			item->Pose.Orientation.SetY(savedItem->position()->y_rot());
			item->Pose.Orientation.SetZ(savedItem->position()->z_rot());
		}

		item->Animation.Velocity = savedItem->velocity();
		item->Animation.VerticalVelocity = savedItem->vertical_velocity();

		// Do the correct way for assigning new room number
		if (item->ObjectNumber == ID_LARA)
		{
			LaraItem->Location.roomNumber = roomNumber;
			LaraItem->Location.yNumber = item->Pose.Position.y;
			item->RoomNumber = roomNumber;
			Lara.ItemNumber = i;
			LaraItem = item;
			UpdateItemRoom(item, -LARA_HEIGHT / 2);
		}
		else
		{
			if (item->RoomNumber != roomNumber)
				ItemNewRoom(i, roomNumber);

			if (obj->shadowSize)
			{
				FLOOR_INFO* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
				item->Floor = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
			}
		}

		// Animations
		item->Animation.ActiveState = savedItem->active_state();
		item->Animation.RequiredState = savedItem->required_state();
		item->Animation.TargetState = savedItem->target_state();
		item->Animation.AnimNumber = obj->animIndex + savedItem->anim_number();
		item->Animation.FrameNumber = savedItem->frame_number();

		// Hit points
		item->HitPoints = savedItem->hit_points();

		// Flags and timers
		for (int j = 0; j < 7; j++)
			item->ItemFlags[j] = savedItem->item_flags()->Get(j);
		item->Timer = savedItem->timer();
		item->TriggerFlags = savedItem->trigger_flags();
		item->Flags = savedItem->flags();

		// Carried item
		item->CarriedItem = savedItem->carried_item();

		// Activate item if needed
		if (savedItem->active() && !item->Active)
			AddActiveItem(i);

		item->Active = savedItem->active();
		item->HitStatus = savedItem->hit_stauts();
		item->Status = savedItem->status();
		item->AIBits = savedItem->ai_bits();
		item->Animation.Airborne = savedItem->airborne();
		item->Collidable = savedItem->collidable();
		item->LookedAt = savedItem->looked_at();

		// Creature data for intelligent items
		if (item->ObjectNumber != ID_LARA && obj->intelligent)
		{
			EnableBaddieAI(i, true);

			auto creature = GetCreatureInfo(item);
			auto data = savedItem->data();
			auto savedCreature = (Save::Creature*)data;

			if (savedCreature == nullptr)
				continue;

			creature->Alerted = savedCreature->alerted();
			creature->LOT.CanJump = savedCreature->can_jump();
			creature->LOT.CanMonkey = savedCreature->can_monkey();
			if (savedCreature->enemy() >= 0)
				creature->Enemy = &g_Level.Items[savedCreature->enemy()];
			creature->FiredWeapon = savedCreature->fired_weapon();
			creature->Flags = savedCreature->flags();
			creature->Friendly = savedCreature->friendly();
			creature->HeadLeft = savedCreature->head_left();
			creature->HeadRight = savedCreature->head_right();
			creature->HurtByLara = savedCreature->hurt_by_lara();
			creature->LocationAI = savedCreature->location_ai();
			creature->LOT.IsAmphibious = savedCreature->is_amphibious();
			creature->LOT.IsJumping = savedCreature->is_jumping();
			creature->LOT.IsMonkeying = savedCreature->is_monkeying();
			for (int j = 0; j < 4; j++)
				creature->JointRotation[j] = savedCreature->joint_rotation()->Get(j);
			creature->JumpAhead = savedCreature->jump_ahead();
			creature->MaxTurn = savedCreature->maximum_turn();
			creature->MonkeySwingAhead = savedCreature->monkey_swing_ahead();
			creature->Mood = (MoodType)savedCreature->mood();
			creature->Patrol = savedCreature->patrol();
			creature->Poisoned = savedCreature->poisoned();
			creature->ReachedGoal = savedCreature->reached_goal();
			creature->Tosspad = savedCreature->tosspad();
		}
		else if (savedItem->data_type() == Save::ItemData::Short)
		{
			auto data = savedItem->data();
			auto savedData = (Save::Short*)data;
			item->Data = savedData->scalar();
		}

		// Mesh stuff
		item->MeshBits = savedItem->mesh_bits();
		item->SwapMeshFlags = savedItem->swap_mesh_flags();

		// Now some post-load specific hacks for objects
		if (item->ObjectNumber >= ID_PUZZLE_HOLE1 
			&& item->ObjectNumber <= ID_PUZZLE_HOLE16 
			&& (item->Status == ITEM_ACTIVE
				|| item->Status == ITEM_DEACTIVATED))
		{
			item->ObjectNumber = (GAME_OBJECT_ID)((int)item->ObjectNumber + ID_PUZZLE_DONE1 - ID_PUZZLE_HOLE1);
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + savedItem->anim_number();
		}

		if ((item->ObjectNumber >= ID_SMASH_OBJECT1)
			&& (item->ObjectNumber <= ID_SMASH_OBJECT8)
			&& (item->Flags & ONESHOT))
			item->MeshBits = 0x00100;

		if (obj->floor != nullptr)
			UpdateBridgeItem(itemNumber);
	}

	for (int i = 0; i < s->bats()->size(); i++)
	{
		auto batInfo = s->bats()->Get(i);
		auto* bat = &Bats[i];

		bat->On = batInfo->on();
		bat->Counter = batInfo->counter();
		bat->RoomNumber = batInfo->room_number();
		bat->Pose.Position.x = batInfo->x();
		bat->Pose.Position.y = batInfo->y();
		bat->Pose.Position.z = batInfo->z();
		bat->Pose.Orientation.SetX(batInfo->x_rot());
		bat->Pose.Orientation.SetY(batInfo->y_rot());
		bat->Pose.Orientation.SetZ(batInfo->z_rot());
	}

	for (int i = 0; i < s->rats()->size(); i++)
	{
		auto ratInfo = s->rats()->Get(i);
		auto* rat = &Rats[i];

		rat->On = ratInfo->on();
		rat->Flags = ratInfo->flags();
		rat->RoomNumber = ratInfo->room_number();
		rat->Pose.Position.x = ratInfo->x();
		rat->Pose.Position.y = ratInfo->y();
		rat->Pose.Position.z = ratInfo->z();
		rat->Pose.Orientation.SetX(ratInfo->x_rot());
		rat->Pose.Orientation.SetY(ratInfo->y_rot());
		rat->Pose.Orientation.SetZ(ratInfo->z_rot());
	}

	for (int i = 0; i < s->spiders()->size(); i++)
	{
		auto spiderInfo = s->spiders()->Get(i);
		auto* spider = &Spiders[i];

		spider->On = spiderInfo->on();
		spider->Flags = spiderInfo->flags();
		spider->RoomNumber = spiderInfo->room_number();
		spider->Pose.Position.x = spiderInfo->x();
		spider->Pose.Position.y = spiderInfo->y();
		spider->Pose.Position.z = spiderInfo->z();
		spider->Pose.Orientation.SetX(spiderInfo->x_rot());
		spider->Pose.Orientation.SetY(spiderInfo->y_rot());
		spider->Pose.Orientation.SetZ(spiderInfo->z_rot());
	}

	for (int i = 0; i < s->scarabs()->size(); i++)
	{
		auto beetleInfo = s->scarabs()->Get(i);
		auto* Beetle = &BeetleSwarm[i];

		Beetle->On = beetleInfo->on();
		Beetle->Flags = beetleInfo->flags();
		Beetle->RoomNumber = beetleInfo->room_number();
		Beetle->Pose.Position.x = beetleInfo->x();
		Beetle->Pose.Position.y = beetleInfo->y();
		Beetle->Pose.Position.z = beetleInfo->z();
		Beetle->Pose.Orientation.SetX(beetleInfo->x_rot());
		Beetle->Pose.Orientation.SetY(beetleInfo->y_rot());
		Beetle->Pose.Orientation.SetZ(beetleInfo->z_rot());
	}

	JustLoaded = 1;	

	// Lara
	ZeroMemory(Lara.Inventory.Puzzles, NUM_PUZZLES * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->puzzles()->size(); i++)
	{
		Lara.Inventory.Puzzles[i] = s->lara()->inventory()->puzzles()->Get(i);
	}

	ZeroMemory(Lara.Inventory.PuzzlesCombo, NUM_PUZZLES * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->puzzles_combo()->size(); i++)
	{
		Lara.Inventory.PuzzlesCombo[i] = s->lara()->inventory()->puzzles_combo()->Get(i);
	}

	ZeroMemory(Lara.Inventory.Keys, NUM_KEYS * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->keys()->size(); i++)
	{
		Lara.Inventory.Keys[i] = s->lara()->inventory()->keys()->Get(i);
	}

	ZeroMemory(Lara.Inventory.KeysCombo, NUM_KEYS * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->keys_combo()->size(); i++)
	{
		Lara.Inventory.KeysCombo[i] = s->lara()->inventory()->keys_combo()->Get(i);
	}

	ZeroMemory(Lara.Inventory.Pickups, NUM_PICKUPS * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->pickups()->size(); i++)
	{
		Lara.Inventory.Pickups[i] = s->lara()->inventory()->pickups()->Get(i);
	}

	ZeroMemory(Lara.Inventory.PickupsCombo, NUM_PICKUPS * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->pickups_combo()->size(); i++)
	{
		Lara.Inventory.Pickups[i] = s->lara()->inventory()->pickups_combo()->Get(i);
	}

	ZeroMemory(Lara.Inventory.Examines, NUM_EXAMINES * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->examines()->size(); i++)
	{
		Lara.Inventory.Examines[i] = s->lara()->inventory()->examines()->Get(i);
	}

	ZeroMemory(Lara.Inventory.ExaminesCombo, NUM_EXAMINES * 2 * sizeof(int));
	for (int i = 0; i < s->lara()->inventory()->examines_combo()->size(); i++)
	{
		Lara.Inventory.ExaminesCombo[i] = s->lara()->inventory()->examines_combo()->Get(i);
	}

	for (int i = 0; i < s->lara()->mesh_ptrs()->size(); i++)
	{
		Lara.MeshPtrs[i] = s->lara()->mesh_ptrs()->Get(i);
	}

	for (int i = 0; i < 15; i++)
	{
		Lara.Wet[i] = s->lara()->wet()->Get(i);
	}

	Lara.Air = s->lara()->air();
	Lara.BurnCount = s->lara()->burn_count();
	Lara.BurnType = (BurnType)s->lara()->burn_type();
	Lara.Burn = s->lara()->burn();
	Lara.BurnBlue = s->lara()->burn_blue();
	Lara.BurnSmoke = s->lara()->burn_smoke();
	Lara.Control.CalculatedJumpVelocity = s->lara()->control()->calculated_jump_velocity();
	Lara.Control.CanMonkeySwing = s->lara()->control()->can_monkey_swing();
	Lara.Control.CanClimbLadder = s->lara()->control()->is_climbing_ladder();
	Lara.Control.Count.Death = s->lara()->control()->count()->death();
	Lara.Control.Count.NoCheat = s->lara()->control()->count()->no_cheat();
	Lara.Control.Count.Pose = s->lara()->control()->count()->pose();
	Lara.Control.Count.PositionAdjust = s->lara()->control()->count()->position_adjust();
	Lara.Control.Count.RunJump = s->lara()->control()->count()->run_jump();
	Lara.Control.Count.Death = s->lara()->control()->count()->death();
	Lara.Control.IsClimbingLadder = s->lara()->control()->is_climbing_ladder();
	Lara.Control.IsLow = s->lara()->control()->is_low();
	Lara.Control.IsMoving = s->lara()->control()->is_moving();
	Lara.Control.JumpDirection = (JumpDirection)s->lara()->control()->jump_direction();
	Lara.Control.KeepLow = s->lara()->control()->keep_low();
	Lara.Control.CanLook = s->lara()->control()->can_look();
	Lara.Control.MoveAngle = s->lara()->control()->move_angle();
	Lara.Control.RunJumpQueued = s->lara()->control()->run_jump_queued();
	Lara.Control.TurnRate = s->lara()->control()->turn_rate();
	Lara.Control.Locked = s->lara()->control()->locked();
	Lara.Control.HandStatus = (HandStatus)s->lara()->control()->hand_status();
	Lara.Control.Weapon.GunType = (LaraWeaponType)s->lara()->control()->weapon()->gun_type();
	Lara.Control.Weapon.HasFired = s->lara()->control()->weapon()->has_fired();
	Lara.Control.Weapon.Fired = s->lara()->control()->weapon()->fired();
	Lara.Control.Weapon.LastGunType = (LaraWeaponType)s->lara()->control()->weapon()->last_gun_type();
	Lara.Control.Weapon.RequestGunType = (LaraWeaponType)s->lara()->control()->weapon()->request_gun_type();
	Lara.Control.Weapon.WeaponItem = s->lara()->control()->weapon()->weapon_item();
	Lara.Control.Weapon.HolsterInfo.BackHolster = (HolsterSlot)s->lara()->control()->weapon()->holster_info()->back_holster();
	Lara.Control.Weapon.HolsterInfo.LeftHolster = (HolsterSlot)s->lara()->control()->weapon()->holster_info()->left_holster();
	Lara.Control.Weapon.HolsterInfo.RightHolster = (HolsterSlot)s->lara()->control()->weapon()->holster_info()->right_holster();
	Lara.Control.Weapon.UziLeft = s->lara()->control()->weapon()->uzi_left();
	Lara.Control.Weapon.UziRight = s->lara()->control()->weapon()->uzi_right();
	Lara.ExtraAnim = s->lara()->extra_anim();
	Lara.ExtraHeadRot.SetX( s->lara()->extra_head_rot()->x());
	Lara.ExtraHeadRot.SetY(s->lara()->extra_head_rot()->y());
	Lara.ExtraHeadRot.SetZ(s->lara()->extra_head_rot()->z());
	Lara.ExtraTorsoRot.SetZ(s->lara()->extra_torso_rot()->x());
	Lara.ExtraTorsoRot.SetY(s->lara()->extra_torso_rot()->y());
	Lara.ExtraTorsoRot.SetZ(s->lara()->extra_torso_rot()->z());
	Lara.ExtraVelocity.x = s->lara()->extra_velocity()->x();
	Lara.ExtraVelocity.y = s->lara()->extra_velocity()->y();
	Lara.ExtraVelocity.z = s->lara()->extra_velocity()->z();
	Lara.WaterCurrentActive = s->lara()->water_current_active();
	Lara.WaterCurrentPull.x = s->lara()->water_current_pull()->x();
	Lara.WaterCurrentPull.y = s->lara()->water_current_pull()->y();
	Lara.WaterCurrentPull.z = s->lara()->water_current_pull()->z();
	Lara.Flare.Life = s->lara()->flare()->life();
	Lara.Flare.ControlLeft = s->lara()->flare()->control_left();
	Lara.Flare.Frame = s->lara()->flare()->frame();
	Lara.HighestLocation = s->lara()->highest_location();
	Lara.HitDirection = s->lara()->hit_direction();
	Lara.HitFrame = s->lara()->hit_frame();
	Lara.InteractedItem = s->lara()->interacted_item();
	Lara.Inventory.BeetleComponents = s->lara()->inventory()->beetle_components();
	Lara.Inventory.BeetleLife = s->lara()->inventory()->beetle_life();
	Lara.Inventory.BigWaterskin = s->lara()->inventory()->big_waterskin();
	Lara.Inventory.HasBinoculars = s->lara()->inventory()->has_binoculars();
	Lara.Inventory.HasCrowbar = s->lara()->inventory()->has_crowbar();
	Lara.Inventory.HasLasersight = s->lara()->inventory()->has_lasersight();
	Lara.Inventory.HasSilencer = s->lara()->inventory()->has_silencer();
	Lara.Inventory.HasTorch = s->lara()->inventory()->has_torch();
	Lara.Inventory.IsBusy = s->lara()->inventory()->is_busy();
	Lara.Inventory.OldBusy = s->lara()->inventory()->old_busy();
	Lara.Inventory.SmallWaterskin = s->lara()->inventory()->small_waterskin();
	Lara.Inventory.TotalFlares = s->lara()->inventory()->total_flares();
	Lara.Inventory.TotalLargeMedipacks = s->lara()->inventory()->total_large_medipacks();
	Lara.Inventory.TotalSecrets = s->lara()->inventory()->total_secrets();
	Lara.Inventory.TotalSmallMedipacks = s->lara()->inventory()->total_small_medipacks();
	Lara.ItemNumber = s->lara()->item_number();
	Lara.LeftArm.AnimNumber = s->lara()->left_arm()->anim_number();
	Lara.LeftArm.FlashGun = s->lara()->left_arm()->flash_gun();
	Lara.LeftArm.FrameBase = s->lara()->left_arm()->frame_base();
	Lara.LeftArm.FrameNumber = s->lara()->left_arm()->frame_number();
	Lara.LeftArm.Locked = s->lara()->left_arm()->locked();
	Lara.LeftArm.Orientation.SetX(s->lara()->left_arm()->rotation()->x());
	Lara.LeftArm.Orientation.SetY(s->lara()->left_arm()->rotation()->y());
	Lara.LeftArm.Orientation.SetZ(s->lara()->left_arm()->rotation()->z());
	Lara.LitTorch = s->lara()->lit_torch();
	Lara.Location = s->lara()->location();
	Lara.LocationPad = s->lara()->location_pad();
	Lara.NextCornerPos = PoseData(
		s->lara()->next_corner_position()->x(),
		s->lara()->next_corner_position()->y(),
		s->lara()->next_corner_position()->z(),
		s->lara()->next_corner_rotation()->x(),
		s->lara()->next_corner_rotation()->y(),
		s->lara()->next_corner_rotation()->z());
	Lara.PoisonPotency = s->lara()->poison_potency();
	Lara.ProjectedFloorHeight = s->lara()->projected_floor_height();
	Lara.RightArm.AnimNumber = s->lara()->right_arm()->anim_number();
	Lara.RightArm.FlashGun = s->lara()->right_arm()->flash_gun();
	Lara.RightArm.FrameBase = s->lara()->right_arm()->frame_base();
	Lara.RightArm.FrameNumber = s->lara()->right_arm()->frame_number();
	Lara.RightArm.Locked = s->lara()->right_arm()->locked();
	Lara.RightArm.Orientation.SetX(s->lara()->right_arm()->rotation()->x());
	Lara.RightArm.Orientation.SetY(s->lara()->right_arm()->rotation()->y());
	Lara.RightArm.Orientation.SetZ(s->lara()->right_arm()->rotation()->z());
	Lara.Control.Minecart.Left = s->lara()->control()->minecart()->left();
	Lara.Control.Minecart.Right = s->lara()->control()->minecart()->right();
	Lara.Control.Rope.Segment = s->lara()->control()->rope()->segment();
	Lara.Control.Rope.Direction = s->lara()->control()->rope()->direction();
	Lara.Control.Rope.ArcFront = s->lara()->control()->rope()->arc_front();
	Lara.Control.Rope.ArcBack = s->lara()->control()->rope()->arc_back();
	Lara.Control.Rope.LastX = s->lara()->control()->rope()->last_x();
	Lara.Control.Rope.MaxXForward = s->lara()->control()->rope()->max_x_forward();
	Lara.Control.Rope.MaxXBackward = s->lara()->control()->rope()->max_x_backward();
	Lara.Control.Rope.DFrame = s->lara()->control()->rope()->dframe();
	Lara.Control.Rope.Frame = s->lara()->control()->rope()->frame();
	Lara.Control.Rope.FrameRate = s->lara()->control()->rope()->frame_rate();
	Lara.Control.Rope.Y = s->lara()->control()->rope()->y();
	Lara.Control.Rope.Ptr = s->lara()->control()->rope()->ptr();
	Lara.Control.Rope.Offset = s->lara()->control()->rope()->offset();
	Lara.Control.Rope.DownVel = s->lara()->control()->rope()->down_vel();
	Lara.Control.Rope.Flag = s->lara()->control()->rope()->flag();
	Lara.Control.Rope.Count = s->lara()->control()->rope()->count();
	Lara.Control.Subsuit.XRot = s->lara()->control()->subsuit()->x_rot();
	Lara.Control.Subsuit.DXRot = s->lara()->control()->subsuit()->d_x_rot();
	Lara.Control.Subsuit.Velocity[0] = s->lara()->control()->subsuit()->velocity()->Get(0);
	Lara.Control.Subsuit.Velocity[1] = s->lara()->control()->subsuit()->velocity()->Get(1);
	Lara.Control.Subsuit.VerticalVelocity = s->lara()->control()->subsuit()->vertical_velocity();
	Lara.Control.Subsuit.XRotVel = s->lara()->control()->subsuit()->x_rot_vel();
	Lara.Control.Subsuit.HitCount = s->lara()->control()->subsuit()->hit_count();
	Lara.Control.Tightrope.Balance = s->lara()->control()->tightrope()->balance();
	Lara.Control.Tightrope.CanDismount = s->lara()->control()->tightrope()->can_dismount();
	Lara.Control.Tightrope.TightropeItem = s->lara()->control()->tightrope()->tightrope_item();
	Lara.Control.Tightrope.TimeOnTightrope = s->lara()->control()->tightrope()->time_on_tightrope();
	Lara.Control.WaterStatus = (WaterStatus)s->lara()->control()->water_status();
	Lara.SpasmEffectCount = s->lara()->spasm_effect_count();
	Lara.SprintEnergy = s->lara()->sprint_energy();
	Lara.TargetEntity = (s->lara()->target_entity_number() >= 0 ? &g_Level.Items[s->lara()->target_entity_number()] : nullptr);
	Lara.TargetArmOrient.SetY(s->lara()->target_arm_angles()->Get(0));
	Lara.TargetArmOrient.SetX(s->lara()->target_arm_angles()->Get(1));
	Lara.TargetOrientation.SetY(s->lara()->target_facing_angle());
	Lara.Vehicle = s->lara()->vehicle();
	Lara.WaterSurfaceDist = s->lara()->water_surface_dist();

	for (int i = 0; i < s->lara()->weapons()->size(); i++)
	{
		auto* info = s->lara()->weapons()->Get(i);

		for (int j = 0; j < info->ammo()->size(); j++)
		{
			Lara.Weapons[i].Ammo[j].setInfinite(info->ammo()->Get(j)->is_infinite());
			Lara.Weapons[i].Ammo[j] = info->ammo()->Get(j)->count();
		}

		Lara.Weapons[i].HasLasersight = info->has_lasersight();
		Lara.Weapons[i].HasSilencer = info->has_silencer();
		Lara.Weapons[i].Present = info->present();
		Lara.Weapons[i].SelectedAmmo = (WeaponAmmoType)info->selected_ammo();
	}

	if (Lara.BurnType != BurnType::None)
	{
		char flag = 0;
		Lara.BurnType = BurnType::None;
		if (Lara.BurnSmoke)
		{
			flag = 1;
			Lara.BurnSmoke = 0;
		}

		LaraBurn(LaraItem);

		if (flag)
			Lara.BurnSmoke = 1;
	}

	// Rope
	if (Lara.Control.Rope.Ptr >= 0)
	{
		ROPE_STRUCT* rope = &Ropes[Lara.Control.Rope.Ptr];
		
		for (int i = 0; i < ROPE_SEGMENTS; i++)
		{
			rope->segment[i] = Vector3Int(
				s->rope()->segments()->Get(i)->x(),
				s->rope()->segments()->Get(i)->y(),
				s->rope()->segments()->Get(i)->z());

			rope->normalisedSegment[i] = Vector3Int(
				s->rope()->normalised_segments()->Get(i)->x(),
				s->rope()->normalised_segments()->Get(i)->y(),
				s->rope()->normalised_segments()->Get(i)->z());

			rope->meshSegment[i] = Vector3Int(
				s->rope()->mesh_segments()->Get(i)->x(),
				s->rope()->mesh_segments()->Get(i)->y(),
				s->rope()->mesh_segments()->Get(i)->z());

			rope->coords[i] = Vector3Int(
				s->rope()->coords()->Get(i)->x(),
				s->rope()->coords()->Get(i)->y(),
				s->rope()->coords()->Get(i)->z());

			rope->velocity[i] = Vector3Int(
				s->rope()->velocities()->Get(i)->x(),
				s->rope()->velocities()->Get(i)->y(),
				s->rope()->velocities()->Get(i)->z());
		}

		rope->coiled = s->rope()->coiled();
		rope->active = s->rope()->active();
		rope->position = Vector3Int(
			s->rope()->position()->x(),
			s->rope()->position()->y(),
			s->rope()->position()->z());

		CurrentPendulum.position = Vector3Int(
			s->pendulum()->position()->x(),
			s->pendulum()->position()->y(),
			s->pendulum()->position()->z());

		CurrentPendulum.velocity = Vector3Int(
			s->pendulum()->velocity()->x(),
			s->pendulum()->velocity()->y(),
			s->pendulum()->velocity()->z());

		CurrentPendulum.node = s->pendulum()->node();
		CurrentPendulum.rope = rope;

		AlternatePendulum.position = Vector3Int(
			s->alternate_pendulum()->position()->x(),
			s->alternate_pendulum()->position()->y(),
			s->alternate_pendulum()->position()->z());

		AlternatePendulum.velocity = Vector3Int(
			s->alternate_pendulum()->velocity()->x(),
			s->alternate_pendulum()->velocity()->y(),
			s->alternate_pendulum()->velocity()->z());

		AlternatePendulum.node = s->alternate_pendulum()->node();
		AlternatePendulum.rope = rope;
	}

	return true;
}

bool SaveGame::LoadHeader(int slot, SaveGameHeader* header)
{
	auto fileName = SAVEGAME_PATH + "savegame." + std::to_string(slot);

	std::ifstream file;
	file.open(fileName, std::ios_base::app | std::ios_base::binary);
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(length);
	file.read(buffer.get(), length);
	file.close();

	const Save::SaveGame* s = Save::GetSaveGame(buffer.get());

	header->Level = s->header()->level();
	header->LevelName = s->header()->level_name()->str();
	header->Days = s->header()->days();
	header->Hours = s->header()->hours();
	header->Minutes = s->header()->minutes();
	header->Seconds = s->header()->seconds();
	header->Level = s->header()->level();
	header->Timer = s->header()->timer();
	header->Count = s->header()->count();

	return true;
}