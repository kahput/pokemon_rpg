#pragma once
#include <raylib.h>
#include <stdint.h>

typedef struct {
	Vector2 position, scale;
	float rotation;
} Transform2D;

typedef struct {
	Texture2D* texture;
	Rectangle rect;
	Transform2D transform;
} Sprite;

typedef struct {
	Sprite* sprites;
	uint32_t count, capacity;
} SpriteGroup;

typedef struct {
	Texture2D texture;
	uint32_t* tiles;
	uint32_t width, height, tile_width, tile_height;
	uint32_t tile_count, tile_offset;
} Tilemap;

typedef struct {
	Camera2D camera;
	Vector2 direction;
	uint32_t speed;
	uint32_t sprite_id;
} Player;
