#include "ecs.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define ENTITY_INDEX_BITS 24
#define ENTITY_GENERATION_BITS 8
#define ENTITY_INDEX_MASK ((1 << ENTITY_INDEX_BITS) - 1)
#define ENTITY_GENERATION_MASK ((1 << ENTITY_GENERATION_BITS) - 1)

typedef struct {
	void* data;
	size_t element_size;
	u32 count, capacity;
} ComponentArray;

typedef struct {
	SystemPointer ptr;
	u32* entities;
	u32 signature, component_count, entity_count, entity_capcity;
} System;

typedef struct {
	ComponentArray components[MAX_COMPONENTS];
	System systems[MAX_SYSTEMS];
	ComponentView view;
	u32 component_count, system_count;
	u32 *entity_signatures, *entity_to_index, *index_to_entity, *entity_free_ids;
	u32 entity_count, entity_capcity, entity_free_ids_count, systems_entity_capacity;
} ECS;

static ECS g_world = { 0 };

// Helper functions
u32 get_entity_id(Entity entity) {
	return entity & ENTITY_INDEX_MASK;
}

u8 get_entity_generation(Entity entity) {
	return (entity >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK;
}

void system_add_entity(Entity entity) {
	for (int sys = 0; sys < g_world.system_count; sys++) {
		System* system = &g_world.systems[sys];
		u32 index = g_world.entity_to_index[get_entity_id(entity)];

		if ((g_world.entity_signatures[index] & system->signature) == system->signature) {
			if (system->entity_count == system->entity_capcity) {
				system->entity_capcity = system->entity_capcity ? system->entity_capcity * 2 : 32;
				u32* tmp = realloc(system->entities, system->entity_capcity * sizeof(u32));
				if (!tmp) {
					printf("Realloc failed for system->entites\n");
					exit(EXIT_FAILURE);
				}
				system->entities = tmp;

				if (g_world.systems_entity_capacity < system->entity_capcity) {
					for (int comp = 0; comp < g_world.component_count; comp++) {
						g_world.systems_entity_capacity = system->entity_capcity;
						void* tmp = realloc(g_world.view.data[comp], g_world.systems_entity_capacity * g_world.components[comp].element_size);
						if (!tmp) {
							printf("Realloc failed for view\n");
							exit(EXIT_FAILURE);
						}
						g_world.view.data[comp] = tmp;
					}

					printf("INFO: SYSTEM: System view capcity = %d\n", g_world.systems_entity_capacity);
				}
			}

			system->entities[system->entity_count++] = entity;
			printf("INFO: SYSTEM: [ID %d] added Entity[ID %d] at index [%d]\n", sys, get_entity_id(system->entities[system->entity_count - 1]), system->entity_count - 1);
		}
	}
}

void system_remove_entity(Entity entity) {
	for (int sys = 0; sys < g_world.system_count; sys++) {
		System* system = &g_world.systems[sys];
		u32 index = g_world.entity_to_index[get_entity_id(entity)];

		for (int index = 0; index < system->entity_count; index++) {
			if (system->entities[index] == entity) {
				u32 last_index = system->entity_count - 1;
				if (index != last_index)
					system->entities[index] = system->entities[last_index];

				system->entity_count--;
				printf("INFO: SYSTEM: [ID %d] removed Entity[ID %d] at index [%d]\n", sys, get_entity_id(entity), index);
			}
		}
	}
}

void ecs_startup(u32 component_count, ...) {
	assert(component_count <= MAX_COMPONENTS);
	g_world.component_count = component_count;

	va_list args;
	va_start(args, component_count);
	for (int i = 0; i < component_count; i++) {
		g_world.components[i].element_size = va_arg(args, size_t);
		printf("INFO: COMPONENT: [ID %d | %zuB] Component defined.\n", i, g_world.components[i].element_size);
	}
	va_end(args);
}

void ecs_clear() {
	while (g_world.entity_count > 0)
		ecs_destroy_entity(g_world.index_to_entity[g_world.entity_count - 1]);
}

void ecs_shutdown() {
	// Free entites
	free(g_world.entity_signatures);
	free(g_world.index_to_entity);
	free(g_world.entity_to_index);
	free(g_world.entity_free_ids);
	printf("INFO: ECS shutdown successfullly\n");

	// Free component data
	for (int i = 0; i < g_world.component_count; i++) {
		free(g_world.components[i].data);
		free(g_world.view.data[i]);
	}

	// Free systems
	for (int i = 0; i < g_world.system_count; i++)
		free(g_world.systems[i].entities);
}

// ECS managment
Entity ecs_create_entity() {
	if (g_world.entity_count == g_world.entity_capcity) {
		g_world.entity_capcity = g_world.entity_capcity ? g_world.entity_capcity * 2 : 32;
		u32* tmp = realloc(g_world.entity_signatures, g_world.entity_capcity * sizeof(u32));
		if (!tmp) {
			printf("Realloc failed for entity_signatures\n");
			exit(EXIT_FAILURE); // Or handle error
		}
		g_world.entity_signatures = tmp;

		tmp = realloc(g_world.index_to_entity, g_world.entity_capcity * sizeof(u32));
		if (!tmp) {
			printf("Realloc failed for index_to_entity\n");
			exit(EXIT_FAILURE); // Or handle error
		}
		g_world.index_to_entity = tmp;

		tmp = realloc(g_world.entity_to_index, g_world.entity_capcity * sizeof(u32));
		if (!tmp) {
			printf("Realloc failed for entity_to_index\n");
			exit(EXIT_FAILURE); // Or handle error
		}
		g_world.entity_to_index = tmp;

		tmp = realloc(g_world.entity_free_ids, g_world.entity_capcity * sizeof(u32));
		if (!tmp) {
			printf("Realloc failed for entity_free_ids\n");
			exit(EXIT_FAILURE); // Or handle error
		}
		g_world.entity_free_ids = tmp;
		memset(g_world.index_to_entity + g_world.entity_count, 0, (g_world.entity_capcity - g_world.entity_count) * sizeof(u32));

		printf("INFO: ENTITY: Capacity resized to %d.\n", g_world.entity_capcity);
	}

	u32 entity_id, index;
	if (g_world.entity_free_ids_count > 0) {
		entity_id = g_world.entity_free_ids[--g_world.entity_free_ids_count];
		index = g_world.entity_count++;
	} else {
		entity_id = index = g_world.entity_count++;
	}

	u8 entity_generation = get_entity_generation(g_world.index_to_entity[index]);
	if (entity_generation == 0)
		entity_generation = 1;

	g_world.entity_signatures[index] = 0;
	g_world.index_to_entity[index] = entity_id | (entity_generation << ENTITY_INDEX_BITS);
	g_world.entity_to_index[entity_id] = index;
	printf("INFO: ENTITY: [ID %d | IDX %d | GEN %d] Entity created\n", entity_id, index, entity_generation);
	printf("INFO: ENTITY: TOTAL = %d, CAPACITY = %d\n", g_world.entity_count, g_world.entity_capcity);
	system_add_entity(g_world.index_to_entity[index]);
	return g_world.index_to_entity[index];
}

void ecs_destroy_entity(Entity entity) {
	if (!ecs_validate_entity(entity)) {
		printf("WARNING: ENTITY: [ID %d] Invalid entity in ecs_destroy_entity()\n", get_entity_id(entity));
		return;
	}
	u32 deleted_entity_id = get_entity_id(entity);
	u32 deleted_entity_index = g_world.entity_to_index[deleted_entity_id];
	u32 deleted_entity_generation = get_entity_generation(entity);
	u32 last_index = --g_world.entity_count;

	system_remove_entity(entity);

	printf("INFO: ENTITY: [ID %d | IDX %d | GEN %d] Entity marked for deletion\n", deleted_entity_id, deleted_entity_index, deleted_entity_generation);

	if (deleted_entity_index != last_index) {
		for (u32 comp = 0; comp < g_world.component_count; comp++) {
			size_t element_size = g_world.components[comp].element_size;
			void* component_data = g_world.components[comp].data;

			void* dst = (u8*)component_data + deleted_entity_index * element_size;
			void* src = (u8*)component_data + last_index * element_size;
			memcpy(dst, src, element_size);
		}

		Entity moved_entity = g_world.index_to_entity[last_index];
		u32 moved_entity_id = get_entity_id(moved_entity);
		u32 moved_entity_generation = get_entity_generation(moved_entity);
		g_world.entity_to_index[moved_entity_id] = deleted_entity_index;
		g_world.index_to_entity[deleted_entity_index] = moved_entity;
		g_world.entity_signatures[deleted_entity_index] = g_world.entity_signatures[last_index];
		printf("INFO: ENTITY: [ID %d | IDX %d | GEN %d] Entity moved to index %d\n",
				moved_entity_id,
				last_index,
				moved_entity_generation,
				g_world.entity_to_index[moved_entity_id]

		);
	}

	g_world.index_to_entity[last_index] = ENTITY_INDEX_MASK | (++deleted_entity_generation << ENTITY_INDEX_BITS);
	g_world.entity_to_index[deleted_entity_id] = UINT32_MAX;
	g_world.entity_signatures[last_index] = 0;
	g_world.entity_free_ids[g_world.entity_free_ids_count++] = deleted_entity_id;
	printf("INFO: ENTITY: [ID %d | IDX %d | GEN %d] Entity deleted\n", deleted_entity_id, deleted_entity_index, deleted_entity_generation);
}

bool ecs_validate_entity(Entity entity) {
	u32 entity_id = get_entity_id(entity);
	u8 entity_generation = get_entity_generation(entity);
	u32 entity_index = g_world.entity_to_index[entity_id];

	if (entity_index >= g_world.entity_count)
		return false;
	if (entity_generation != get_entity_generation(g_world.index_to_entity[entity_index]))
		return false;

	return true;
}

void ecs_attach_component(Entity entity, u32 component_id, void* data) {
	assert(component_id < g_world.component_count);
	if (!ecs_validate_entity(entity)) {
		printf("WARNING: ENTITY: [ID %d] Invalid entity in ecs_attach_component()\n", get_entity_id(entity));
		return;
	}
	u32 entity_id = get_entity_id(entity);
	u32 index = g_world.entity_to_index[entity_id];

	if ((g_world.entity_signatures[index] & (1 << component_id)) == (1 << component_id)) {
		printf("WARNING: ENTITY: [ID %d | %zuB] Component already added to entity [ID %d]\n", component_id, g_world.components[component_id].element_size, index);
		return;
	}

	ComponentArray* array = &g_world.components[component_id];
	size_t element_size = g_world.components[component_id].element_size;

	if (array->count >= array->capacity) {
		array->capacity = array->capacity ? array->capacity * 2 : 32;
		array->data = realloc(array->data, array->capacity * element_size);

		printf("INFO: COMPONENT: [IDX %d] Component array resized to %d\n", component_id, array->capacity);
	}

	g_world.entity_signatures[index] |= 1 << component_id;
	memcpy((u8*)array->data + index * element_size, data, element_size);

	printf("INFO: COMPONENT: [ID %d | %zuB] Component added to entity [ID %d] at index %d\n", component_id, element_size, entity_id, index);
	array->count++;

	system_remove_entity(entity);
	system_add_entity(entity);
}

void ecs_detach_component(Entity entity, u32 component_id) {
	assert(component_id < g_world.component_count);
	if (!ecs_validate_entity(entity)) {
		printf("WARNING: ENTITY: [ID %d] Invalid entity in ecs_detach_component()\n", get_entity_id(entity));
		return;
	}
	u32 entity_id = get_entity_id(entity);
	u32 index = g_world.entity_to_index[entity_id];
	assert(index < g_world.entity_count);
	assert(component_id < g_world.component_count);
	ComponentArray* array = &g_world.components[component_id];
	array->count--;

	g_world.entity_signatures[index] &= ~(1 << component_id);

	system_remove_entity(entity);
	system_add_entity(entity);
}

void* ecs_fetch_component(Entity entity, u32 component_id) {
	assert(component_id < g_world.component_count);
	if (!ecs_validate_entity(entity)) {
		printf("WARNING: ENTITY: [ID %d] Invalid entity in ecs_fetch_component()\n", get_entity_id(entity));
		return NULL;
	}
	u32 entity_id = get_entity_id(entity);
	u32 index = g_world.entity_to_index[entity_id];
	assert(index < g_world.entity_count);
	assert(component_id < g_world.component_count);
	assert((g_world.entity_signatures[index] & (1 << component_id)) == (1 << component_id) && "Entity does not have component");
	ComponentArray* array = &g_world.components[component_id];
	size_t element_size = g_world.components[component_id].element_size;

	return ((u8*)array->data + index * element_size);
	return NULL;
}

bool ecs_has_component(Entity entity, u32 component_id) {
	u32 index = g_world.entity_to_index[get_entity_id(entity)];
	return (g_world.entity_signatures[index] & (1 << component_id)) == (1 << component_id);
}

void ecs_attach_system(SystemPointer system_ptr, u32 component_count, ...) {
	assert(component_count <= MAX_COMPONENTS && g_world.system_count < MAX_SYSTEMS);
	System* system = &g_world.systems[g_world.system_count++];

	va_list args;
	va_start(args, component_count);
	for (int i = 0; i < component_count; i++) {
		system->signature |= 1 << va_arg(args, u32);
	}
	system->ptr = system_ptr;
	system->component_count = component_count;
	va_end(args);

	system->entity_count = system->entity_capcity = 0;
	u32 relevant_indices[g_world.entity_count];
	for (int index = 0; index < g_world.entity_count; index++) {
		if ((g_world.entity_signatures[index] & system->signature) == system->signature) {
			relevant_indices[system->entity_count++] = index;
		}
	}

	system->entities = malloc(system->entity_count * sizeof(u32));
	system->entity_capcity = system->entity_count;
	for (int index = 0; index < system->entity_capcity; index++) {
		system->entities[index] = g_world.index_to_entity[relevant_indices[index]];
	}

	printf("INFO: SYSTEM: [IDX %d | SIG %d] System added\n", (g_world.system_count - 1), system->signature);
}

void* ecs_view_fetch(ComponentView* view, u32 component_id) {
	return view->data[component_id];
}

void ecs_update(f64 dt) {
	for (int sys = 0; sys < g_world.system_count; sys++) {
		System* system = &g_world.systems[sys];

		ComponentView* view = &g_world.view;
		view->count = system->entity_count;

		for (int index = 0; index < system->entity_count; index++) {
			u32 entity_component_index = g_world.entity_to_index[get_entity_id(system->entities[index])];
			for (int comp = 0; comp < g_world.component_count; comp++) {
				u32 component_id = system->signature >> comp & 1;
				if (!component_id)
					continue;
				ComponentArray* component = &g_world.components[comp];
				void* dst = (u8*)view->data[comp] + index * component->element_size;
				void* src = (u8*)component->data + entity_component_index * component->element_size;
				memcpy(dst, src, component->element_size);
			}
		}

		system->ptr(view, dt);

		for (int index = 0; index < system->entity_count; index++) {
			u32 entity_component_index = g_world.entity_to_index[get_entity_id(system->entities[index])];
			for (int comp = 0; comp < g_world.component_count; comp++) {
				u32 component_id = system->signature >> comp & 1;
				if (!component_id)
					continue;
				ComponentArray* component = &g_world.components[comp];
				void* src = (u8*)view->data[comp] + index * component->element_size;
				void* dst = (u8*)component->data + entity_component_index * component->element_size;
				memcpy(dst, src, component->element_size);
			}
		}

		view->count = 0;
	}
}
