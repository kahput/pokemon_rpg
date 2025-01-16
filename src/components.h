#pragma once
#include "settings.h"
#include <raylib.h>

typedef enum {
	COMPONENT_SPRITE,
	COMPONENT_TILEMAP,
	COMPONENT_TRANSFORM,
	COMPONENT_PLAYER,
	COMPONENT_CAMERA,

	COMPONENT_COUNT,
} ComponentID;

typedef struct {
	Texture2D* texture;
	Rectangle rect;
} Sprite;

typedef struct {
	Texture2D texture;
	u32* tiles;
	u32 width, height, tile_width, tile_height;
	u32 tile_count, tile_offset;
} Tilemap;

typedef struct {
	Vector2 position, scale;
	f32 rotation;
} Transform2D;

typedef struct {
	Vector2 direction;
	u32 speed;
} Player;
