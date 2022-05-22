/*
 * This file is part of vimix - video live mixer
 *
 * **Copyright** (C) 2019-2022 Bruno Herbelin <bruno.herbelin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
**/


#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <tinyxml2.h>
#include "tinyxml2Toolkit.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "defines.h"
#include "Log.h"
#include "Scene.h"
#include "Primitives.h"
#include "ImageShader.h"
#include "ImageProcessingShader.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "CloneSource.h"
#include "FrameBufferFilter.h"
#include "DelayFilter.h"
#include "ImageFilter.h"
#include "RenderSource.h"
#include "SessionSource.h"
#include "PatternSource.h"
#include "DeviceSource.h"
#include "NetworkSource.h"
#include "SrtReceiverSource.h"
#include "MultiFileSource.h"
#include "SessionCreator.h"
#include "SessionVisitor.h"
#include "Settings.h"
#include "Mixer.h"
#include "MixingGroup.h"
#include "ActionManager.h"

#include "imgui.h"
#include "ImGuiToolkit.h"
#include "BaseToolkit.h"
#include "UserInterfaceManager.h"
#include "SystemToolkit.h"

#include "ImGuiVisitor.h"

ImGuiVisitor::ImGuiVisitor()
{

}

void ImGuiVisitor::visit(Node &)
{

}

