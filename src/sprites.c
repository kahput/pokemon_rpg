#include "sprites.h"
#include "components.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SpriteGroup g_groups[SPRITE_GROUP_COUNT];

SpriteID sprite_push(Sprite* sprite, Group id) {
	if (id >= SPRITE_GROUP_COUNT) {
		printf("ERROR: Sprite group ID out of bounds\n");
		exit(1);
	}
	SpriteGroup* group = &g_groups[id];
	if (group->count == group->capacity) {
		group->capacity = group->capacity ? group->capacity * 2 : 32;
		group->sprites = resize_array(group->sprites, group->count, group->capacity, sizeof(Sprite));
	}

	memcpy(group->sprites + group->count, sprite, sizeof(Sprite));

	return group->count++;
}

SpriteID sprite_push_animated(AnimatedSprite* sprite, Group id) {
	if (id >= SPRITE_GROUP_COUNT) {
		printf("ERROR: Sprite group ID out of bounds\n");
		exit(1);
	}
	SpriteGroup* group = &g_groups[id];
	if (group->animated_count == group->animated_capacity) {
		group->animated_capacity = group->animated_capacity ? group->animated_capacity * 2 : 32;
		group->animated_sprites = resize_array(group->animated_sprites, group->animated_count, group->animated_capacity, sizeof(AnimatedSprite));
	}

	memcpy(group->animated_sprites + group->animated_count, sprite, sizeof(AnimatedSprite));

	return group->animated_count++;
}

SpriteGroup* sprite_fetch_group(Group id) {
	if (id >= SPRITE_GROUP_COUNT) {
		printf("ERROR: Sprite group ID out of bounds\n");
		exit(1);
	}
	return &g_groups[id];
}
