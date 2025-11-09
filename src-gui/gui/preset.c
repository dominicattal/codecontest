#include "internal.h"
#include <string.h>

void gui_preset_load(GUIComp* root)
{
    const char* string = "The quick brpwn fox jumps over the lazy dog\nThe brown quick xof mpusj vreo het azly gdo";
    GUIComp* comp = gui_comp_create(100, 100, 600, 600);
    gui_comp_set_color(comp, 255, 255, 255, 255);
    gui_comp_copy_text(comp, strlen(string), string);
    gui_comp_set_font_size(comp, 32);
    gui_comp_attach(root, comp);
}
