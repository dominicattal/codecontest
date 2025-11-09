#include "internal.h"
#include "../renderer.h"
#include "../event.h"

GUIContext gui_context;

static void gui_update_comps_helper(GUIComp* comp, f32 dt)
{
    gui_comp_update(comp, dt);
    for (i32 i = 0; i < gui_comp_num_children(comp); i++)
        gui_update_comps_helper(comp->children[i], dt);
}

static void gui_update_comps(f32 dt)
{
    gui_update_comps_helper(gui_context.root, dt);
}

GUIComp* gui_get_event_comp(GUIEventCompEnum type)
{
    return gui_context.event_comps[type];
}

void gui_set_event_comp(GUIEventCompEnum type, GUIComp* comp)
{
    gui_context.event_comps[type] = comp;
}

bool gui_event_comp_equal(GUIEventCompEnum type, GUIComp* comp)
{
    return gui_context.event_comps[type] == comp;
}

void gui_init(void)
{
    gui_render_init();
    gui_comp_init();
    gui_preset_load(gui_context.root);
}

void gui_update(f32 dt)
{
    gui_update_comps(dt);
    gui_update_vertex_data();
    event_queue_flush();
}

void gui_cleanup(void)
{
    gui_comp_cleanup();
    gui_render_cleanup();
}

