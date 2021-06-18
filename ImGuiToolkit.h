#ifndef __IMGUI_TOOLKIT_H_
#define __IMGUI_TOOLKIT_H_

#include <glib.h>
#include <string>
#include <list>
#include <vector>

#include "rsc/fonts/IconsFontAwesome5.h"

namespace ImGuiToolkit
{
    // Icons from resource icon.dds
    void Icon (int i, int j, bool enabled = true);
    bool IconButton (int i, int j, const char *tooltips = nullptr);
    bool IconButton (const char* icon, const char *tooltips = nullptr);
    bool IconToggle (int i, int j, int i_toggle, int j_toggle, bool* toggle, const char *tooltips[] = nullptr);
    void ShowIconsWindow(bool* p_open);

    // icon buttons
    bool ButtonIcon (int i, int j, const char* tooltip = nullptr);
    bool ButtonIconToggle (int i, int j, int i_toggle, int j_toggle, bool* toggle);
    bool ButtonIconMultistate (std::vector<std::pair<int, int> > icons, int* state);
    bool ComboIcon (std::vector<std::pair<int, int> > icons, std::vector<std::string> labels, int* state);

    // buttons
    bool ButtonToggle  (const char* label, bool* toggle);
    bool ButtonSwitch  (const char* label, bool* toggle , const char *help = nullptr);
    void ButtonOpenUrl (const char* label, const char* url, const ImVec2& size_arg = ImVec2(0,0));

    // tooltip and mouse over
    void ToolTip    (const char* desc, const char* shortcut = nullptr);
    void HelpMarker (const char* desc, const char* icon = ICON_FA_QUESTION_CIRCLE, const char* shortcut = nullptr);
    void HelpIcon   (const char* desc, int i = 19, int j = 5, const char* shortcut = nullptr);

    // sliders
    bool TimelineSlider (const char* label, guint64 *time, guint64 start, guint64 end, guint64 step, const float width);
    void RenderTimeline (struct ImGuiWindow* window, struct ImRect timeline_bbox, guint64 start, guint64 end, guint64 step, bool verticalflip = false);
    void Timeline (const char* label, guint64 time, guint64 start, guint64 end, guint64 step, const float width);
    bool InvisibleSliderInt(const char* label, uint *index, uint min, uint max, const ImVec2 size);
    bool EditPlotLines(const char* label, float *array, int values_count, float values_min, float values_max, const ImVec2 size);
    bool EditPlotHistoLines(const char* label, float *histogram_array, float *lines_array, int values_count, float values_min, float values_max, guint64 start, guint64 end, bool cut, bool *released, const ImVec2 size);
    void ShowPlotHistoLines(const char* label, float *histogram_array, float *lines_array, int values_count, float values_min, float values_max, const ImVec2 size);

    // fonts from ressources 'fonts/'
    typedef enum {
        FONT_DEFAULT =0,
        FONT_BOLD,
        FONT_ITALIC,
        FONT_MONO,
        FONT_LARGE
    } font_style;
    void SetFont (font_style type, const std::string &ttf_font_name, int pointsize, int oversample = 2);
    void PushFont (font_style type);

    // text input
    bool InputText(const char* label, std::string* str);
    bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), int linesize = 0);

    // accent color of UI
    typedef enum {
        ACCENT_BLUE =0,
        ACCENT_ORANGE,
        ACCENT_GREY
    } accent_color;
    void SetAccentColor (accent_color color);
    struct ImVec4 HighlightColor (bool active = true);

    // varia
    void WindowText(const char* window_name, ImVec2 window_pos, const char* text);
    bool WindowButton(const char* window_name, ImVec2 window_pos, const char* text);
    void WindowDragFloat(const char* window_name, ImVec2 window_pos, float* v, float v_speed, float v_min, float v_max, const char* format);

}

#endif // __IMGUI_TOOLKIT_H_
