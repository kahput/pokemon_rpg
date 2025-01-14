#include "components.h"
#include "ecs.h"
#include "settings.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tmx.h>

void* raylib_tex_loader(const char* path) {
	Texture2D* returnValue = malloc(sizeof(Texture2D));
	*returnValue = LoadTexture(path);
	return returnValue;
}

void raylib_free_tex(void* ptr) {
	UnloadTexture(*((Texture2D*)ptr));
	free(ptr);
}

Entity create_tilemap_from_tmx(tmx_map* map, const char* layer_name);
Entity create_player_at_marker(tmx_map* map, const char* position);
void draw_tilemap(ComponentView* view, f32 dt);
void draw_sprites(ComponentView* view, f32 dt);

int main(void) {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Monster-gather");
	ecs_startup(2, sizeof(Sprite), sizeof(Tilemap));

	ecs_attach_system(draw_tilemap, 1, COMPONENT_TILEMAP);
	ecs_attach_system(draw_sprites, 1, COMPONENT_SPRITE);

	/* Set the callback globs in the main function */
	tmx_img_load_func = raylib_tex_loader;
	tmx_img_free_func = raylib_free_tex;

	tmx_map* map = tmx_load("assets/data/maps/world.tmx");
	Entity tilemap = create_tilemap_from_tmx(map, "Terrain");
	Entity player = create_player_at_marker(map, "house");

	while (!WindowShouldClose()) {
		ClearBackground(BLACK);
		BeginDrawing();

		ecs_update(GetFrameTime());
		DrawFPS(10, 10);

		EndDrawing();
	}

	ecs_shutdown();
	tmx_map_free(map);
	CloseWindow();
}

Entity create_player_at_marker(tmx_map* map, const char* position) {
	tmx_layer* layer = map->ly_head;
	tmx_object* player_position_marker = NULL;
	while (layer) {
		if (strcmp(layer->name, "Entities") == 0) {
			player_position_marker = layer->content.objgr->head;
			while (player_position_marker) {
				tmx_properties* properties = player_position_marker->properties;
				if (properties) {
					tmx_property* value = tmx_get_property(properties, "pos");
					if (value && strcmp(value->value.string, position) == 0)
						break;
				}

				player_position_marker = player_position_marker->next;
			}
		}
		layer = layer->next;
	}

	Entity player = ecs_create_entity();
	ecs_attach_component(
		player,
		COMPONENT_SPRITE,
		&(Sprite){
			.position = {
				.x = player_position_marker->x,
				.y = player_position_marker->y,
			},
			.rect = {
				.x = player_position_marker->x - TILE_SIZE * .5f,
				.y = player_position_marker->y - TILE_SIZE * .5f,
				.width = TILE_SIZE,
				.height = TILE_SIZE,
			},
			.texture = NULL,
		});
	return player;
}

Entity create_tilemap_from_tmx(tmx_map* map, const char* layer_name) {
	tmx_layer* layer = map->ly_head;
	while (layer) {
		if (strcmp(layer->name, layer_name) == 0)
			break;
		layer = layer->next;
	}

	Entity tilemap_entity = ecs_create_entity();
	Tilemap tilemap = { 0 };
	tilemap.width = map->width, tilemap.height = map->height;
	tilemap.tile_width = map->tile_width, tilemap.tile_height = map->tile_height;
	tilemap.texture = *(Texture2D*)map->tiles[1]->tileset->image->resource_image;
	tilemap.tile_offset = 1, tilemap.tile_count = map->tiles[1]->tileset->tilecount;
	tilemap.tiles = malloc(sizeof(u32) * tilemap.width * tilemap.height);

	for (int y = 0; y < tilemap.width; y++) {
		for (int x = 0; x < tilemap.height; x++) {
			int gid = (layer->content.gids[x + (y * map->width)]) & TMX_FLIP_BITS_REMOVAL;
			tilemap.tiles[x + y * tilemap.width] = gid;
		}
	}

	ecs_attach_component(tilemap_entity, COMPONENT_TILEMAP, &tilemap);
	return tilemap_entity;
}

void draw_tilemap(ComponentView* view, f32 dt) {
	Tilemap* tilemap = (Tilemap*)ecs_view_fetch(view, COMPONENT_TILEMAP);
	for (int entity = 0; entity < view->count; entity++) {
		for (u32 y = 0; y < tilemap[entity].height; y++) {
			for (u32 x = 0; x < tilemap[entity].height; x++) {
				u32 index = x + y * tilemap[entity].width;
				u32 gid = tilemap->tiles[index];
				Rectangle rect = {
					.x = (int)((gid - tilemap[entity].tile_offset) % 10) * tilemap->tile_width,
					.y = (int)((gid - tilemap[entity].tile_offset) / 10) * tilemap->tile_height,
					.width = tilemap->tile_width,
					.height = tilemap->tile_height
				};

				DrawTextureRec(tilemap[entity].texture, rect, (Vector2){ .x = x * TILE_SIZE, .y = y * TILE_SIZE }, WHITE);
			}
		}
	}
}

void draw_sprites(ComponentView* view, f32 dt) {
	Sprite* sprites = (Sprite*)ecs_view_fetch(view, COMPONENT_SPRITE);

	for (int entity = 0; entity < view->count; entity++) {
		Sprite sprite = sprites[entity];

		DrawRectangleRec(sprite.rect, RED);
	}
}
