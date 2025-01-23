#pragma once

#include "components.h"
#include <stdint.h>

typedef uint32_t SpriteID;

typedef enum {
	SPRITE_GROUP_ALL,

	SPRITE_GROUP_COUNT
} Group;

SpriteID sprite_push(Sprite* sprite, Group id);
SpriteID sprite_push_animated(AnimatedSprite* sprite, Group id);
SpriteGroup* sprite_fetch_group(Group id);
