#include "components.h"
#include "settings.h"
#include "sprites.h"

#include <raylib.h>
#include <raymath.h>
#include <stdint.h>
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

static Tilemap* g_tilemap = NULL;
static SpriteGroup g_all_sprites = { 0 };

void create_tilemap_from_tmx(tmx_map* map, const char* layer_name);
Player create_player_at_marker(tmx_map* map, const char* position);

void handle_input(Player* player, float dt);
void update_positions(SpriteGroup* sprites, Player* player, float dt);

void draw_tilemap(const Tilemap* tilemap);
void draw_sprites(const Sprite* sprites, uint32_t count);

int main(void) {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Monster-gather");

	/* Set the callback globs in the main function */
	tmx_img_load_func = raylib_tex_loader;
	tmx_img_free_func = raylib_free_tex;

	tmx_map* map = tmx_load("assets/data/maps/hospital.tmx");
	create_tilemap_from_tmx(map, "Terrain");
	Player player = create_player_at_marker(map, "world");

	SpriteGroup* all_sprites = get_group(SPRITE_GROUP_ALL);

	while (!WindowShouldClose()) {
		ClearBackground(BLACK);
		BeginDrawing();

		float dt = GetFrameTime();
		handle_input(&player, dt);
		update_positions(all_sprites, &player, dt);

		BeginMode2D(player.camera);

		draw_tilemap(g_tilemap);
		draw_sprites(all_sprites->sprites, all_sprites->count);

		EndMode2D();
		DrawFPS(10, 10);
		EndDrawing();
	}

	tmx_map_free(map);
	CloseWindow();
}

void handle_input(Player* player, float dt) {
	player->direction.x = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
	player->direction.y = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
	player->direction = Vector2Normalize(player->direction);
}

void update_positions(SpriteGroup* sprites, Player* player, float dt) {
	for (int entity = 0; entity < sprites->count; entity++) {
		Sprite* sprite = &sprites->sprites[entity];
		Transform2D* transform = &sprites->sprites[entity].transform;

		if (entity == player->sprite_id) {
			transform->position.x += player->direction.x * player->speed * dt;
			transform->position.y += player->direction.y * player->speed * dt;
			player->camera.target = (Vector2){
				.x = transform->position.x + sprite->rect.width * .5f,
				.y = transform->position.y + sprite->rect.height * .5f,
			};
		}
	}
}

Player create_player_at_marker(tmx_map* map, const char* position) {
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

	Transform2D transform = (Transform2D){
		.position = { player_position_marker->x, player_position_marker->y },
		.scale = { 1.f, 1.f },
		.rotation = 0.f
	};

	Sprite sprite = (Sprite){
		.rect = {
			.x = 0.f,
			.y = 0.f,
			.width = TILE_SIZE,
			.height = TILE_SIZE,
		},
		.transform = transform,
		.texture = NULL,
	};

	Camera2D camera = (Camera2D){
		.target = { .x = player_position_marker->x + (TILE_SIZE * .5f), .y = player_position_marker->y + (TILE_SIZE * .5f) },
		.offset = { .x = WINDOW_WIDTH * .5f, WINDOW_HEIGHT * .5f },
		.rotation = 0.f,
		.zoom = 1.f
	};

	return (Player){
		.camera = camera,
		.direction = { 0, 0 },
		.speed = 250,
		.sprite_id = add_to_group(&sprite, SPRITE_GROUP_ALL)
	};
}

