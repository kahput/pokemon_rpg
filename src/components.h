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
	Rectangle rect;
	Transform2D transform;
	uint32_t count,  frame_index;
	Texture2D** textures;
} AnimatedSprite;

typedef struct {
	Sprite* sprites;
	uint32_t count, capacity;
	AnimatedSprite* animated_sprites;
	uint32_t animated_count, animated_capacity;
} SpriteGroup;

typedef struct {
	Texture2D texture;
	uint32_t* tiles;
	uint32_t width, height, tile_width, tile_height;
	uint32_t tile_offset;
} Tilemap;

typedef struct {
	Camera2D camera;
	Vector2 direction;
	uint32_t speed;
	uint32_t sprite_id;
} Player;
