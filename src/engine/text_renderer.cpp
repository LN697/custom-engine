#include "engine/text_renderer.h"
#include <imgui.h>

namespace engine {

TextRenderer::TextRenderer()
    : visible_chars_(0), elapsed_(0.0f), chars_per_second_(30.0f), playing_(false) {}

void TextRenderer::set_text(const std::string& text) {
    full_text_ = text;
    visible_chars_ = 0;
    elapsed_ = 0.0f;
    playing_ = false;
}

void TextRenderer::start() {
    visible_chars_ = 0;
    elapsed_ = 0.0f;
    playing_ = true;
}

void TextRenderer::update(float dt) {
    if (!playing_) return;
    elapsed_ += dt;
    size_t target = static_cast<size_t>(elapsed_ * chars_per_second_);
    if (target >= full_text_.size()) {
        visible_chars_ = full_text_.size();
        playing_ = false;
    } else {
        visible_chars_ = target;
    }
}

void TextRenderer::render() {
    if (full_text_.empty()) return;
    std::string display = full_text_.substr(0, visible_chars_);

    // Slightly opaque background so text remains readable over scenes
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 140), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 140), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("CutsceneText", nullptr, flags)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
        ImGui::TextWrapped("%s", display.c_str());
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    ImGui::End();
}

} // namespace engine
