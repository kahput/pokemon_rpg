#include "sprites.h"
#include "components.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SpriteGroup g_groups[SPRITE_GROUP_COUNT];

SpriteID add_to_group(Sprite* sprite, Group id) {
	if (id >= SPRITE_GROUP_COUNT) {
		printf("ERROR: Sprite group ID out of bounds\n");
		exit(1);
	}
	SpriteGroup* group = &g_groups[id];
	if (group->capacity == group->count) {
		group->capacity = group->capacity ? group->capacity * 2 : 32;
		group->sprites = resize_array(group->sprites, group->count, group->capacity, sizeof(Sprite));
	}

	memcpy(group->sprites + group->count, sprite, sizeof(Sprite));

	return group->count++;
}

SpriteGroup* get_group(Group id) {
	if (id >= SPRITE_GROUP_COUNT) {
		printf("ERROR: Sprite group ID out of bounds\n");
		exit(1);
	}
	return &g_groups[id];
}
