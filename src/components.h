#pragma once
#include "settings.h"
#include <raylib.h>

typedef enum {
	COMPONENT_SPRITE,
	COMPONENT_TILEMAP,

	COMPONENT_COUNT,
} ComponentID;

typedef struct {
	Texture2D* texture;
	Rectangle rect;
	Vector2 position;
} Sprite;

typedef struct {
	Texture2D texture;
	u32* tiles;
	u32 width, height, tile_width, tile_height;
	u32 tile_count, tile_offset;
} Tilemap;
