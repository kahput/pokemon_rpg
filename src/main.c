#include "ecs.h"
#include "settings.h"

#include <raylib.h>
#include <stdlib.h>
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
	Texture2D image;
	Rectangle rect;
	Vector2 position;
} Drawable;

void setup(tmx_map* map);
void draw_tileset(ComponentView* view, f32 dt);

int main(void) {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Monster-gather");
	ecs_startup(1, sizeof(Drawable));
	ecs_attach_system(draw_tileset, 1, 0);

	/* Set the callback globs in the main function */
	tmx_img_load_func = raylib_tex_loader;
	tmx_img_free_func = raylib_free_tex;

	tmx_map* map = tmx_load("assets/data/maps/world.tmx");
	setup(map);

	while (!WindowShouldClose()) {
		ClearBackground(BLACK);
		BeginDrawing();

		ecs_update(GetFrameTime());

		EndDrawing();
	}
}
void draw_tileset(ComponentView* view, f32 dt) {
	Drawable* drawables = ecs_view_fetch(view, 0);

	for (u32 entity = 0; entity < view->count; entity++) {
		Drawable drawable = drawables[entity];

		DrawTextureRec(drawables->image, drawable.rect, drawable.position, WHITE);
	}
}

void setup(tmx_map* map) {
	tmx_layer* layer = map->ly_head;

	for (int y = 0; y < map->height; y++) {
		for (int x = 0; x < map->width; x++) {
			u32 gid = (layer->content.gids[x + (y * map->width)]) & TMX_FLIP_BITS_REMOVAL;
			if (!map->tiles[gid])
				continue;
			tmx_tileset* tileset = map->tiles[gid]->tileset;
			Rectangle rec = {
				.x = map->tiles[gid]->ul_x,
				.y = map->tiles[gid]->ul_y,
				.width = tileset->tile_width,
				.height = tileset->tile_height,
			};
			tmx_image* im = map->tiles[gid]->image;
			void* image;
			if (im) {
				image = im->resource_image;
			} else {
				image = tileset->image->resource_image;
			}
			Entity tile = ecs_create_entity();
			ecs_attach_component(tile, 0, &(Drawable){ .image = *(Texture2D*)image, .rect = rec, .position = { .x = x * TILE_SIZE, y * TILE_SIZE } });
		}
	}
}
