#include "internal.h"
#include <string.h>

void gui_preset_load(GUIComp* root)
{
    const char* string = "The quick broown fox jumps over the lazy dog";
    GUIComp* comp = gui_comp_create(100, 100, 300, 50);
    gui_comp_set_color(comp, 255, 255, 255, 255);
    gui_comp_copy_text(comp, strlen(string), string);
    gui_comp_set_font_size(comp, 16);
    gui_comp_attach(root, comp);
}
