#pragma once
#include <vector>
#include "Specific/phd_global.h"

struct ItemInfo;

struct BOX_NODE 
{
	int exitBox;
	int searchNumber;
	int nextExpansion;
	int boxNumber;
};

enum ZoneType
{
	ZT_Skeleton,
	ZT_Basic,
	ZT_Croc, // water zone (also include ground ?)
	ZT_Human,
	ZT_VonCroy,
	ZT_Fly
};

enum ZoneTypeAttribute
{
	ZTA_None = -1, // default zone (which defined
	ZTA_Skeleton,
	ZTA_Basic,
	ZTA_Croc, // water zone (also include ground ?)
	ZTA_Human, // no jump nor monkey !
	ZTA_VonCroy, // jump + monkey + longjump
	ZTA_Fly,

	// these are not real zone, just addons for InitialiseSlot()

	ZTA_HumanJump,
	ZTA_HumanJumpAndMonkey,
	ZTA_HumanLongJumpAndMonkey, // von croy
	ZTA_Spider,
	ZTA_Blockable, // for trex, shiva, etc..
	ZTA_SophiaLee, // dont want sophia to go down again !
	ZTA_Ape, // only 2 click climb
	ZTA_SkidooArmed
};

struct LOTInfo 
{
	bool Initialised;

	std::vector<BOX_NODE> Node;
	int Head;
	int Tail;

	int SearchNumber;
	int BlockMask;
	short Step;
	short Drop;
	short ZoneCount;
	int TargetBox;
	int RequiredBox;
	short Fly;

	bool CanJump;
	bool CanMonkey;
	bool IsJumping;
	bool IsMonkeying;
	bool IsAmphibious;

	Vector3Int Target;
	ZoneType Zone;
};

enum class MoodType 
{
	Bored,
	Attack,
	Escape,
	Stalk
};

enum class CreatureAIPriority
{
	None,
	High,
	Medium,
	Low
};

struct CreatureInfo 
{
	short ItemNumber;

	short MaxTurn;
	short JointRotation[4];
	bool HeadLeft;
	bool HeadRight;

	bool Patrol;			// Unused?
	bool Alerted;
	bool Friendly;
	bool HurtByLara;
	bool Poisoned;
	bool JumpAhead;
	bool MonkeySwingAhead;
	bool ReachedGoal;

	short Tosspad;
	short LocationAI;
	short FiredWeapon;

	LOTInfo LOT;
	MoodType Mood;
	ItemInfo* Enemy;
	short AITargetNumber;
	ItemInfo* AITarget;
	short Pad;				// Unused?
	Vector3Int Target;

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CreatureAIPriority Priority;
	size_t FramesSinceLOTUpdate;
#endif

	short Flags;
};
