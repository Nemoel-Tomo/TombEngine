#pragma once

struct PushableInfo
{
	int height;				// Height for collision. Also in floor procedure.
	int weight;
	int stackLimit;
	int moveX;				// Used for pushable movement code.
	int moveZ;				// Used for pushable movement code
	short linkedIndex;		// Using itemFlags[1] for now
	short gravity;			// Fall acceleration
	short loopSound;		// Looped sound index for movement
	short stopSound;		// Ending sound index
	short fallSound;		// Sound on hitting floor (if dropped)
	short climb;			// Unused for now.
	bool canFall;			// OCB 32
	bool hasFloorCeiling;	// has floor and ceiling procedures (OCB 64)
	bool disablePull;		// OCB 128
	bool disablePush;		// OCB 256
	bool disableW;			// OCB 512 (W+E)
	bool disableE;			// OCB 512 (W+E)
	bool disableN;			// OCB 1024 (N+S)
	bool disableS;			// OCB 1024 (N+S)
};
