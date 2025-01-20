#include "settings.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Battle Positions (Definition and Initialization)
const BattlePositions BATTLE_POSITIONS = {
	.left = {
		.top = { 360, 260 },
		.center = { 190, 400 },
		.bottom = { 410, 520 } },
	.right = { .top = { 900, 260 }, .center = { 1110, 390 }, .bottom = { 900, 550 } }
};

// Battle Choices (Definition and Initialization)
const BattleChoices BATTLE_CHOICES = {
	.full = {
		.fight = { .pos = { 30, -60 }, .icon = "sword" },
		.defend = { .pos = { 40, -20 }, .icon = "shield" },
		.switch_option = { .pos = { 40, 20 }, .icon = "arrows" },
		.catch = { .pos = { 30, 60 }, .icon = "hand" } },
	.limited = { .fight = { .pos = { 30, -40 }, .icon = "sword" }, .defend = { .pos = { 40, 0 }, .icon = "shield" }, .switch_option = { .pos = { 30, 40 }, .icon = "arrows" } }

};

void* resize_array(void* data, uint32_t old_capacity, uint32_t new_capacity, uint32_t element_size) {
	void* tmp = realloc(data, new_capacity * element_size);
	if (!tmp) {
		printf("Realloc failed at %s:%d\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE); // Or handle error
	}
	memset((uint8_t*)tmp + (old_capacity * element_size), 0, (new_capacity - old_capacity) * element_size);
	return tmp;
}
