#pragma once

#include "settings.h"

#include <assert.h>

#define MAX_COMPONENTS 32
#define MAX_SYSTEMS 8

typedef struct {
	void* data[MAX_COMPONENTS];
	u32 count;
} ComponentView;
typedef u32 Entity;
typedef void (*SystemPointer)(ComponentView*, f32);

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

void* ecs_view_fetch(ComponentView* view, u32 component_id);

void ecs_attach_system(SystemPointer system_ptr, u32 component_count, ...);

void ecs_update(f64 dt);
