#pragma once

#define TR5_DEBUG // when defined, use the old GAME_OBJECT_ID enum to debug with TR5 level

#ifdef TR5_DEBUG
typedef enum GAME_OBJECT_ID
{
	ID_LARA,
	ID_PISTOLS_ANIM,
	ID_UZI_ANIM,
	ID_SHOTGUN_ANIM,
	ID_CROSSBOW_ANIM,
	ID_HK_ANIM,
	ID_REVOLVER_ANIM,
	ID_LARA_FLARE_ANIM,
	ID_LARA_SKIN,
	ID_LARA_SKIN_JOINTS,
	ID_LARA_SCREAM,
	ID_LARA_CROSSBOW_LASER,
	ID_LARA_REVOLVER_LASER,
	ID_LARA_HOLSTERS,
	ID_LARA_HOLSTERS_PISTOLS,
	ID_LARA_HOLSTERS_UZIS,
	ID_LARA_HOLSTERS_REVOLVER,
	ID_LARA_SPEECH_HEAD1,
	ID_LARA_SPEECH_HEAD2,
	ID_LARA_SPEECH_HEAD3,
	ID_LARA_SPEECH_HEAD4,
	ID_ACTOR1_SPEECH_HEAD1,
	ID_ACTOR1_SPEECH_HEAD2,
	ID_ACTOR2_SPEECH_HEAD1,
	ID_ACTOR2_SPEECH_HEAD2,
	ID_LARA_EXTRA_MESH1,
	ID_LARA_EXTRA_MESH2,
	ID_LARA_EXTRA_MESH3,
	ID_LARA_CROWBAR_ANIM,
	ID_LARA_TORCH_ANIM,
	ID_LARA_HAIR,
	ID_SAS,
	ID_JEEP,   // TR4 - 95% Working
	ID_SWAT,
	ID_MOTORBIKE,   // TR4
	ID_SWAT_PLUS,
	ID_JEEP_LARA_ANIMS,   // TR4 - OK
	ID_BLUE_GUARD,
	ID_LARA_MOTORBIKE_ANIMS,   // TR4 - OK
	ID_TWOGUN,
	ID_SKELETON, // TR4 - OK 
	ID_DOG,
	ID_CROCODILE, // TR4 - OK
	ID_LARA_EXTRA_ANIMS, // TR1-2-3 - OK
	ID_MUMMY, // TR4 - OK
	ID_TR5_LARSON,
	ID_SMALL_SCORPION, // TR4 - OK
	ID_TR5_PIERRE,
	ID_BAT, // TR4 - OK - (Problem of pathfinding)
	ID_MAFIA,
	ID_GUIDE, // TR4 - OK - Needs testing (need custom level)
	ID_MAFIA2,
	ID_SPHINX, // TR4 - OK - Needs testing
	ID_SAILOR,
	ID_HORSEMAN, // TR4
	ID_CRANE_GUY,
	ID_HORSE, // TR4
	ID_LION,
	ID_WILD_BOAR, // TR4 - OK
	ID_GLADIATOR,
	ID_HARPY, // TR4 - OK
	ID_ROMAN_GOD,
	ID_DEMIGOD1, // TR4 - OK
	ID_HYDRA,
	ID_DEMIGOD2, // TR4 - OK 
	ID_GUARDIAN,
	ID_DEMIGOD3, // TR4 - OK
	ID_HITMAN,
	ID_LITTLE_BEETLE, // TR4
	ID_SCIENTIST,
	ID_MERCENARY, // TR3
	ID_WILLOWISP, // REPLACE?
	ID_SHARK, // TR2 - OK
	ID_INVISIBLE_GHOST, // Andy2 invisible ghost TO REPLACE?
	ID_BADDY1, // TR4 - OK (implement picking, correct special moves)
	ID_REAPER,
	ID_BADDY2, //TR4 - OK
	ID_BROWN_BEAST, // Andy2 brown beast TO REPLACE?
	ID_CHAIN, // TR4 - OK
	ID_GREEN_TEETH, // LAGOON WITCH TO REPLACE?
	ID_PLOUGH, // TR4 - OK
	ID_ATTACK_SUB,
	ID_STARGATE, // TR4 - OK
	ID_SNIPER,
	ID_BURNING_FLOOR, // TR4
	ID_HUSKIE,
	ID_COG, // TR4 - OK
	ID_SAS_CAIRO, // TR4 - OK
	ID_SPIKEBALL, // TR4 - OK
	ID_IMP,
	ID_FLOOR_4BLADES, // TR4 - OK
	ID_GUNSHIP,
	ID_CEILING_4BLADES, // TR4 - OK
	ID_BATS,
	ID_RATS,
	ID_SPIDER,
	ID_FLAMETHROWER_BADDY, // TR3 - 50 - OK needs testing
	ID_AUTOGUN,
	ID_ELECTRICAL_CABLES,
	ID_DARTS,
	ID_DART_EMITTER,
	ID_HOMING_DART_EMITTER,
	ID_FALLING_CEILING,
	ID_FALLING_BLOCK,
	ID_FALLING_BLOCK2,
	ID_CRUMBLING_FLOOR,
	ID_TRAPDOOR1,
	ID_TRAPDOOR2,
	ID_TRAPDOOR3,
	ID_FLOOR_TRAPDOOR1,
	ID_FLOOR_TRAPDOOR2,
	ID_CEILING_TRAPDOOR1,
	ID_CEILING_TRAPDOOR2,
	ID_SCALING_TRAPDOOR,
	ID_ROLLINGBALL,
	ID_ROLLINGBARREL,
	ID_SPIKY_CEILING, // TR3 - OK
	ID_TEETH_SPIKES,
	ID_ROME_HAMMER,
	ID_HAMMER2,
	ID_FLAME,
	ID_FLAME_EMITTER,
	ID_FLAME_EMITTER2,
	ID_FLAME_EMITTER3,
	ID_TO_REPLACE_2, // TR3
	ID_BURNING_ROOTS,
	ID_ROPE,
	ID_FIREROPE,
	ID_POLEROPE,
	ID_PROPELLER_H, // TO REPLACE?
	ID_PROPELLER_V, // TO REPLACE?
	ID_ENEMY_JEEP, // TR4
	ID_ONEBLOCK_PLATFORM,
	ID_TWOBLOCK_PLATFORM,
	ID_RAISING_BLOCK1,
	ID_RAISING_BLOCK2,
	ID_HEADSET_TALKING_POINT,
	ID_PUSHABLE_OBJECT1,
	ID_PUSHABLE_OBJECT2,
	ID_PUSHABLE_OBJECT3,
	ID_PUSHABLE_OBJECT4,
	ID_PUSHABLE_OBJECT5,
	ID_WRECKING_BALL,
	ID_ZIPLINE_HANDLE, // Already in TR5 - to test
	ID_TORPEDO,
	ID_CHAFF,
	ID_SNOWMOBILE, // TR2
	ID_ELECTRIC_FENCE,
	ID_LIFT,
	ID_SNOWMOBILE_LARA_ANIMS, // TR2 - OK
	ID_YETI, // TR2 - 45
	ID_QUAD, // TR3 - OK
	ID_QUAD_LARA_ANIMS, // TR3 - OK
	ID_COMPY, // TR3 - 34
	ID_TIGHT_ROPE,
	ID_PARALLEL_BARS,
	ID_XRAY_CONTROLLER,
	ID_RAT, // TR2 - OK
	ID_PORTAL,
	ID_GEN_SLOT1,
	ID_GEN_SLOT2,
	ID_GEN_SLOT3,
	ID_GEN_SLOT4,
	ID_IMP_ROCK,
	ID_SEARCH_OBJECT1,
	ID_CATWALK_BLADE, // TR4 - OK
	ID_SEARCH_OBJECT2,
	ID_PLINTH_BLADE, // TR4 - OK
	ID_SEARCH_OBJECT3,
	ID_SETH_BLADE, // TR4 - OK
	ID_SEARCH_OBJECT4,
	ID_SLICER_DICER, // TR4 - OK - Needs testing
	ID_PUZZLE_ITEM1,
	ID_PUZZLE_ITEM2,
	ID_PUZZLE_ITEM3,
	ID_PUZZLE_ITEM4,
	ID_PUZZLE_ITEM5,
	ID_PUZZLE_ITEM6,
	ID_PUZZLE_ITEM7,
	ID_PUZZLE_ITEM8,
	ID_PUZZLE_ITEM1_COMBO1,
	ID_PUZZLE_ITEM1_COMBO2,
	ID_PUZZLE_ITEM2_COMBO1,
	ID_PUZZLE_ITEM2_COMBO2,
	ID_PUZZLE_ITEM3_COMBO1,
	ID_PUZZLE_ITEM3_COMBO2,
	ID_PUZZLE_ITEM4_COMBO1,
	ID_PUZZLE_ITEM4_COMBO2,
	ID_PUZZLE_ITEM5_COMBO1,
	ID_PUZZLE_ITEM5_COMBO2,
	ID_PUZZLE_ITEM6_COMBO1,
	ID_PUZZLE_ITEM6_COMBO2,
	ID_PUZZLE_ITEM7_COMBO1,
	ID_PUZZLE_ITEM7_COMBO2,
	ID_PUZZLE_ITEM8_COMBO1,
	ID_PUZZLE_ITEM8_COMBO2,
	ID_KEY_ITEM1,
	ID_KEY_ITEM2,
	ID_KEY_ITEM3,
	ID_KEY_ITEM4,
	ID_KEY_ITEM5,
	ID_KEY_ITEM6,
	ID_KEY_ITEM7,
	ID_KEY_ITEM8,
	ID_KEY_ITEM1_COMBO1,
	ID_KEY_ITEM1_COMBO2,
	ID_KEY_ITEM2_COMBO1,
	ID_KEY_ITEM2_COMBO2,
	ID_KEY_ITEM3_COMBO1,
	ID_KEY_ITEM3_COMBO2,
	ID_KEY_ITEM4_COMBO1,
	ID_KEY_ITEM4_COMBO2,
	ID_KEY_ITEM5_COMBO1,
	ID_KEY_ITEM5_COMBO2,
	ID_KEY_ITEM6_COMBO1,
	ID_KEY_ITEM6_COMBO2,
	ID_KEY_ITEM7_COMBO1,
	ID_KEY_ITEM7_COMBO2,
	ID_KEY_ITEM8_COMBO1,
	ID_KEY_ITEM8_COMBO2,
	ID_PICKUP_ITEM1,
	ID_PICKUP_ITEM2,
	ID_PICKUP_ITEM3,
	ID_PICKUP_ITEM4,
	ID_PICKUP_ITEM1_COMBO1,
	ID_PICKUP_ITEM1_COMBO2,
	ID_PICKUP_ITEM2_COMBO1,
	ID_PICKUP_ITEM2_COMBO2,
	ID_PICKUP_ITEM3_COMBO1,
	ID_PICKUP_ITEM3_COMBO2,
	ID_PICKUP_ITEM4_COMBO1,
	ID_PICKUP_ITEM4_COMBO2,
	ID_EXAMINE1,
	ID_EXAMINE2,
	ID_EXAMINE3,
	ID_GAME_PIECE1,
	ID_GAME_PIECE2,
	ID_GAME_PIECE3,
	ID_TRIBEBOSS, // TR3 - 36
	ID_HAMMER_ITEM,
	ID_CROWBAR_ITEM,
	ID_BURNING_TORCH_ITEM,
	ID_PUZZLE_HOLE1,
	ID_PUZZLE_HOLE2,
	ID_PUZZLE_HOLE3,
	ID_PUZZLE_HOLE4,
	ID_PUZZLE_HOLE5,
	ID_PUZZLE_HOLE6,
	ID_PUZZLE_HOLE7,
	ID_PUZZLE_HOLE8,
	ID_PUZZLE_DONE1,
	ID_PUZZLE_DONE2,
	ID_PUZZLE_DONE3,
	ID_PUZZLE_DONE4,
	ID_PUZZLE_DONE5,
	ID_PUZZLE_DONE6,
	ID_PUZZLE_DONE7,
	ID_PUZZLE_DONE8,
	ID_KEY_HOLE1,
	ID_KEY_HOLE2,
	ID_KEY_HOLE3,
	ID_KEY_HOLE4,
	ID_KEY_HOLE5,
	ID_KEY_HOLE6,
	ID_KEY_HOLE7,
	ID_KEY_HOLE8,
	ID_SWITCH_TYPE1,
	ID_SWITCH_TYPE2,
	ID_SWITCH_TYPE3,
	ID_SWITCH_TYPE4,
	ID_SWITCH_TYPE5,
	ID_SWITCH_TYPE6,
	ID_SHOOT_SWITCH1,
	ID_SHOOT_SWITCH2,
	ID_AIRLOCK_SWITCH,
	ID_UNDERWATER_SWITCH1,
	ID_UNDERWATER_SWITCH2,
	ID_TURN_SWITCH,
	ID_COG_SWITCH,
	ID_LEVER_SWITCH,
	ID_JUMP_SWITCH,
	ID_CROWBAR_SWITCH,
	ID_PULLEY,
	ID_CROWDOVE_SWITCH,
	ID_DOOR_TYPE1,
	ID_SARCOPHAGUS, // TR4 - OK - Needs testing
	ID_DOOR_TYPE2,
	ID_BIRD_BLADE, // TR4 - OK
	ID_DOOR_TYPE3,
	ID_KAYAK, // TR3 - 14
	ID_DOOR_TYPE4,
	ID_KAYAK_LARA_ANIMS, // TR3 - OK
	ID_DOOR_TYPE5,
	ID_BOAT, // TR2 - 15
	ID_DOOR_TYPE6,
	ID_BOAT_LARA_ANIMS, // TR2 - OK
	ID_DOOR_TYPE7,
	ID_TRIBESMAN_WITH_AX, // TR3 - OK
	ID_DOOR_TYPE8,
	ID_TRIBESMAN_WITH_DARTS, // TR3 - OK
	ID_CLOSED_DOOR1,
	ID_SCUBA_DIVER, // TR3 - OK
	ID_CLOSED_DOOR2,
	ID_CROW, // TR3 - OK
	ID_CLOSED_DOOR3,
	ID_TIGER, // TR3 - OK
	ID_CLOSED_DOOR4,
	ID_EAGLE, // TR3 - OK
	ID_CLOSED_DOOR5,
	ID_RAPTOR, // TR3 - OK
	ID_CLOSED_DOOR6,
	ID_TYRANNOSAUR, // TR3 - OK
	ID_LIFT_DOORS1,
	ID_COBRA, // TR3 - OK
	ID_LIFT_DOORS2,
	ID_MONKEY, // TR3 - OK - Needs testing
	ID_PUSHPULL_DOOR1,
	ID_MP_WITH_STICK, // TR3 - 60
	ID_PUSHPULL_DOOR2,
	ID_MP_WITH_GUN, // TR3 - OK - Needs testing
	ID_KICK_DOOR1,
	ID_WALL_MOUNTED_BLADE, // TR3 - 111
	ID_KICK_DOOR2,
	ID_SPIKY_WALL, // TR3 - OK
	ID_UNDERWATER_DOOR,
	ID_TO_REPLACE1, // TR3 - 127
	ID_DOUBLE_DOORS,
	ID_FISH_EMITTER, // TR3 - 338 - OK - Needs testing and drawing
	ID_SEQUENCE_DOOR1,
	ID_SEQUENCE_SWITCH1,
	ID_SEQUENCE_SWITCH2,
	ID_SEQUENCE_SWITCH3,
	ID_STEEL_DOOR,
	ID_GOD_HEAD,
	ID_PISTOLS_ITEM,
	ID_PISTOLS_AMMO_ITEM,
	ID_UZI_ITEM,
	ID_UZI_AMMO_ITEM,
	ID_SHOTGUN_ITEM,
	ID_SHOTGUN_AMMO1_ITEM,
	ID_SHOTGUN_AMMO2_ITEM,
	ID_CROSSBOW_ITEM,
	ID_CROSSBOW_AMMO1_ITEM,
	ID_CROSSBOW_AMMO2_ITEM,
	ID_CROSSBOW_BOLT,
	ID_HK_ITEM,
	ID_HK_AMMO_ITEM,
	ID_REVOLVER_ITEM,
	ID_REVOLVER_AMMO_ITEM,
	ID_BIGMEDI_ITEM,
	ID_SMALLMEDI_ITEM,
	ID_LASERSIGHT_ITEM,
	ID_BINOCULARS_ITEM,
	ID_SILENCER_ITEM,
	ID_FLARE_ITEM,
	ID_FLARE_INV_ITEM,
	ID_COMPASS_ITEM,
	ID_INVENTORY_PASSPORT,
	ID_INVENTORY_SUNGLASSES,
	ID_INVENTORY_KEYS,
	ID_INVENTORY_HEADPHONES,
	ID_INVENTORY_POLAROID,
	ID_OBELISK, // TR4
	ID_SMOKE_EMITTER_WHITE,
	ID_SMOKE_EMITTER_BLACK,
	ID_SMOKE_EMITTER,
	ID_EARTHQUAKE,
	ID_BUBBLES,
	ID_WATERFALLMIST,
	ID_GUNSHELL,
	ID_SHOTGUNSHELL,
	ID_GUN_FLASH,
	ID_COLOR_LIGHT,
	ID_BLINKING_LIGHT,
	ID_PULSE_LIGHT,
	ID_STROBE_LIGHT,
	ID_ELECTRICAL_LIGHT,
	ID_LENS_FLARE,
	ID_AI_GUARD,
	ID_AI_AMBUSH,
	ID_AI_PATROL1,
	ID_AI_MODIFY,
	ID_AI_FOLLOW,
	ID_AI_PATROL2,
	ID_AI_X1,
	ID_AI_X2,
	ID_LARA_START_POS,
	ID_TELEPORTER,
	ID_LIFT_TELEPORTER,
	ID_RAISING_COG,
	ID_LASERS,
	ID_STEAM_LASERS,
	ID_FLOOR_LASERS,
	ID_KILL_ALL_TRIGGERS,
	ID_TRIGGER_TRIGGERER,
	ID_HIGH_OBJECT1,
	ID_HIGH_OBJECT2,
	ID_SMASH_OBJECT1,
	ID_SMASH_OBJECT2,
	ID_SMASH_OBJECT3,
	ID_SMASH_OBJECT4,
	ID_SMASH_OBJECT5,
	ID_SMASH_OBJECT6,
	ID_SMASH_OBJECT7,
	ID_SMASH_OBJECT8,
	ID_MESHSWAP1,
	ID_MESHSWAP2,
	ID_MESHSWAP3,
	ID_BODY_PART,
	ID_CAMERA_TARGET,
	ID_WATERFALL1,
	ID_WATERFALL2,
	ID_WATERFALL3,
	ID_FISHTANK,
	ID_WATERFALLSS1,
	ID_WATERFALLSS2,
	ID_ANIMATING1,
	ID_BARRACUDA, // TR2 - OK
	ID_ANIMATING2,
	ID_MONK1,
	ID_ANIMATING3,
	ID_MONK2,
	ID_ANIMATING4,
	ID_DISK_SHOOTER, // TR2 - 62 
	ID_ANIMATING5,
	ID_DISK, // TR2 - 61
	ID_ANIMATING6,
	ID_STATUE_WITH_BLADE, // TR2 - OK
	ID_ANIMATING7,
	ID_SPRINGBOARD, // TR2 - OK
	ID_ANIMATING8,
	ID_ROLLING_SPINDLE, // TR2 - OK
	ID_ANIMATING9,
	ID_WOLF, // TR1 - OK NEED TEST
	ID_ANIMATING10,
	ID_BEAR, // TR1 - OK NEED TEST
	ID_ANIMATING11,
	ID_APE, // TR1 - OK NEED TEST
	ID_ANIMATING12,
	ID_WINGED_MUMMY, // TR1 - 20
	ID_ANIMATING13,
	ID_CENTAUR_MUTANT,  // TR1 - 23
	ID_ANIMATING14,
	ID_WINGED_NATLA, // TR1 - 33
	ID_ANIMATING15,
	ID_GIANT_MUTANT, // TR1 - 34
	ID_ANIMATING16,
	ID_SCUBA_HARPOON, // TR3 - OK
	ID_BRIDGE_FLAT,
	ID_BRIDGE_FLAT_MIP,
	ID_BRIDGE_TILT1,
	ID_BRIDGE_TILT1_MIP,
	ID_BRIDGE_TILT2,
	ID_BRIDGE_TILT2_MIP,
	ID_HORIZON,
	ID_SKY_GRAPHICS,
	ID_BINOCULAR_GRAPHICS,
	ID_TARGET_GRAPHICS,
	ID_DEFAULT_SPRITES,
	ID_MISC_SPRITES,
	ID_ENERGY_BUBBLES, // TR4 - Energy attacks of harpy, demigod...
	ID_GRENADE_ITEM,
	ID_GRENADE_AMMO1_ITEM,
	ID_GRENADE,
	ID_GRENADE_ANIM,
	ID_HARPOON_ITEM,
	ID_HARPOON_AMMO_ITEM,
	ID_HARPOON,
	ID_HARPOON_ANIM,
	ID_ROCKET_LAUNCHER_ITEM,
	ID_ROCKET_LAUNCHER_AMMO_ITEM,
	ID_ROCKET,
	ID_ROCKET_ANIM,
	ID_CROSSBOW_AMMO3_ITEM,
	ID_GRENADE_AMMO2_ITEM,
	ID_GRENADE_AMMO3_ITEM,
	ID_GOLDROSE_ITEM,
	ID_DIARY,
	ID_VON_CROY,  // TR4
	ID_SETHA, // TR4
	ID_KNIGHT_TEMPLAR, // TR4 - OK
	ID_SCORPION, // TR4 - OK
	ID_NEW_SLOT_482,
	ID_AHMET, // TR4
	ID_LARA_DOUBLE, // TR4 - OK - Needs testing
	ID_PLANET_EFFECT, // TR4
	ID_BUTTERFLY, // TR4
	ID_BIG_BEETLE, // TR4
	ID_VEHICLE_SMASHABLE_FLOOR, // TR4
	ID_VEHICLE_SMASHABLE_WALL, // TR4
	ID_SENTRY_GUN, // TR4 - OK
	ID_MINE,  // TR4 - OK - Needs testing
	ID_MAPPER, // TR4
	ID_WHEEL_OF_FORTUNE, // TR4
	ID_LIGHTING_CONDUCTOR, // TR4
	ID_LOCUST_EMITTER, // TR4
	ID_SCALES, // TR4 - OK - Needs testing
	ID_WATERSKIN1_EMPTY, // TR4 - OK
	ID_WATERSKIN1_1, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN1_2, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN1_3, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_EMPTY, // TR4 - OK
	ID_WATERSKIN2_1, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_2, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_3, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_4, // TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_5, // TR4 - OK - Needs testing and inventory handling
	ID_LARA_WATER_MESH, // TR4
	ID_LARA_PETROL_MESH, // TR4
	ID_LARA_DIRT_MESH, // TR4
	ID_JEAN_YVES, // TR4 - OK - Needs testing
	ID_TROOPS, // TR4 - OK - Needs testing
	ID_WRAITH1, // TR4
	ID_WRAITH2, // TR4
	ID_WRAITH3, // TR4
	ID_WRAITH4, // TR4
	ID_CLOCKWORK_BEETLE, // TR4
	ID_CLOCKWORK_BEETLE_COMBO1, // TR4
	ID_CLOCKWORK_BEETLE_COMBO2, // TR4
	ID_PROJ_NATLA, // TR1 - OFF
	ID_PROJ_SHARD, // TR1 - OFF
	ID_PROJ_BOMB,  // TR1 - OFF
	ID_EVIL_LARA,  // TR1 - OFF (Cant be targeted (if enabled, evil lara die anim is played)) (cant move for now)
	ID_NATLA,      // TR1 - OFF
	ID_GOON_SILENCER1, // TR2 - ON
	ID_GOON_SILENCER2, // TR2 - ON
	ID_GOON_SILENCER3, // TR2 - ON
	ID_WORKER_SHOTGUN, // TR2 - ON
	ID_WORKER_MACHINEGUN, // TR2 - ON
	ID_WORKER_DUAL_REVOLVER, // TR2 - ON
	ID_WORKER_FLAMETHROWER, // TR2 - ON
	ID_SMALL_SPIDER, // TR2 - ON
	ID_BIG_SPIDER, // TR2 - ON
	ID_TONY_BOSS, // TR3 - 90% (need effect)
	ID_BIRDMONSTER, // TR2 - ON
	ID_KNIFETHROWER, // TR2 - (Crash the game when shooting)
	ID_KNIFETHROWER_KNIFE, // TR2 - ON
	ID_MERCENARY_UZI, // TR2 - ON (Need to detect the monk first)
	ID_MERCENARY_AUTOPISTOLS1, // TR2 - ON (Need to detect the monk first)
	ID_MERCENARY_AUTOPISTOLS2, // TR2 - ON (Need to detect the monk first)
	ID_SWORD_GUARDIAN, // TR2 - ON (blocked if the floor are 2+ click) (need a custom render)
	ID_SWORD_GUARDIAN_STATUE, // TR2 - ON
	ID_SPEAR_GUARDIAN, // TR2 - ON (need a custom render)
	ID_SPEAR_GUARDIAN_STATUE, // TR2 - ON
	ID_SHIVA, // TR3 - (Need Test)
	ID_SHIVA_STATUE, // TR3 - (Need Test)
	ID_DRAGON_FRONT, // TR2 - (Need Test) (Need flame effect) (Need Explosion Effect (TR2 one)) (spawned by MARCO_BARTOLI)
	ID_DRAGON_BACK,  // TR2 - (Need Test)
	ID_DRAGON_BONE_FRONT, // TR2 - ON
	ID_DRAGON_BONE_BACK, // TR2 - ON
	ID_SPHERE_OF_DOOM, // TR2 - 209 (Dragon Explosion Effect 1)
	ID_SPHERE_OF_DOOM2, // TR2 - 210 (Dragon Explosion Effect 2)
	ID_SPHERE_OF_DOOM3, // TR2 - 211 (Dragon Explosion Effect 3)
	ID_MARCO_BARTOLI, // TR2 - (Need Test)
	ID_SNOWMOBILE_GUN, // TR2 - (Need Test)
	ID_SNOWMOBILE_DRIVER, // TR2 - (Need Test)
	ID_MINECART, // TR3 - (Need Test)
	ID_MINECART_LARA_ANIMS, // TR3 - (Need Test)
	ID_UPV, // TR3 - 90% (Need new render) (lara cant enter)
	ID_UPV_LARA_ANIMS, // TR3 - ON
	ID_LASER_BOLT, // TR3 - OFF (Boss Laser Bolt)
	ID_DIVER, // TR3 - OFF
	ID_WHALE, // TR3 - OFF
	ID_WILLARD_BOSS, // TR3 - OFF
	ID_WHITE_SOLDIER, // TR3 - OFF (Oil-SMG)
	ID_MUTANT1, // TR3 - OFF (Wingmute)
	ID_MUTANT2, // TR3 - OFF (Claw Mutant)
	ID_MUTANT3, // TR3 - OFF (Hybrid Mutant)
	ID_BURNT_MUTANT, // TR3 - OFF (Seal Mutant)
	ID_SWAT_GUN, // TR3 - OFF
	ID_STHPAC_MERCENARY, // TR3 - OFF
	ID_LIZARD_MAN, // TR3 - OFF
	ID_MP1, // TR3 - OFF (Area 51 Baton Man)
	ID_MP2, // TR3 - OFF (Area 51  Pistol MP)
	ID_CIVVIE, // TR3 - OFF
	ID_PRISONER, // TR3 - OFF (Area 51 Prisoner)
	ID_SECURITY_GUARD, // TR3 - OFF
	ID_ELECTRIC_CLEANER, // TR3 - OFF
	ID_LON_MERCENARY1, // TR3 - OFF
	ID_PUNK1, // TR3 - OFF
	ID_WINSTON, // TR3 - OFF
	ID_ARMY_WINSTON, // TR3 - OFF
	ID_TARGETS, // TR3 - OFF
	ID_SMALL_DINOSAUR, // TR3 - 34
	ID_LIZARD_THING, // TR3 - 35
	ID_PUNA_BOSS, // TR3 - 36
	ID_SOPHIA_LEE_BOSS, // TR3 - 57
	ID_EXAMINE1_COMBO1,
	ID_EXAMINE1_COMBO2,
	ID_EXAMINE2_COMBO1,
	ID_EXAMINE2_COMBO2,
	ID_EXAMINE3_COMBO1,
	ID_EXAMINE3_COMBO2,

	ID_NUMBER_OBJECTS
};
#else
typedef enum GAME_OBJECT_ID
{
	/* Lara Primary Slot */
	ID_LARA,
	ID_LARA_EXTRA_ANIMS,
	ID_PISTOLS_ANIM,
	ID_UZI_ANIM,
	ID_SHOTGUN_ANIM,
	ID_REVOLVER_ANIM,
	ID_CROSSBOW_ANIM,
	ID_HK_ANIM,
	ID_GRENADE_ANIM,
	ID_ROCKET_ANIM,
	ID_HARPOON_ANIM,
	ID_LARA_FLARE_ANIM,
	ID_LARA_SKIN,
	ID_LARA_SKIN_JOINTS,
	ID_LARA_SCREAM,
	ID_LARA_CROSSBOW_LASER,
	ID_LARA_REVOLVER_LASER,
	ID_LARA_HOLSTERS,
	ID_LARA_HOLSTERS_PISTOLS,
	ID_LARA_HOLSTERS_UZIS,
	ID_LARA_HOLSTERS_REVOLVER,
	ID_LARA_SPEECH_HEAD1,
	ID_LARA_SPEECH_HEAD2,
	ID_LARA_SPEECH_HEAD3,
	ID_LARA_SPEECH_HEAD4,
	ID_ACTOR1_SPEECH_HEAD1,
	ID_ACTOR1_SPEECH_HEAD2,
	ID_ACTOR2_SPEECH_HEAD1,
	ID_ACTOR2_SPEECH_HEAD2,
	ID_LARA_EXTRA_MESH1,
	ID_LARA_EXTRA_MESH2,
	ID_LARA_EXTRA_MESH3,
	ID_LARA_WATER_MESH,
	ID_LARA_PETROL_MESH,
	ID_LARA_DIRT_MESH,
	ID_LARA_CROWBAR_ANIM,
	ID_LARA_TORCH_ANIM,
	ID_LARA_HAIR,

	/* Lara Vehicles */
	ID_SNOWMOBILE_LARA_ANIMS,
	ID_SNOWMOBILE,
	ID_QUAD_LARA_ANIMS,
	ID_QUAD,
	ID_BOAT_LARA_ANIMS,
	ID_BOAT,
	ID_KAYAK_LARA_ANIMS,
	ID_KAYAK,
	ID_UPV_LARA_ANIMS,
	ID_UPV,											// TR3 - 90% (Need new render) (lara cant enter)
	ID_MINECART_LARA_ANIMS,
	ID_MINECART,
	ID_JEEP_LARA_ANIMS,
	ID_JEEP,										// TR4 - 95% Working
	ID_MOTORBIKE_LARA_ANIMS,
	ID_MOTORBIKE,

	/* Enemy */
	ID_WOLF,
	ID_BEAR,
	ID_APE,
	ID_WINGED_MUMMY,
	ID_CENTAUR_MUTANT,
	ID_EVIL_LARA,
	ID_NATLA,
	ID_WINGED_NATLA,
	ID_GIANT_MUTANT,
	ID_PROJ_NATLA,
	ID_PROJ_SHARD,
	ID_PROJ_BOMB,
	ID_SCUBA_HARPOON,
	ID_SMALL_SPIDER,
	ID_BIG_SPIDER,
	ID_GOON_SILENCER1,
	ID_GOON_SILENCER2,
	ID_GOON_SILENCER3,
	ID_BARRACUDA,
	ID_WORKER_SHOTGUN,
	ID_WORKER_MACHINEGUN,
	ID_WORKER_DUAL_REVOLVER,
	ID_WORKER_FLAMETHROWER,
	ID_MONK1,
	ID_MONK2,
	ID_BIRDMONSTER,
	ID_KNIFETHROWER,
	ID_KNIFETHROWER_KNIFE,
	ID_MERCENARY_UZI,
	ID_MERCENARY_AUTOPISTOLS1,
	ID_MERCENARY_AUTOPISTOLS2,
	ID_SPEAR_GUARDIAN,
	ID_SPEAR_GUARDIAN_STATUE,
	ID_SWORD_GUARDIAN,
	ID_SWORD_GUARDIAN_STATUE,
	ID_SHIVA,
	ID_SHIVA_STATUE,
	ID_MARCO_BARTOLI,
	ID_DRAGON_FRONT,
	ID_DRAGON_BACK,
	ID_DRAGON_BONE_FRONT,
	ID_DRAGON_BONE_BACK,
	ID_SNOWMOBILE_GUN,
	ID_SNOWMOBILE_DRIVER,
	ID_ENEMY_JEEP,
	ID_SAS,
	ID_SWAT,
	ID_SWAT_PLUS,
	ID_BLUE_GUARD,
	ID_TWOGUN,
	ID_SKELETON,
	ID_DOG,
	ID_CROCODILE,
	ID_MUMMY,
	ID_TR5_LARSON,
	ID_SMALL_SCORPION,
	ID_TR5_PIERRE,
	ID_YETI,
	ID_COMPY,
	ID_BAT,
	ID_RAT,
	ID_MAFIA,
	ID_GUIDE,
	ID_MAFIA2,
	ID_SPHINX,
	ID_SAILOR,
	ID_HORSEMAN,
	ID_CRANE_GUY,
	ID_HORSE,
	ID_LION,
	ID_WILD_BOAR,
	ID_GLADIATOR,
	ID_HARPY,
	ID_ROMAN_GOD,
	ID_HYDRA,
	ID_DEMIGOD1,
	ID_DEMIGOD2,
	ID_DEMIGOD3,
	ID_GUARDIAN,
	ID_HITMAN,
	ID_LITTLE_BEETLE,
	ID_SCIENTIST,
	ID_MERCENARY,
	ID_WILLOWISP,
	ID_SHARK,
	ID_INVISIBLE_GHOST,
	ID_BADDY1,
	ID_BADDY2,
	ID_REAPER,
	ID_BROWN_BEAST,
	ID_ATTACK_SUB,
	ID_SNIPER,
	ID_HUSKIE,
	ID_COG,
	ID_SAS_CAIRO,
	ID_IMP,
	ID_IMP_ROCK,			// maybe needed for imp so moved here !
	ID_BATS,
	ID_RATS,
	ID_SPIDER,
	ID_FLAMETHROWER_BADDY,
	ID_GUNSHIP,
	ID_AUTOGUN,
	ID_TRIBEBOSS,
	ID_TRIBESMAN_WITH_AX,
	ID_TRIBESMAN_WITH_DARTS,
	ID_SCUBA_DIVER,
	ID_CROW,
	ID_TIGER,
	ID_EAGLE,
	ID_RAPTOR,
	ID_TYRANNOSAUR,
	ID_COBRA,
	ID_MONKEY,
	ID_MP_WITH_STICK,
	ID_MP_WITH_GUN,
	ID_MUTANT2,
	ID_WHALE,
	ID_SMALL_DINOSAUR,		// TR3 - 34
	ID_LIZARD,				// TR3 - 35
	ID_TONY_BOSS,
	ID_PUNA_BOSS,			// TR3 - 36
	ID_SOPHIA_LEE_BOSS,		// TR3 - 57
	ID_LASER_BOLT,
	ID_LARA_DOUBLE,			// TR4 - OK - Needs testing
	ID_FISH_EMITTER,		// TR3 - 338 - OK - Needs testing and drawing
	ID_VON_CROY,
	ID_SETHA,
	ID_KNIGHT_TEMPLAR,
	ID_SCORPION,
	ID_AHMET,
	ID_BIG_BEETLE,
	ID_SENTRY_GUN,
	ID_JEAN_YVES,			// TR4 - OK - Needs testing
	ID_TROOPS,				// TR4 - OK - Needs testing
	ID_WRAITH1,
	ID_WRAITH2,
	ID_WRAITH3,
	ID_WRAITH4,
	ID_SPHERE_OF_DOOM,      // Used by the dragon/boss to appear or death explosion.
	ID_SPHERE_OF_DOOM2,
	ID_SPHERE_OF_DOOM3,

	/* Traps / Doors */
	ID_ELECTRICAL_CABLES,
	ID_SPIKEBALL,
	ID_BURNING_FLOOR,
	ID_FLOOR_4BLADES,
	ID_CEILING_4BLADES,
	ID_CATWALK_BLADE,
	ID_SETH_BLADE,
	ID_PLINTH_BLADE,
	ID_SLICER_DICER,			// TR4 - OK - Needs testing
	ID_BIRD_BLADE,
	ID_WALL_MOUNTED_BLADE,		// TR3 - 111
	ID_STATUE_WITH_BLADE,
	ID_SCALES,					// TR4 - OK - Needs testing
	ID_DARTS,
	ID_DART_EMITTER,
	ID_HOMING_DART_EMITTER,
	ID_DISK_SHOOTER,
	ID_DISK,
	ID_FALLING_CEILING,
	ID_FALLING_BLOCK,
	ID_FALLING_BLOCK2,
	ID_CRUMBLING_FLOOR,
	ID_TRAPDOOR1,
	ID_TRAPDOOR2,
	ID_TRAPDOOR3,
	ID_FLOOR_TRAPDOOR1,
	ID_FLOOR_TRAPDOOR2,
	ID_CEILING_TRAPDOOR1,
	ID_CEILING_TRAPDOOR2,
	ID_SCALING_TRAPDOOR,
	ID_ROLLINGBALL,
	ID_ROLLINGBARREL,
	ID_CHAIN,
	ID_PLOUGH,
	ID_STARGATE,
	ID_SPIKY_WALL,
	ID_SPIKY_CEILING,
	ID_TEETH_SPIKES,
	ID_ROME_HAMMER,
	ID_HAMMER2,
	ID_FLAME,
	ID_FLAME_EMITTER,
	ID_FLAME_EMITTER2,
	ID_FLAME_EMITTER3,
	ID_TO_REPLACE_1,			// TR3 - 127
	ID_TO_REPLACE_2,
	ID_BURNING_ROOTS,
	ID_ROPE,
	ID_FIREROPE,
	ID_POLEROPE,
	ID_PROPELLER_H,
	ID_PROPELLER_V,
	ID_ONEBLOCK_PLATFORM,
	ID_TWOBLOCK_PLATFORM,
	ID_RAISING_BLOCK1,
	ID_RAISING_BLOCK2,
	ID_PUSHABLE_OBJECT1,
	ID_PUSHABLE_OBJECT2,
	ID_PUSHABLE_OBJECT3,
	ID_PUSHABLE_OBJECT4,
	ID_PUSHABLE_OBJECT5,
	ID_WRECKING_BALL,
	ID_ZIPLINE_HANDLE,
	ID_TORPEDO,
	ID_CHAFF,
	ID_ELECTRIC_FENCE,
	ID_LIFT,
	ID_TIGHT_ROPE,
	ID_PARALLEL_BARS,
	ID_XRAY_CONTROLLER,
	ID_PORTAL,
	ID_GEN_SLOT1,
	ID_GEN_SLOT2,
	ID_GEN_SLOT3,
	ID_GEN_SLOT4,
	ID_OBELISK,
	ID_WHEEL_OF_FORTUNE,
	ID_GAME_PIECE1,
	ID_GAME_PIECE2,
	ID_GAME_PIECE3,
	ID_SPRINGBOARD,
	ID_ROLLING_SPINDLE,
	ID_RAISING_COG,
	ID_MINE,				// TR4 - OK - Needs testing

	/* Searchable Objects */

	ID_SEARCH_OBJECT1,
	ID_SEARCH_OBJECT2,
	ID_SEARCH_OBJECT3,
	ID_SEARCH_OBJECT4,
	ID_SARCOPHAGUS,

	/* Items */

	ID_PUZZLE_ITEM1,
	ID_PUZZLE_ITEM2,
	ID_PUZZLE_ITEM3,
	ID_PUZZLE_ITEM4,
	ID_PUZZLE_ITEM5,
	ID_PUZZLE_ITEM6,
	ID_PUZZLE_ITEM7,
	ID_PUZZLE_ITEM8,
	ID_PUZZLE_ITEM1_COMBO1,
	ID_PUZZLE_ITEM1_COMBO2,
	ID_PUZZLE_ITEM2_COMBO1,
	ID_PUZZLE_ITEM2_COMBO2,
	ID_PUZZLE_ITEM3_COMBO1,
	ID_PUZZLE_ITEM3_COMBO2,
	ID_PUZZLE_ITEM4_COMBO1,
	ID_PUZZLE_ITEM4_COMBO2,
	ID_PUZZLE_ITEM5_COMBO1,
	ID_PUZZLE_ITEM5_COMBO2,
	ID_PUZZLE_ITEM6_COMBO1,
	ID_PUZZLE_ITEM6_COMBO2,
	ID_PUZZLE_ITEM7_COMBO1,
	ID_PUZZLE_ITEM7_COMBO2,
	ID_PUZZLE_ITEM8_COMBO1,
	ID_PUZZLE_ITEM8_COMBO2,
	ID_KEY_ITEM1,
	ID_KEY_ITEM2,
	ID_KEY_ITEM3,
	ID_KEY_ITEM4,
	ID_KEY_ITEM5,
	ID_KEY_ITEM6,
	ID_KEY_ITEM7,
	ID_KEY_ITEM8,
	ID_KEY_ITEM1_COMBO1,
	ID_KEY_ITEM1_COMBO2,
	ID_KEY_ITEM2_COMBO1,
	ID_KEY_ITEM2_COMBO2,
	ID_KEY_ITEM3_COMBO1,
	ID_KEY_ITEM3_COMBO2,
	ID_KEY_ITEM4_COMBO1,
	ID_KEY_ITEM4_COMBO2,
	ID_KEY_ITEM5_COMBO1,
	ID_KEY_ITEM5_COMBO2,
	ID_KEY_ITEM6_COMBO1,
	ID_KEY_ITEM6_COMBO2,
	ID_KEY_ITEM7_COMBO1,
	ID_KEY_ITEM7_COMBO2,
	ID_KEY_ITEM8_COMBO1,
	ID_KEY_ITEM8_COMBO2,
	ID_PICKUP_ITEM1,
	ID_PICKUP_ITEM2,
	ID_PICKUP_ITEM3,
	ID_PICKUP_ITEM4,
	ID_PICKUP_ITEM1_COMBO1,
	ID_PICKUP_ITEM1_COMBO2,
	ID_PICKUP_ITEM2_COMBO1,
	ID_PICKUP_ITEM2_COMBO2,
	ID_PICKUP_ITEM3_COMBO1,
	ID_PICKUP_ITEM3_COMBO2,
	ID_PICKUP_ITEM4_COMBO1,
	ID_PICKUP_ITEM4_COMBO2,
	ID_EXAMINE1,
	ID_EXAMINE2,
	ID_EXAMINE3,
	ID_HAMMER_ITEM,
	ID_CROWBAR_ITEM,
	ID_BURNING_TORCH_ITEM,
	ID_PUZZLE_HOLE1,
	ID_PUZZLE_HOLE2,
	ID_PUZZLE_HOLE3,
	ID_PUZZLE_HOLE4,
	ID_PUZZLE_HOLE5,
	ID_PUZZLE_HOLE6,
	ID_PUZZLE_HOLE7,
	ID_PUZZLE_HOLE8,
	ID_PUZZLE_DONE1,
	ID_PUZZLE_DONE2,
	ID_PUZZLE_DONE3,
	ID_PUZZLE_DONE4,
	ID_PUZZLE_DONE5,
	ID_PUZZLE_DONE6,
	ID_PUZZLE_DONE7,
	ID_PUZZLE_DONE8,
	ID_KEY_HOLE1,
	ID_KEY_HOLE2,
	ID_KEY_HOLE3,
	ID_KEY_HOLE4,
	ID_KEY_HOLE5,
	ID_KEY_HOLE6,
	ID_KEY_HOLE7,
	ID_KEY_HOLE8,
	ID_CLOCKWORK_BEETLE,
	ID_CLOCKWORK_BEETLE_COMBO1,
	ID_CLOCKWORK_BEETLE_COMBO2,
	ID_SWITCH_TYPE1,
	ID_SWITCH_TYPE2,
	ID_SWITCH_TYPE3,
	ID_SWITCH_TYPE4,
	ID_SWITCH_TYPE5,
	ID_SWITCH_TYPE6,
	ID_SHOOT_SWITCH1,
	ID_SHOOT_SWITCH2,
	ID_AIRLOCK_SWITCH,
	ID_UNDERWATER_SWITCH1,
	ID_UNDERWATER_SWITCH2,
	ID_TURN_SWITCH,
	ID_COG_SWITCH,
	ID_LEVER_SWITCH,
	ID_JUMP_SWITCH,
	ID_CROWBAR_SWITCH,
	ID_PULLEY,
	ID_CROWDOVE_SWITCH,
	ID_DOOR_TYPE1,
	ID_DOOR_TYPE2,
	ID_DOOR_TYPE3,
	ID_DOOR_TYPE4,
	ID_DOOR_TYPE5,
	ID_DOOR_TYPE6,
	ID_DOOR_TYPE7,
	ID_DOOR_TYPE8,
	ID_CLOSED_DOOR1,
	ID_CLOSED_DOOR2,
	ID_CLOSED_DOOR3,
	ID_CLOSED_DOOR4,
	ID_CLOSED_DOOR5,
	ID_CLOSED_DOOR6,
	ID_LIFT_DOORS1,
	ID_LIFT_DOORS2,
	ID_PUSHPULL_DOOR1,
	ID_PUSHPULL_DOOR2,
	ID_KICK_DOOR1,
	ID_KICK_DOOR2,
	ID_UNDERWATER_DOOR,
	ID_DOUBLE_DOORS,
	ID_SEQUENCE_DOOR1,
	ID_SEQUENCE_SWITCH1,
	ID_SEQUENCE_SWITCH2,
	ID_SEQUENCE_SWITCH3,
	ID_STEEL_DOOR,
	ID_GOD_HEAD,

	/* Lara Items */

	ID_PISTOLS_ITEM,
	ID_PISTOLS_AMMO_ITEM,
	ID_UZI_ITEM,
	ID_UZI_AMMO_ITEM,
	ID_SHOTGUN_ITEM,
	ID_SHOTGUN_AMMO1_ITEM,
	ID_SHOTGUN_AMMO2_ITEM,
	ID_REVOLVER_ITEM,
	ID_REVOLVER_AMMO_ITEM,
	ID_CROSSBOW_ITEM,
	ID_CROSSBOW_AMMO1_ITEM,
	ID_CROSSBOW_AMMO2_ITEM,
	ID_CROSSBOW_AMMO3_ITEM,
	ID_CROSSBOW_BOLT,
	ID_HK_ITEM,
	ID_HK_AMMO_ITEM,
	ID_GRENADE_ITEM,
	ID_GRENADE_AMMO1_ITEM,
	ID_GRENADE_AMMO2_ITEM,
	ID_GRENADE_AMMO3_ITEM,
	ID_GRENADE,
	ID_ROCKET_LAUNCHER_ITEM,
	ID_ROCKET_LAUNCHER_AMMO_ITEM,
	ID_ROCKET,
	ID_HARPOON_ITEM,
	ID_HARPOON_AMMO_ITEM,
	ID_HARPOON,
	ID_GOLDROSE_ITEM,
	ID_BIGMEDI_ITEM,
	ID_SMALLMEDI_ITEM,
	ID_LASERSIGHT_ITEM,
	ID_BINOCULARS_ITEM,
	ID_SILENCER_ITEM,
	ID_FLARE_ITEM,
	ID_FLARE_INV_ITEM,
	ID_COMPASS_ITEM,
	ID_DIARY,
	ID_INVENTORY_PASSPORT,
	ID_INVENTORY_SUNGLASSES,
	ID_INVENTORY_KEYS,
	ID_INVENTORY_HEADPHONES,
	ID_INVENTORY_POLAROID,
	ID_WATERSKIN1_EMPTY,
	ID_WATERSKIN1_1,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN1_2,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN1_3,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_EMPTY,
	ID_WATERSKIN2_1,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_2,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_3,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_4,				// TR4 - OK - Needs testing and inventory handling
	ID_WATERSKIN2_5,				// TR4 - OK - Needs testing and inventory handling

	/* other effect in-game */
	ID_SMOKE_EMITTER_WHITE,
	ID_SMOKE_EMITTER_BLACK,
	ID_SMOKE_EMITTER,
	ID_LOCUST_EMITTER,
	ID_EARTHQUAKE,
	ID_BUBBLES,
	ID_WATERFALLMIST,
	ID_GUNSHELL,
	ID_SHOTGUNSHELL,
	ID_GUN_FLASH,
	ID_GUN_FLASH2,					// used for M16/MP5/HK
	ID_LIGHTING_CONDUCTOR,
	ID_DYNAMIC_LIGHT,				// Merge all lights in a single null mesh
	ID_LENS_FLARE,
	ID_ENERGY_BUBBLES,
	ID_PLANET_EFFECT,
	ID_BUTTERFLY,
	ID_AI_GUARD,
	ID_AI_AMBUSH,
	ID_AI_PATROL1,
	ID_AI_MODIFY,
	ID_AI_FOLLOW,
	ID_AI_PATROL2,
	ID_AI_X1,
	ID_AI_X2,
	ID_LARA_START_POS,
	ID_TELEPORTER,
	ID_LIFT_TELEPORTER,
	ID_LASERS,
	ID_MAPPER,
	ID_STEAM_LASERS,
	ID_FLOOR_LASERS,
	ID_KILL_ALL_TRIGGERS,
	ID_TRIGGER_TRIGGERER,
	ID_HIGH_OBJECT1,
	ID_HIGH_OBJECT2,
	ID_SMASH_OBJECT1,
	ID_SMASH_OBJECT2,
	ID_SMASH_OBJECT3,
	ID_SMASH_OBJECT4,
	ID_SMASH_OBJECT5,
	ID_SMASH_OBJECT6,
	ID_SMASH_OBJECT7,
	ID_SMASH_OBJECT8,
	ID_VEHICLE_SMASHABLE_FLOOR,
	ID_VEHICLE_SMASHABLE_WALL,
	ID_MESHSWAP1,
	ID_MESHSWAP2,
	ID_MESHSWAP3,
	ID_BODY_PART,
	ID_CAMERA_TARGET,
	ID_WATERFALL1,
	ID_WATERFALL2,
	ID_WATERFALL3,
	ID_WATERFALLSS1,
	ID_WATERFALLSS2,
	ID_FISHTANK,
	ID_ANIMATING1,
	ID_ANIMATING2,
	ID_ANIMATING3,
	ID_ANIMATING4,
	ID_ANIMATING5,
	ID_ANIMATING6,
	ID_ANIMATING7,
	ID_ANIMATING8,
	ID_ANIMATING9,
	ID_ANIMATING10,
	ID_ANIMATING11,
	ID_ANIMATING12,
	ID_ANIMATING13,
	ID_ANIMATING14,
	ID_ANIMATING15,
	ID_ANIMATING16,
	ID_BRIDGE_FLAT,
	ID_BRIDGE_TILT1,
	ID_BRIDGE_TILT2,
	ID_HORIZON,
	ID_BINOCULAR_GRAPHICS,
	ID_TARGET_GRAPHICS,
	ID_SKY_GRAPHICS,
	ID_DEFAULT_SPRITES,
	ID_MISC_SPRITES,
	ID_CUSTOM_SPRITES,
	ID_NUMBER_OBJECTS,		// NEED TO BE AT THE END !!!!
};
#endif