void ImGuiVisitor::visit(Group &n)
{
    // MODEL VIEW
    ImGui::PushID(std::to_string(n.id()).c_str());

    if (ImGuiToolkit::ButtonIcon(1, 16)) {
        n.translation_.x = 0.f;
        n.translation_.y = 0.f;
        n.rotation_.z = 0.f;
        n.scale_.x = 1.f;
        n.scale_.y = 1.f;
        Action::manager().store("Geometry Reset");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Geometry");

    if (ImGuiToolkit::ButtonIcon(6, 15)) {
        n.translation_.x = 0.f;
        n.translation_.y = 0.f;
        Action::manager().store("Position 0.0, 0.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    float translation[2] = { n.translation_.x, n.translation_.y};
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if ( ImGui::SliderFloat2("Position", translation, -5.0, 5.0) )
    {
        n.translation_.x = translation[0];
        n.translation_.y = translation[1];
    }
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Position " << std::setprecision(3) << n.translation_.x << ", " << n.translation_.y;
        Action::manager().store(oss.str());
    }
    if (ImGuiToolkit::ButtonIcon(3, 15))  {
        n.scale_.x = 1.f;
        n.scale_.y = 1.f;
        Action::manager().store("Scale 1.0 x 1.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    float scale[2] = { n.scale_.x, n.scale_.y} ;
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if ( ImGui::SliderFloat2("Scale", scale, -MAX_SCALE, MAX_SCALE, "%.2f") )
    {
        n.scale_.x = CLAMP_SCALE(scale[0]);
        n.scale_.y = CLAMP_SCALE(scale[1]);
    }
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Scale " << std::setprecision(3) << n.scale_.x << " x " << n.scale_.y;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::ButtonIcon(18, 9)){
        n.rotation_.z = 0.f;
        Action::manager().store("Angle 0.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderAngle("Angle", &(n.rotation_.z), -180.f, 180.f) ;
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        std::ostringstream oss;
        oss << "Angle " << std::setprecision(3) << n.rotation_.z * 180.f / M_PI;
        Action::manager().store(oss.str());
    }


    ImGui::PopID();

    // spacing
    ImGui::Spacing();
}

void ImGuiVisitor::visit(Switch &n)
{
    if (n.numChildren()>0)
        n.activeChild()->accept(*this);
}

void ImGuiVisitor::visit(Scene &n)
{
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Scene Property Tree"))
    {
        n.root()->accept(*this);
    }
}

void ImGuiVisitor::visit(Primitive &n)
{
    ImGui::PushID(std::to_string(n.id()).c_str());
    ImGui::Text("Primitive");

    n.shader()->accept(*this);

    ImGui::PopID();
}

void ImGuiVisitor::visit(FrameBufferSurface &)
{
    ImGui::Text("Framebuffer");
}

void ImGuiVisitor::visit(MediaPlayer &)
{
    ImGui::Text("Media Player");
}

void ImGuiVisitor::visit(Shader &n)
{
    ImGui::PushID(std::to_string(n.id()).c_str());

    // Base color
//    if (ImGuiToolkit::ButtonIcon(10, 2)) {
//        n.blending = Shader::BLEND_OPACITY;
//        n.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
//    }
//    ImGui::SameLine(0, IMGUI_SAME_LINE);
//    ImGui::ColorEdit3("Color", glm::value_ptr(n.color), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel ) ;
//    ImGui::SameLine(0, 5);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    int mode = n.blending;
    if (ImGui::Combo("Blending", &mode, "Normal\0Screen\0Subtract\0Multiply\0Soft light"
                     "\0Hard light\0Soft subtract\0Lighten only\0") ) {
        n.blending = Shader::BlendMode(mode);

        std::ostringstream oss;
        oss << "Blending ";
        switch(n.blending) {
        case Shader::BLEND_OPACITY:
            oss<<"Normal";
            break;
        case Shader::BLEND_SCREEN:
            oss<<"Screen";
            break;
        case Shader::BLEND_SUBTRACT:
            oss<<"Subtract";
            break;
        case Shader::BLEND_MULTIPLY:
            oss<<"Multiply";
            break;
        case Shader::BLEND_HARD_LIGHT:
            oss<<"Hard light";
            break;
        case Shader::BLEND_SOFT_LIGHT:
            oss<<"Soft light";
            break;
        case Shader::BLEND_SOFT_SUBTRACT:
            oss<<"Soft subtract";
            break;
        case Shader::BLEND_LIGHTEN_ONLY:
            oss<<"Lighten only";
            break;
        case Shader::BLEND_NONE:
            oss<<"None";
            break;
        }
        Action::manager().store(oss.str());
    }

    ImGui::PopID();
}

//void ImGuiVisitor::visit(ImageShader &n)
//{
//    ImGui::PushID(std::to_string(n.id()).c_str());
//    // get index of the mask used in this ImageShader
//    int item_current = n.mask;
////    if (ImGuiToolkit::ButtonIcon(10, 3)) n.mask = 0;
////    ImGui::SameLine(0, IMGUI_SAME_LINE);
//    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
//    // combo list of masks
//    if ( ImGui::Combo("Mask", &item_current, ImageShader::mask_names, IM_ARRAYSIZE(ImageShader::mask_names) ) )
//    {
//        if (item_current < (int) ImageShader::mask_presets.size())
//            n.mask = item_current;
//        else {
//            // TODO ask for custom mask
//        }
//        Action::manager().store("Mask "+ std::string(ImageShader::mask_names[n.mask]));
//    }
//    ImGui::PopID();
//}

void ImGuiVisitor::visit(ImageProcessingShader &n)
{
    ImGui::PushID(std::to_string(n.id()).c_str());

    if (ImGuiToolkit::IconButton(6, 4)) {
        n.gamma = glm::vec4(1.f, 1.f, 1.f, 1.f);
        Action::manager().store("Gamma & Color");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::ColorEdit3("Gamma Color", glm::value_ptr(n.gamma), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel) ;
    if (ImGui::IsItemDeactivatedAfterEdit())
        Action::manager().store("Gamma Color changed");

    ImGui::SameLine(0, 5);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderFloat("Gamma", &n.gamma.w, 0.5f, 10.f, "%.2f", 2.f);
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Gamma " << std::setprecision(2) << n.gamma.w;
        Action::manager().store(oss.str());
    }

//    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
//    ImGui::SliderFloat4("Levels", glm::value_ptr(n.levels), 0.0, 1.0);

    if (ImGuiToolkit::IconButton(5, 16)) {
        n.brightness = 0.f;
        n.contrast = 0.f;
        Action::manager().store("B & C  0.0 0.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    float bc[2] = { n.brightness, n.contrast};
    if ( ImGui::SliderFloat2("B & C", bc, -1.0, 1.0) )
    {
        n.brightness = bc[0];
        n.contrast = bc[1];
    }
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "B & C  " << std::setprecision(2) << n.brightness << " " << n.contrast;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(9, 16)) {
        n.saturation = 0.f;
        Action::manager().store("Saturation 0.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderFloat("Saturation", &n.saturation, -1.0, 1.0);
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Saturation " << std::setprecision(2) << n.saturation;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(12, 4)) {
        n.hueshift = 0.f;
        Action::manager().store("Hue shift 0.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderFloat("Hue shift", &n.hueshift, 0.0, 1.0);
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Hue shift " << std::setprecision(2) << n.hueshift;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(18, 1)) {
        n.nbColors = 0;
        Action::manager().store("Posterize None");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderInt("Posterize", &n.nbColors, 0, 16, n.nbColors == 0 ? "None" : "%d colors");
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Posterize ";
        if (n.nbColors == 0) oss << "None"; else oss << n.nbColors;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(8, 1)) {
        n.threshold = 0.f;
        Action::manager().store("Threshold None");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderFloat("Threshold", &n.threshold, 0.0, 1.0, n.threshold < 0.001 ? "None" : "%.2f");
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Threshold ";
        if (n.threshold < 0.001) oss << "None"; else oss << std::setprecision(2) << n.threshold;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(3, 1)) {
        n.lumakey = 0.f;
        Action::manager().store("Lumakey 0.0");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderFloat("Lumakey", &n.lumakey, 0.0, 1.0);
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Lumakey " << std::setprecision(2) << n.lumakey;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(13, 4)) {
        n.chromakey = glm::vec4(0.f, 0.8f, 0.f, 1.f);
        n.chromadelta = 0.f;
        Action::manager().store("Chromakey & Color Reset");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::ColorEdit3("Chroma color", glm::value_ptr(n.chromakey), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel  ) ;    
    if (ImGui::IsItemDeactivatedAfterEdit())
        Action::manager().store("Chroma color changed");
    ImGui::SameLine(0, 5);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderFloat("Chromakey", &n.chromadelta, 0.0, 1.0, n.chromadelta < 0.001 ? "None" : "Tolerance %.2f");
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << "Chromakey ";
        if (n.chromadelta < 0.001) oss << "None"; else oss << std::setprecision(2) << n.chromadelta;
        Action::manager().store(oss.str());
    }

    if (ImGuiToolkit::IconButton(6, 16)) {
        n.invert = 0;
        Action::manager().store("Invert None");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if (ImGui::Combo("Invert", &n.invert, "None\0Color RGB\0Luminance\0"))
        Action::manager().store("Invert " + std::string(n.invert<1 ? "None": (n.invert>1 ? "Luminance" : "Color")));

    ImGui::PopID();

    ImGui::Spacing();
}

void ImGuiVisitor::visit (Source& s)
{
    ImGui::PushID(std::to_string(s.id()).c_str());
    // blending
    s.blendingShader()->accept(*this);

    // preview
    float preview_width = ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN;
    float preview_height = 4.5f * ImGui::GetFrameHeightWithSpacing();
    ImVec2 pos = ImGui::GetCursorPos(); // remember where we were...

    float space = ImGui::GetStyle().ItemSpacing.y;
    float width = preview_width;
    float height = s.frame()->projectionArea().y * width / ( s.frame()->projectionArea().x * s.frame()->aspectRatio());
    if (height > preview_height - space) {
        height = preview_height - space;
        width = height * s.frame()->aspectRatio() * ( s.frame()->projectionArea().x / s.frame()->projectionArea().y);
    }
    // centered image
    ImGui::SetCursorPos( ImVec2(pos.x + 0.5f * (preview_width-width), pos.y + 0.5f * (preview_height-height-space)) );
    ImGui::Image((void*)(uintptr_t) s.frame()->texture(), ImVec2(width, height));

    // inform on visibility status
    ImGui::SetCursorPos( ImVec2(preview_width + 20, pos.y ) );
    if (s.active()) {
        if (s.blendingShader()->color.a > 0.f)
            ImGuiToolkit::Indication("Visible", ICON_FA_EYE);
        else
            ImGuiToolkit::Indication("Not visible", ICON_FA_EYE_SLASH);
    }
    else
        ImGuiToolkit::Indication("Inactive", ICON_FA_SNOWFLAKE);

    // Inform on workspace
    ImGui::SetCursorPos( ImVec2(preview_width + 20, pos.y + ImGui::GetFrameHeightWithSpacing()) );
    if (s.workspace() == Source::BACKGROUND)
        ImGuiToolkit::Indication("in Background",10, 16);
    else if (s.workspace() == Source::FOREGROUND)
        ImGuiToolkit::Indication("in Foreground",12, 16);
    else
        ImGuiToolkit::Indication("in Workspace",11, 16);

    // Inform on link
    ImGui::SetCursorPos( ImVec2(preview_width + 20, pos.y + 2.1f * ImGui::GetFrameHeightWithSpacing()) );
    if (s.mixingGroup() != nullptr) {
        if (ImGuiToolkit::IconButton(ICON_FA_LINK, "Linked")){
            Mixer::selection().clear();
            Mixer::selection().add( s.mixingGroup()->getCopy() );
        }
    }
    else
        ImGuiToolkit::Indication("not Linked", ICON_FA_UNLINK);

    // locking
    ImGui::SetCursorPos( ImVec2(preview_width + 20, pos.y + 3.f * ImGui::GetFrameHeightWithSpacing()) );
    const char *tooltip[2] = {"Unlocked", "Locked"};
    bool l = s.locked();
    if (ImGuiToolkit::IconToggle(15,6,17,6, &l, tooltip ) ) {
        s.setLocked(l);
        if (l) {
            Mixer::selection().clear();
            Action::manager().store(s.name() + std::string(": lock."));
        }
        else {
            Mixer::selection().set(&s);
            Action::manager().store(s.name() + std::string(": unlock."));
        }
    }

    // Filter
    bool on = s.imageProcessingEnabled();
    ImGui::SetCursorPos( ImVec2( pos.x, pos.y + preview_height));
    if (on) {
        ImGuiToolkit::Icon(6, 2);
        ImGui::SameLine(0, IMGUI_SAME_LINE);
        ImGui::Text("Color correction");
    }
    else {
        ImGuiToolkit::Indication("Color correction disabled", 6, 2);
        ImGui::SameLine(0, IMGUI_SAME_LINE);
        ImGui::TextDisabled("Color correction");
    }
    pos = ImGui::GetCursorPos();

    // menu icon for image processing
    ImGui::SameLine(preview_width, 2 * IMGUI_SAME_LINE);
    if (ImGuiToolkit::IconButton(5, 8))
        ImGui::OpenPopup( "MenuImageProcessing" );

    if (ImGui::BeginPopup( "MenuImageProcessing" ))
    {
        if (ImGui::MenuItem("Enable", NULL, &on)) {
            std::ostringstream oss;
            oss << s.name() << ": " << ( on ? "Enable Color correction" : "Disable Color correction");
            Action::manager().store(oss.str());
            s.setImageProcessingEnabled(on);
        }

        if (s.processingshader_link_.connected()) {
            if (ImGui::MenuItem( "Unfollow", NULL, false, on)){
                s.processingshader_link_.disconnect();
            }
        }
        else {
            if (ImGui::MenuItem("Reset", NULL, false, on )){
                ImageProcessingShader defaultvalues;
                s.processingShader()->copy(defaultvalues);
                s.processingshader_link_.disconnect();
                std::ostringstream oss;
                oss << s.name() << ": " << "Reset Filter";
                Action::manager().store(oss.str());
            }
            if (ImGui::MenuItem("Copy", NULL, false, on )){
                std::string clipboard = SessionVisitor::getClipboard(s.processingShader());
                if (!clipboard.empty())
                    ImGui::SetClipboardText(clipboard.c_str());
            }
            const char *clipboard = ImGui::GetClipboardText();
            const bool can_paste = (clipboard != nullptr && SessionLoader::isClipboard(clipboard));
            if (ImGui::MenuItem("Paste", NULL, false, can_paste)) {
                SessionLoader::applyImageProcessing(s, clipboard);
                std::ostringstream oss;
                oss << s.name() << ": " << "Change Filter";
                Action::manager().store(oss.str());
            }
//            // NON-stable style follow mechanism
//            ImGui::Separator();
//            if (ImGui::BeginMenu("Follow", on))
//            {
//                for (auto mpit = Mixer::manager().session()->begin();
//                     mpit != Mixer::manager().session()->end(); mpit++ )
//                {
//                    std::string label = (*mpit)->name();
//                    if ( (*mpit)->id() != s.id() &&
//                         (*mpit)->imageProcessingEnabled() &&
//                         !(*mpit)->processingshader_link_.connected()) {
//                        if (ImGui::MenuItem( label.c_str() )){
//                            s.processingshader_link_.connect(*mpit);
//                            s.touch();
//                        }
//                    }
//                }
//                ImGui::EndMenu();
//            }
        }

        ImGui::EndPopup();
    }

    if (s.imageProcessingEnabled()) {

        // full panel for image processing
        ImGui::SetCursorPos( pos );

        if (s.processingshader_link_.connected()) {
            Source *target = s.processingshader_link_.source();
            ImGui::Text("Following");
            if ( target != nullptr && ImGui::Button(target->name().c_str(), ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
                Mixer::manager().setCurrentSource(target);
        }
        else
            s.processingShader()->accept(*this);
    }

    ImGui::Spacing();

    ImGui::PopID();

    ImGuiToolkit::Icon(s.icon().x, s.icon().y);
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("%s", s.info().c_str());
}

void ImGuiVisitor::visit (MediaSource& s)
{
    // Media info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, 10.f + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    // folder
    std::string path = SystemToolkit::path_filename(s.path());
    std::string label = BaseToolkit::truncated(path, 25);
    label = BaseToolkit::transliterate(label);
    ImGuiToolkit::ButtonOpenUrl( label.c_str(), path.c_str(), ImVec2(IMGUI_RIGHT_ALIGN, 0) );

    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Folder");
}

void ImGuiVisitor::visit (SessionFileSource& s)
{
    if (s.session() == nullptr)
        return;

    // info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, 10.f + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    if ( ImGui::Button( ICON_FA_SIGN_OUT_ALT " Import", ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
        Mixer::manager().import( &s );
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Sources");

    // versions
    SessionSnapshots *versions = s.session()->snapshots();
    if (versions->keys_.size()>0) {
        ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
        if (ImGui::BeginCombo("Version", ICON_FA_CODE_BRANCH " Select" ) )
        {
            for (auto v = versions->keys_.crbegin() ; v != versions->keys_.crend(); ++v){
                std::string label = std::to_string(*v);
                const tinyxml2::XMLElement *snap = versions->xmlDoc_->FirstChildElement( SNAPSHOT_NODE(*v).c_str() );
                if (snap)
                    label = snap->Attribute("label");
                if (ImGui::Selectable( label.c_str() )) {
                    s.session()->applySnapshot(*v);
                }
            }
            ImGui::EndCombo();
        }
    }

    // fading
    if (ImGuiToolkit::IconButton(2, 1)) s.session()->setFadingTarget(0.f);
    int f = 100 - int(s.session()->fading() * 100.f);
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if (ImGui::SliderInt("Fading", &f, 0, 100, f > 99 ? "None" : "%d %%") )
        s.session()->setFadingTarget( float(100 - f) * 0.01f );
    if (ImGui::IsItemDeactivatedAfterEdit()){
        std::ostringstream oss;
        oss << s.name() << ": Fading " << f << " %";
        Action::manager().store(oss.str());
    }

    // file open
    if ( ImGui::Button( ICON_FA_FILE_UPLOAD " Open", ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
        Mixer::manager().set( s.detach() );
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("File");
    // file path
    std::string path = SystemToolkit::path_filename(s.path());
    std::string label = BaseToolkit::truncated(path, 25);
    label = BaseToolkit::transliterate(label);
    ImGuiToolkit::ButtonOpenUrl( label.c_str(), path.c_str(), ImVec2(IMGUI_RIGHT_ALIGN, 0) );
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Folder");

}

void ImGuiVisitor::visit (SessionGroupSource& s)
{
    Session *session = s.session();
    if (session == nullptr)
        return;

    // info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, 10.f + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    // Show list of sources in text bloc (multi line, dark background)
    ImGuiTextBuffer info;
    info.append( BaseToolkit::joinned( session->getNameList(), '\n').c_str() );
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.14f, 0.14f, 0.9f));
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::InputTextMultiline("##sourcesingroup", (char *)info.c_str(), info.size(),
                              ImVec2(IMGUI_RIGHT_ALIGN, CLAMP(session->size(), 2, 5) * ImGui::GetTextLineHeightWithSpacing()),
                              ImGuiInputTextFlags_ReadOnly);
    ImGui::PopStyleColor(1);

    if ( ImGui::Button( ICON_FA_SIGN_OUT_ALT " Import", ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
        Mixer::manager().import( &s );
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Sources");
}

void ImGuiVisitor::visit (RenderSource& s)
{
    // info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    ImVec2 pos = ImGui::GetCursorPos();
    ImGui::SameLine(0, 0);
    ImGui::SameLine(0, 10.f + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
        UserInterface::manager().showSourceEditor(&s);
    ImGui::SameLine();
    if (ImGuiToolkit::IconButton(ICON_FA_DESKTOP, "Show Output"))
        Settings::application.widget.preview = true;
    ImGui::SetCursorPos(pos);

    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    int m = (int) s.renderingProvenance();
    if (ImGui::Combo("Render", &m, RenderSource::rendering_provenance_label, IM_ARRAYSIZE(RenderSource::rendering_provenance_label)) )
        s.setRenderingProvenance((RenderSource::RenderSourceProvenance)m);

}

void ImGuiVisitor::visit (FrameBufferFilter&)
{

}

void ImGuiVisitor::visit (PassthroughFilter&)
{

}

void ImGuiVisitor::visit (DelayFilter& f)
{
    if (ImGuiToolkit::IconButton(10, 15)) {
        f.setDelay(0.f);
        Action::manager().store("Delay 0 s");
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    float d = f.delay();
    if (ImGui::SliderFloat("Delay", &d, 0.f, 2.f, "%.2f s"))
        f.setDelay(d);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        std::ostringstream oss;
        oss << "Delay " << std::setprecision(3) << d << " s";
        Action::manager().store(oss.str());
    }
}

void ImGuiVisitor::visit (ImageFilter& f)
{
    // Selection of Algorithm
    if (ImGuiToolkit::IconButton(1, 4)) {
        FilteringProgram target;
        f.setProgram( target );
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if (ImGui::BeginCombo("##Filters", f.program().name().c_str()) )
    {
        for (auto p = FilteringProgram::presets.begin(); p != FilteringProgram::presets.end(); ++p){
            if (ImGui::Selectable( p->name().c_str() )) {
                // apply the selected filter to the source
                f.setProgram( *p );
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Algorithm");

    // List of parameters
    std::map<std::string, float> filter_parameters = f.program().parameters();
    FilteringProgram target = f.program();
    for (auto param = filter_parameters.begin(); param != filter_parameters.end(); ++param)
    {
        ImGui::PushID( param->first.c_str() );
        float v = param->second;
        if (ImGuiToolkit::IconButton(11, 11)) {
            v = 0.f;
            target.setParameter(param->first, v);
            f.setProgram( target );
        }
        ImGui::SameLine(0, IMGUI_SAME_LINE);
        ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
        if (ImGui::SliderFloat( param->first.c_str(), &v, 0.f, 1.f, "%.2f")) {
            target.setParameter(param->first, v);
            f.setProgram( target );
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            // TODO UNDO
            //            std::ostringstream oss;
            //            oss << "Delay " << std::setprecision(3) << d << " s";
            //            Action::manager().store(oss.str());
        }
        ImGui::PopID();
    }

    // Open Editor
    ImGuiToolkit::IconButton(18, 18);
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    if ( ImGui::Button( ICON_FA_CODE "  Edit", ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
        Settings::application.widget.shader_editor = true;
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Code");
}

void ImGuiVisitor::visit (CloneSource& s)
{
    // info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    ImVec2 pos = ImGui::GetCursorPos();
    ImGui::SameLine(0, 0);
    ImGui::SameLine(0, 10.f + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
        UserInterface::manager().showSourceEditor(&s);
    ImGui::SetCursorPos(pos);

    // link to origin source
    std::string label = std::string(s.origin()->initials()) + " - " + s.origin()->name();
    if (ImGui::Button(label.c_str(), ImVec2(IMGUI_RIGHT_ALIGN, 0) ))
        Mixer::manager().setCurrentSource(s.origin());
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Origin");

    // filter selection
    std::ostringstream oss;
    oss << s.name();
    if (ImGuiToolkit::IconButton(1, 7)) {
        s.setFilter( FrameBufferFilter::FILTER_PASSTHROUGH );
        oss << ": Filter None";
        Action::manager().store(oss.str());
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    int type = (int) s.filter()->type();
    if (ImGui::Combo("Filter", &type, FrameBufferFilter::type_label, IM_ARRAYSIZE(FrameBufferFilter::type_label) )) {
        s.setFilter( FrameBufferFilter::Type(type) );
        oss << ": Filter " << FrameBufferFilter::type_label[type];
        Action::manager().store(oss.str());
    }

    // filter options
    s.filter()->accept(*this);

}

void ImGuiVisitor::visit (PatternSource& s)
{
    // stream info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, IMGUI_SAME_LINE + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if (ImGui::BeginCombo("##Patterns", Pattern::get(s.pattern()->type()).label.c_str()) )
    {
        for (uint p = 0; p < Pattern::count(); ++p){
            if (ImGui::Selectable( Pattern::get(p).label.c_str() )) {
                s.setPattern(p, s.pattern()->resolution());
                info.reset();
                std::ostringstream oss;
                oss << s.name() << ": Pattern " << Pattern::get(p).label;
                Action::manager().store(oss.str());
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Generator");
}

void ImGuiVisitor::visit (DeviceSource& s)
{
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, IMGUI_SAME_LINE + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    if (ImGui::BeginCombo("Device", s.device().c_str()))
    {
        for (int d = 0; d < Device::manager().numDevices(); ++d){
            std::string namedev = Device::manager().name(d);
            if (ImGui::Selectable( namedev.c_str() )) {
                s.setDevice(namedev);
                info.reset();
                std::ostringstream oss;
                oss << s.name() << " Device " << namedev;
                Action::manager().store(oss.str());
            }
        }
        ImGui::EndCombo();
    }

}

void ImGuiVisitor::visit (NetworkSource& s)
{
    ImGuiToolkit::Icon(s.icon().x, s.icon().y);
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Shared stream");

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(IMGUI_COLOR_STREAM, 0.9f));
    ImGui::Text("%s", s.connection().c_str());
    ImGui::PopStyleColor(1);

    // network info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, IMGUI_SAME_LINE + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    if ( ImGui::Button( ICON_FA_REPLY " Reconnect", ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
    {
        s.setConnection(s.connection());
        info.reset();
    }
}


void ImGuiVisitor::visit (MultiFileSource& s)
{
    static uint64_t id = 0;

    // information text
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, IMGUI_SAME_LINE + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    // change range
    static int _begin = -1;
    if (_begin < 0 || id != s.id())
        _begin = s.begin();
    static int _end = -1;
    if (_end < 0 || id != s.id())
        _end = s.end();
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::DragIntRange2("Range", &_begin, &_end, 1, s.sequence().min, s.sequence().max);
    if (ImGui::IsItemDeactivatedAfterEdit()){
        s.setRange( _begin, _end );
        std::ostringstream oss;
        oss << s.name() << ": Range " << _begin << "-" << _end;
        Action::manager().store(oss.str());
        _begin = _end = -1;
    }

    // change framerate
    static int _fps = -1;
    if (_fps < 0 || id != s.id())
        _fps = s.framerate();
    ImGui::SetNextItemWidth(IMGUI_RIGHT_ALIGN);
    ImGui::SliderInt("Framerate", &_fps, 1, 30, "%d fps");
    if (ImGui::IsItemDeactivatedAfterEdit()){
        s.setFramerate(_fps);
        std::ostringstream oss;
        oss << s.name() << ": Framerate " << _fps << " fps";
        Action::manager().store(oss.str());
        _fps = -1;
    }

    // offer to open file browser at location
    std::string path = SystemToolkit::path_filename(s.sequence().location);
    std::string label = BaseToolkit::truncated(path, 25);
    label = BaseToolkit::transliterate(label);
    ImGuiToolkit::ButtonOpenUrl( label.c_str(), path.c_str(), ImVec2(IMGUI_RIGHT_ALIGN, 0) );
    ImGui::SameLine(0, IMGUI_SAME_LINE);
    ImGui::Text("Folder");

    if (id != s.id())
        id = s.id();
}

void ImGuiVisitor::visit (GenericStreamSource& s)
{
    float w = ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN;

    // stream info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + w);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, IMGUI_SAME_LINE + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

    // Prepare display pipeline text
    static int numlines = 0;
    const ImGuiContext& g = *GImGui;
    ImVec2 fieldsize(w,  MAX(3, numlines) * g.FontSize + g.Style.ItemSpacing.y + g.Style.FramePadding.y);

    // Editor
    std::string _description = s.description();
    if ( ImGuiToolkit::InputCodeMultiline("Pipeline", &_description, fieldsize, &numlines) ) {
        s.setDescription(_description);
        Action::manager().store( s.name() + ": Change pipeline");
    }

}


void ImGuiVisitor::visit (SrtReceiverSource& s)
{
    // network info
    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
    s.accept(info);
    ImGui::Text("%s", info.str().c_str());
    ImGui::PopTextWrapPos();

    // icon (>) to open player
    if ( s.playable() ) {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SameLine(0, 0);
        ImGui::SameLine(0, IMGUI_SAME_LINE + ImGui::GetContentRegionAvail().x IMGUI_RIGHT_ALIGN);
        if (ImGuiToolkit::IconButton(ICON_FA_PLAY_CIRCLE, "Open in Player"))
            UserInterface::manager().showSourceEditor(&s);
        ImGui::SetCursorPos(pos);
    }

//    if ( ImGui::Button( ICON_FA_REPLY " Reconnect", ImVec2(IMGUI_RIGHT_ALIGN, 0)) )
//    {
//        s.setConnection(s.connection());
//        info.reset();
//    }
}
