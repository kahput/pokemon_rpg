#include "settings.h"

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