void create_tilemap_from_tmx(tmx_map* map, const char* layer_name) {
	uint32_t offset = 1;
	tmx_layer* layer = map->ly_head;
	tmx_object* object = NULL;
	while (layer) {
		switch (layer->type) {
			case L_LAYER: {
				if (!layer->visible || strcmp(layer->name, "Terrain") != 0)
					break;

				tmx_tileset_list* ts_list = map->ts_head;
				tmx_tileset* ts = map->tiles[layer->content.gids[0]]->tileset;

				while (ts_list) {
					if (strcmp(ts_list->tileset->name, ts->name) == 0)
						break;
					if (ts_list->next == NULL) {
						printf("ERROR: Layer tilset not a part of this map | %s:%d\n", __FILE__, __LINE__);
						exit(1);
					}
					ts_list = ts_list->next;
				}

				Tilemap* tilemap = malloc(sizeof(Tilemap));
				*tilemap = (Tilemap){
					.width = map->width,
					.height = map->height,
					.tile_width = ts->tile_width,
					.tile_height = ts->tile_height,
					.texture = *(Texture2D*)ts->image->resource_image,
					.tile_offset = ts_list->firstgid,
					.tile_count = ts->tilecount,

				};

				printf("TILEMAP_DATA: (%d, %d, %d, %d), offset = %d\n", tilemap->width, tilemap->height, tilemap->tile_width, tilemap->tile_height, tilemap->tile_offset);
				printf("TEUXTRE: %d(%.2d, %.2d)\n", tilemap->texture.id, tilemap->texture.width, tilemap->texture.height);
				tilemap->tiles = malloc(sizeof(uint32_t) * tilemap->width * tilemap->height);

				for (int y = 0; y < tilemap->height; y++) {
					for (int x = 0; x < tilemap->width; x++) {
						int gid = (layer->content.gids[x + (y * map->width)]) & TMX_FLIP_BITS_REMOVAL;
						tilemap->tiles[x + y * tilemap->width] = gid;
					}
				}

				g_tilemap = tilemap;

			} break;
			case L_OBJGR: {
				if (!layer->visible || strcmp(layer->name, "Objects") != 0)
					break;
				object = layer->content.objgr->head;
				printf("OBJ ============== ID: %d, GID: %d, offset: %d\n", object->id, object->content.gid, offset);

				while (object) {
					tmx_tile* tile = map->tiles[object->content.gid];
					Transform2D transform = {
						.position.x = object->x,
						.position.y = object->y - tile->height,
						.scale = { .x = 1.f, .y = 1.f },
						.rotation = object->rotation
					};
					Texture2D* texture = (Texture2D*)tile->image->resource_image;
					Sprite sprite = {
						.rect = {
							.x = tile->ul_x,
							.y = tile->ul_y,
							.width = tile->width,
							.height = tile->height },
						.transform = transform,
						.texture = texture
					};

					printf("TEUXTRE: %d(%.2d, %.2d)\n", texture->id, texture->width, texture->height);
					printf("ID: %d, SPRITE_DATA: (%.2f, %.2f, %.2f, %.2f)\n", tile->id, sprite.rect.x, sprite.rect.y,
						sprite.rect.width, sprite.rect.height);
					add_to_group(&sprite, SPRITE_GROUP_ALL);
					object = object->next;
				}
			} break;
			default: {
			} break;
		}
		layer = layer->next;
	}
}

void draw_tilemap(const Tilemap* tilemap) {
	for (uint32_t y = 0; y < tilemap->height; y++) {
		for (uint32_t x = 0; x < tilemap->width; x++) {
			uint32_t index = x + y * tilemap->width;
			uint32_t gid = tilemap->tiles[index];
			if (gid) {
				Rectangle rect = {
					.x = (int)((gid - tilemap->tile_offset) % 10) * tilemap->tile_width,
					.y = (int)((gid - tilemap->tile_offset) / 10) * tilemap->tile_height,
					.width = tilemap->tile_width,
					.height = tilemap->tile_height
				};

				DrawTextureRec(tilemap->texture, rect, (Vector2){ .x = x * TILE_SIZE, .y = y * TILE_SIZE }, WHITE);
			}
		}
	}
}

void draw_sprites(const Sprite* sprites, uint32_t count) {
	for (int entity = 0; entity < count; entity++) {
		Sprite sprite = sprites[entity];
		Transform2D transform = sprites[entity].transform;

		if (sprite.texture)
			DrawTextureRec(*sprite.texture, sprite.rect, transform.position, WHITE);
		else
			DrawRectangleV(transform.position, (Vector2){ sprite.rect.width, sprite.rect.height }, RED);
	}
}
