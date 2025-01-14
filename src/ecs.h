#pragma once

#include "settings.h"

#include <assert.h>

#define MAX_COMPONENTS 32
#define MAX_SYSTEMS 8

typedef u32 Entity;
typedef struct {
	Entity* entities;
	u32 count;
} View;
typedef void (*SystemPointer)(View*, f32);

void ecs_startup(u32 component_count, ...);
void ecs_clear();
void ecs_shutdown();

Entity ecs_create_entity();
void ecs_destroy_entity(Entity entity);
bool ecs_validate_entity(Entity entity);

void ecs_attach_component(Entity entity, u32 component_id, void* data);
void ecs_detach_component(Entity entity, u32 component_id);
void* ecs_fetch_component(Entity entity, u32 component_id);
bool ecs_has_component(Entity entity, u32 component_id);

void* ecs_fetch_component_array(u32 component_id);

void ecs_attach_system(SystemPointer system_ptr, u32 component_count, ...);

void ecs_update(f64 dt);
