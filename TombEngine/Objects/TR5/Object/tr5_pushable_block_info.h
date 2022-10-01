#pragma once

struct PushableInfo
{
	int Height;		   // Height for collision. Also in floor procedure.
	int Weight;
	int StackLimit;
	int MoveX;		   // Used for pushable movement code.
	int MoveZ;		   // Used for pushable movement code.
	short Gravity;	   // Fall acceleration.
	short FallSoundID; // Index of sound upon hidding the floor.

	short LinkedIndex; // Using itemFlags[1] for now.
	short Climb;	   // Unused for now.

	bool CanFall;		  // OCB 32
	bool HasFloorCeiling; // Has floor and ceiling procedures (OCB 64)
	bool DisablePull;	  // OCB 128
	bool DisablePush;	  // OCB 256
	bool DisableW;		  // OCB 512 (W+E)
	bool DisableE;		  // OCB 512 (W+E)
	bool DisableN;		  // OCB 1024 (N+S)
	bool DisableS;		  // OCB 1024 (N+S)
};
