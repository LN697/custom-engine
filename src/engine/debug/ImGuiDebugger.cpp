#include "engine/debug_imgui.h"
#include "engine/post_processor.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/gl.h>
#include <sstream>

namespace engine {

ImGuiDebugger::ImGuiDebugger()
    : is_initialized_(false),
      window_(nullptr),
      gl_context_(nullptr),
      post_processor_(nullptr),
      movement_smoothing_ptr_(nullptr),
      camera_pos_(0.0f),
      camera_yaw_(0.0f),
      camera_pitch_(0.0f),
      fps_(0.0f),
      frame_count_(0),
      uptime_(0.0f),
      gl_version_("Unknown"),
      gl_renderer_("Unknown"),
      depth_test_enabled_(false) {
    // Initialize console disabled by default and FPS history
    console_enabled_ = false;
    fps_capacity_ = 128;
    fps_history_.assign(fps_capacity_, 0.0f);
    fps_index_ = 0;
    // Event log defaults
    event_log_capacity_ = 512;
    event_log_.clear();
}

ImGuiDebugger::~ImGuiDebugger() {
    shutdown();
}

bool ImGuiDebugger::initialize(SDL_Window* window, SDL_GLContext context) {
    if (is_initialized_) {
        return true;
    }

    window_ = window;
    gl_context_ = context;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");

    is_initialized_ = true;
    return true;
}

void ImGuiDebugger::shutdown() {
    if (!is_initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    is_initialized_ = false;
}

void ImGuiDebugger::begin_frame() {
    if (!is_initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDebugger::draw_ui() {
    if (!is_initialized_) {
        return;
    }

    // Info Window
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Info")) {
        ImGui::Text("FPS: %.1f", fps_);
        ImGui::SameLine(150);
        ImGui::Text("Frame Count: %d", frame_count_);
        ImGui::Text("Uptime: %.2f s", uptime_);
        ImGui::Separator();
        ImGui::PlotLines("FPS Graph", fps_history_.data(), (int)fps_capacity_, (int)fps_index_, nullptr, 0.0f, 200.0f, ImVec2(-1, 100));
        ImGui::Separator();
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camera_pos_.x, camera_pos_.y, camera_pos_.z);
        ImGui::Text("Camera Rotation: Y=%.2f P=%.2f", camera_yaw_, camera_pitch_);
        if (movement_smoothing_ptr_) {
            ImGui::SliderFloat("Movement Smoothing", movement_smoothing_ptr_, 0.1f, 50.0f);
        }
        ImGui::Separator();
        ImGui::Text("GL Info:");
        ImGui::Text("  Version: %s", gl_version_.c_str());
        ImGui::Text("  Renderer: %s", gl_renderer_.c_str());
        ImGui::Text("  Depth Test: %s", depth_test_enabled_ ? "Enabled" : "Disabled");
        ImGui::Separator();
        if (!stage_timings_.empty()) {
            ImGui::Text("Stage Timings (ms):");
            for (const auto& pair : stage_timings_) {
                ImGui::Text("  %s: %.3f", pair.first.c_str(), pair.second);
            }
        }
    }
    ImGui::End();

    // Shaders Window
    ImGui::SetNextWindowPos(ImVec2(520, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Shaders")) {
        if (post_processor_) {
            bool enabled = post_processor_->is_enabled();
            if (ImGui::Checkbox("Postprocess Enabled", &enabled)) {
                post_processor_->set_enabled(enabled);
            }
            ImGui::SameLine();
            if (ImGui::Button("Reload Shader")) {
                post_processor_->load_shader_files("shaders/post_default.vert", "shaders/post_default.frag");
            }
            ImGui::Separator();

            // Effect selector
            int effect = post_processor_->effect();
            const char* effect_items[] = { "None", "Grayscale", "Invert", "Sepia" };
            if (ImGui::Combo("Effect", &effect, effect_items, IM_ARRAYSIZE(effect_items))) {
                post_processor_->set_effect(effect);
            }
            ImGui::Separator();

            // Shader/log status
            std::string log = post_processor_->last_log();
            if (!log.empty()) {
                ImGui::TextWrapped("Log:\n%s", log.c_str());
            } else if (!post_processor_->has_shader()) {
                ImGui::TextWrapped("No shader loaded.");
            } else {
                ImGui::TextWrapped("Shader compiled and ready.");
            }
        } else {
            ImGui::Text("No post processor attached.");
        }
    }
    ImGui::End();

    // Events Window
    ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(910, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Events")) {
        if (ImGui::Button("Clear")) {
            event_log_.clear();
        }
        ImGui::SameLine();
        ImGui::Text("Recent events: %zu", event_log_.size());
        ImGui::Separator();
        ImGui::BeginChild("EventList", ImVec2(0, -30), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& e : event_log_) {
            ImGui::TextUnformatted(e.c_str());
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // Console Window (optional)
    if (console_enabled_) {
        ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Console")) {
            ImGui::BeginChild("ConsoleOutput", ImVec2(0, -30), true);
            for (const auto& log : console_log_) {
                ImGui::TextWrapped("%s", log.c_str());
            }
            ImGui::EndChild();

            static char input_buffer[256] = "";
            if (ImGui::InputText("Command", input_buffer, sizeof(input_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (command_callback_) {
                    command_callback_(input_buffer);
                }
                input_buffer[0] = '\0';
            }
        }
        ImGui::End();
    }
}

void ImGuiDebugger::record_event(const SDL_Event& event) {
    if (!is_initialized_) {
        // Still keep a minimal record even if ImGui not fully initialized
    }

    std::ostringstream ss;
    switch (event.type) {
        case SDL_KEYDOWN:
            ss << "KEYDOWN: " << SDL_GetKeyName(event.key.keysym.sym);
            break;
        case SDL_KEYUP:
            ss << "KEYUP: " << SDL_GetKeyName(event.key.keysym.sym);
            break;
        case SDL_MOUSEBUTTONDOWN:
            ss << "MOUSEBUTTONDOWN: ";
            if (event.button.button == SDL_BUTTON_LEFT) ss << "LEFT";
            else if (event.button.button == SDL_BUTTON_RIGHT) ss << "RIGHT";
            else if (event.button.button == SDL_BUTTON_MIDDLE) ss << "MIDDLE";
            else ss << (int)event.button.button;
            ss << " at (" << event.button.x << "," << event.button.y << ")";
            break;
        case SDL_MOUSEBUTTONUP:
            ss << "MOUSEBUTTONUP: " << (int)event.button.button << " at (" << event.button.x << "," << event.button.y << ")";
            break;
        case SDL_MOUSEMOTION:
            ss << "MOUSEMOTION: (" << event.motion.x << "," << event.motion.y << ")";
            break;
        case SDL_MOUSEWHEEL:
            ss << "MOUSEWHEEL: (" << event.wheel.x << "," << event.wheel.y << ")";
            break;
        case SDL_WINDOWEVENT:
            ss << "WINDOWEVENT: ";
            switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    ss << "Resized to " << event.window.data1 << "x" << event.window.data2;
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    ss << "Close";
                    break;
                default:
                    ss << "Type=" << static_cast<int>(event.window.event);
                    break;
            }
            break;
        case SDL_QUIT:
            ss << "QUIT";
            break;
        default:
            ss << "EVENT: type=" << event.type;
            break;
    }

    event_log_.push_back(ss.str());
    if (event_log_.size() > event_log_capacity_) {
        event_log_.erase(event_log_.begin());
    }
}

void ImGuiDebugger::clear_event_log() {
    event_log_.clear();
}

void ImGuiDebugger::end_frame() {
    if (!is_initialized_) {
        return;
    }

    ImGui::Render();
}

void ImGuiDebugger::render() {
    if (!is_initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiDebugger::process_event(const SDL_Event& event) {
    if (!is_initialized_) {
        return;
    }
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void ImGuiDebugger::add_console_log(const std::string& message) {
    console_log_.push_back(message);
    // Keep console history limited to 100 messages
    if (console_log_.size() > 100) {
        console_log_.erase(console_log_.begin());
    }
}

void ImGuiDebugger::set_stage_timings(const std::vector<std::pair<std::string, float>>& timings) {
    stage_timings_ = timings;
}

void ImGuiDebugger::set_frame_stats(float fps, int frame_count, float uptime) {
    fps_ = fps;
    frame_count_ = frame_count;
    uptime_ = uptime;

    // push into history (ring buffer)
    if (fps_capacity_ > 0) {
        fps_history_[fps_index_] = fps_;
        fps_index_ = (fps_index_ + 1) % fps_capacity_;
    }
}

void ImGuiDebugger::set_gl_info(const char* version, const char* renderer) {
    gl_version_ = version ? version : "Unknown";
    gl_renderer_ = renderer ? renderer : "Unknown";
}

} // namespace engine
