#include "state.h"
#include "window.h"
#include "renderer.h"
#include "gui.h"
#include <stdio.h>
#include <pthread.h>
#include <json.h>

#ifdef DEBUG_BUILD
    #define BUILD_INFO "DEBUG"
#elif RELEASE_BUILD
    #define BUILD_INFO "RELEASE"
#endif

struct {
    HMODULE lib;
    f32 dt;
} state_context;

void state_init(void)
{
    thread_link("Main");
    window_init();
    renderer_init();
    gui_init();
}

void state_loop(void)
{
    f32 next, last, dt;
    last = get_time();
    dt = 0;
    while (!window_closed())
    {
        window_update();
        gui_update(dt);
        gui_render();
        next = get_time();
        dt = next - last;
        last = next;
    }
}

void state_cleanup(void)
{
    log_unlock();
    gui_cleanup();
    renderer_cleanup();
    window_cleanup();
#ifdef DEBUG_BUILD
    print_heap_info();
#endif
    log_cleanup();
}
