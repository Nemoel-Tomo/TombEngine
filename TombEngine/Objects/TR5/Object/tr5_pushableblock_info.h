#pragma once

struct PushableInfo
{
	int Height;				// Height for collision. Also in floor procedure.
	int Weight;
	int StackLimit;
	int MoveX;				// Used for pushable movement code.
	int MoveZ;				// Used for pushable movement code
	short LinkedIndex;		// Using itemFlags[1] for now
	short Gravity;			// Fall acceleration
	short LoopSound;		// Looped sound index for movement
	short StopSound;		// Ending sound index
	short FallSound;		// Sound on hitting floor (if dropped)
	short Climb;			// Unused for now.
	bool CanFall;			// OCB 32
	bool HasFloorCeiling;	// has floor and ceiling procedures (OCB 64)
	bool DisablePull;		// OCB 128
	bool DisablePush;		// OCB 256
	bool DisableW;			// OCB 512 (W+E)
	bool DisableE;			// OCB 512 (W+E)
	bool DisableN;			// OCB 1024 (N+S)
	bool DisableS;			// OCB 1024 (N+S)
};
