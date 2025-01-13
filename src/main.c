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

typedef struct {
	Texture2D* texture;
	Rectangle rect;
	Vector2 position;
} Sprite;

typedef struct {
	Sprite* sprites;
	u32 count;
} SpriteGroup;

SpriteGroup setup(tmx_map* map, const char* player_pos);
void draw_sprites(SpriteGroup* sprites);

int main(void) {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Monster-gather");

	/* Set the callback globs in the main function */
	tmx_img_load_func = raylib_tex_loader;
	tmx_img_free_func = raylib_free_tex;

	tmx_map* map = tmx_load("assets/data/maps/world.tmx");
	SpriteGroup all_sprites = setup(map, "house");

	while (!WindowShouldClose()) {
		ClearBackground(BLACK);
		BeginDrawing();

		draw_sprites(&all_sprites);
		DrawFPS(10, 10);

		EndDrawing();
	}

	tmx_map_free(map);
	CloseWindow();
}

SpriteGroup setup(tmx_map* map, const char* player_pos) {
	tmx_layer* layer = map->ly_head;
	tmx_layer* terrain;
	tmx_object* player_position_markers;
	while (layer) {
		if (strcmp(layer->name, "Entities") == 0) {
			player_position_markers = layer->content.objgr->head;
			while (player_position_markers) {
				tmx_properties* properties = player_position_markers->properties;
				tmx_property* value = tmx_get_property(properties, "pos");
				if (value && strcmp(value->value.string, player_pos) == 0)
					break;

				player_position_markers = player_position_markers->next;
			}
		}
		if (strcmp(layer->name, "Terrain") == 0)
			terrain = layer;
		layer = layer->next;
	}

	printf("Got marker at (%.2f, %.2f)\n", player_position_markers->x, player_position_markers->y);

	Sprite* sprites = malloc(sizeof(Sprite) * map->width * map->height);
	u32 count = 0;
	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			u32 gid = (terrain->content.gids[x + (y * map->width)]) & TMX_FLIP_BITS_REMOVAL;
			if (!map->tiles[gid])
				continue;
			tmx_tileset* tileset = map->tiles[gid]->tileset;
			tmx_image* im = map->tiles[gid]->image;
			void* image;
			if (im) {
				image = im->resource_image;
			} else {
				image = tileset->image->resource_image;
			}
			Rectangle rec = {
				.x = map->tiles[gid]->ul_x,
				.y = map->tiles[gid]->ul_y,
				.width = tileset->tile_width,
				.height = tileset->tile_height,
			};
			sprites[count++] = (Sprite){
				.position = { .x = x * TILE_SIZE, .y = y * TILE_SIZE },
				.rect = rec,
				.texture = (Texture2D*)image
			};
		}
	}

	printf("Sprite[0]: (%.2f, %.2f)\n", sprites[0].position.x, sprites[0].position.y);

	return (SpriteGroup){
		.sprites = sprites,
		.count = count
	};
}

void draw_sprites(SpriteGroup* sprites) {
	for (int i = 0; i < sprites->count; i++)
		DrawTextureRec(*(sprites->sprites[i].texture), sprites->sprites[i].rect, sprites->sprites[i].position, WHITE);
}
