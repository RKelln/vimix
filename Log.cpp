#include "Log.h"

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include "ImGuiToolkit.h"
#include "defines.h"

// multiplatform
#include <tinyfiledialogs.h>

#include <string>
#include <list>
#include <mutex>
using namespace std;

static std::mutex mtx;

struct AppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; 

    AppLog()
    {
        Clear();
    }

    void Clear()
    {
        mtx.lock();
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
        mtx.unlock();
    }

    void AddLog(const char* fmt, va_list args)
    {
        mtx.lock();
        int old_size = Buf.size();
        Buf.appendfv(fmt, args);
        Buf.append("\n");

        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
        mtx.unlock();
    }

    void Draw(const char* title, bool* p_open = NULL)
    {

        ImGui::SetNextWindowPos(ImVec2(430, 660), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1150, 220), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(600, 180), ImVec2(FLT_MAX, FLT_MAX));
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        //  window
        ImGui::SameLine(0, 0);
        bool clear = ImGui::Button( ICON_FA_BACKSPACE " Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button( ICON_FA_COPY " Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -60.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear)
            Clear();
        if (copy)
            ImGui::LogToClipboard();

        ImGuiToolkit::PushFont(ImGuiToolkit::FONT_MONO);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        mtx.lock();

        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
            // especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
            // Here we instead demonstrate using the clipper to only process lines that are within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
            // Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
            // Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }

        mtx.unlock();

        ImGui::PopStyleVar();
        ImGui::PopFont();

        // Auto scroll
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};

static AppLog logs;

void Log::Info(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logs.AddLog(fmt, args);
    va_end(args);
}

void Log::ShowLogWindow(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    logs.Draw( ICON_FA_LIST_UL " Logs", p_open);
}

static list<string> notifications;
static float notifications_timeout = 0.f;

void Log::Notify(const char* fmt, ...)
{
    ImGuiTextBuffer buf;

    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);

    // will display a notification
    notifications.push_back(buf.c_str());
    notifications_timeout = 0.f;

    // always log
    Log::Info("%s\n", buf.c_str());
}


static list<string> warnings;

void Log::Warning(const char* fmt, ...)
{
    ImGuiTextBuffer buf;

    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);

    // will display a warning dialog
    warnings.push_back(buf.c_str());

    // always log
    Log::Info("Warning - %s\n", buf.c_str());
}

void Log::Render(bool showNofitications, bool showWarnings)
{
    bool show_warnings = !warnings.empty() & showWarnings;
    bool show_notification = !notifications.empty() & showNofitications;

    if (!show_notification && !show_warnings)
        return;

    ImGuiIO& io = ImGui::GetIO();
    float width = io.DisplaySize.x * 0.4f;
    float pos = io.DisplaySize.x * 0.3f;

    if (show_notification){
        notifications_timeout += io.DeltaTime;
        float height = ImGui::GetTextLineHeightWithSpacing() * notifications.size();
        float y = -height + MIN( notifications_timeout * height * 10.f, height );

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(COLOR_NAVIGATOR, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(COLOR_NAVIGATOR, 1.f));

        ImGui::SetNextWindowPos( ImVec2(pos, y), ImGuiCond_Always );
        ImGui::SetNextWindowSize( ImVec2(width, height), ImGuiCond_Always );
        ImGui::SetNextWindowBgAlpha(0.8f); // Transparent background
        if (ImGui::Begin("##notification", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav ))
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + width);
            for (list<string>::iterator it=notifications.begin(); it != notifications.end(); ++it) {
                ImGui::Text( ICON_FA_INFO "  %s\n", (*it).c_str());
            }
            ImGui::PopTextWrapPos();

        }
        ImGui::End();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        // stop showing after timeout
        if ( notifications_timeout > IMGUI_NOTIFICATION_DURATION )
            notifications.clear();
    }

    if (show_warnings) {
        ImGui::OpenPopup("Warning");
        if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGuiToolkit::Icon(9, 4);
            ImGui::SameLine(0, 10);
            ImGui::SetNextItemWidth(width);
            ImGui::TextColored(ImVec4(1.0f,0.6f,0.0f,1.0f), "%ld error(s) occured.\n\n", warnings.size());
            ImGui::Dummy(ImVec2(width, 0));

            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + width);
            for (list<string>::iterator it=warnings.begin(); it != warnings.end(); ++it) {
                ImGui::Text("%s \n", (*it).c_str());
                ImGui::Separator();
            }
            ImGui::PopTextWrapPos();

            ImGui::Dummy(ImVec2(width * 0.8f, 0)); ImGui::SameLine(); // right align
            if (ImGui::Button(" Ok ", ImVec2(width * 0.2f, 0))) {
                ImGui::CloseCurrentPopup();
                // messages have been seen
                warnings.clear();
            }

            ImGui::SetItemDefaultFocus();
            ImGui::EndPopup();
        }
    }


}

void Log::Error(const char* fmt, ...)
{
    ImGuiTextBuffer buf;

    va_list args;
    va_start(args, fmt);
    buf.appendfv(fmt, args);
    va_end(args);

    tinyfd_messageBox( APP_TITLE, buf.c_str(), "ok", "error", 0);
    Log::Info("Error - %s\n", buf.c_str());
}

