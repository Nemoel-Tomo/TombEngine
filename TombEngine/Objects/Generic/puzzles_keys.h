#pragma once

struct CollisionInfo;
struct ItemInfo;

// Puzzles
void PuzzleHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void PuzzleDoneCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void PuzzleDone(ItemInfo* item, short itemNumber);
void DoPuzzle();

// Keys
void KeyHoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
