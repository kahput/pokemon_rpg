#pragma once

#include "components.h"
#include <stdint.h>

typedef uint32_t SpriteID;

typedef enum {
	SPRITE_GROUP_ALL,

	SPRITE_GROUP_COUNT
} Group;

SpriteID add_to_group(Sprite* sprite, Group id);
SpriteGroup* get_group(Group id);
