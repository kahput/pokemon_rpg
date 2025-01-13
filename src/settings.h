#pragma once

// fixed width numeric types
#include <inttypes.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef uint32_t b32;
typedef uint8_t b8;

// Game settings
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define TILE_SIZE 64
#define ANIMATION_SPEED 6
#define BATTLE_OUTLINE_WIDTH 4

// Colors
#define PM_WHITE CLITERAL(Color){ 244, 254, 250, 255 }
#define PM_DARK CLITERAL(Color){ 43, 41, 44, 255 }
#define PM_LIGHT CLITERAL(Color){ 200, 200, 200, 255 }
#define PM_GRAY CLITERAL(Color){ 58, 55, 59, 255 }
#define PM_LIGHT_GRAY CLITERAL(Color){ 75, 72, 77, 255 }
#define PM_FIRE CLITERAL(Color){ 248, 160, 96, 255 }
#define PM_WATER CLITERAL(Color){ 80, 176, 216, 255 }
#define PM_PLANT CLITERAL(Color){ 100, 169, 144, 255 }

// World Layers (Enum remains good)
typedef enum {
	WORLD_LAYER_WATER,
	WORLD_LAYER_BG,
	WORLD_LAYER_SHADOW,
	WORLD_LAYER_MAIN,
	WORLD_LAYER_TOP
} WorldLayer;

// Battle Positions (Declare as extern)
typedef struct {
	Vector2 top;
	Vector2 center;
	Vector2 bottom;
} BattlePositionsSide;

typedef struct {
	BattlePositionsSide left;
	BattlePositionsSide right;
} BattlePositions;

extern const BattlePositions BATTLE_POSITIONS; // Declaration

// Battle Layers (Enum remains good)
typedef enum {
	BATTLE_LAYER_OUTLINE,
	BATTLE_LAYER_NAME,
	BATTLE_LAYER_MONSTER,
	BATTLE_LAYER_EFFECTS,
	BATTLE_LAYER_OVERLAY
} BattleLayer;

// Battle Choices (Declare as extern)
typedef struct {
	Vector2 pos;
	const char* icon;
} BattleChoiceOption;

typedef struct {
	BattleChoiceOption fight;
	BattleChoiceOption defend;
	BattleChoiceOption switch_option;
	BattleChoiceOption catch;
} BattleChoicesFull;

typedef struct {
	BattleChoiceOption fight;
	BattleChoiceOption defend;
	BattleChoiceOption switch_option;
} BattleChoicesLimited;

typedef struct {
	BattleChoicesFull full;
	BattleChoicesLimited limited;
} BattleChoices;

extern const BattleChoices BATTLE_CHOICES; // Declaration
