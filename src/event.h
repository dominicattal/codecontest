#ifndef EVENT_H
#define EVENT_H

#include "event.h"
#include "util.h"

// Events provide a thread-safe way of communicating
// between threads. Each thread may enqueue an event in
// any other thread and may flush their own event queue
// when they want.

// Flush all of the events in the calling
// thread's event queue.
void event_queue_flush(void);

// Gui events
void event_create_gui_framebuffer_size_callback(void);
void event_create_gui_cursor_pos_callback(i32 xpos, i32 ypos);
void event_create_gui_mouse_button_callback(i32 button, i32 action, i32 mods);
void event_create_gui_key_callback(i32 key, i32 scancode, i32 action, i32 mods);
void event_create_gui_char_callback(i32 codepoint);

#endif
