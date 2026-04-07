#include "engine/debug/ImGuiDebugger.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl2.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <cfloat>
#include <vector>
#include "engine/renderer/PostProcessor.h"

namespace engine {
namespace debug {

// Helper: clamp the currently active ImGui window to the display area so it
// cannot be dragged completely off-screen. Called from inside an active
// Begin()/End() window block.
static void ClampCurrentWindowToDisplay()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 display = io.DisplaySize;
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();

    float maxX = std::max(0.0f, display.x - size.x);
    float maxY = std::max(0.0f, display.y - size.y);

    float newX = pos.x;
    float newY = pos.y;
    if (newX < 0.0f) newX = 0.0f;
    if (newY < 0.0f) newY = 0.0f;
    if (newX > maxX) newX = maxX;
    if (newY > maxY) newY = maxY;

    if (newX != pos.x || newY != pos.y) {
        ImGui::SetWindowPos(ImVec2(newX, newY), ImGuiCond_Always);
    }
}

ImGuiDebugger::ImGuiDebugger()
        : initialized_(false),
            window_(nullptr),
            cameraX_(0.0f),
            cameraY_(0.0f),
            cameraZ_(0.0f),
            cameraYaw_(0.0f),
            cameraPitch_(0.0f),
            fps_(0.0f),
            frameCount_(0),
            uptime_(0.0f),
            glVersion_("Unknown"),
            glRenderer_("Unknown"),
            depthTestEnabled_(false),
                        showPerformancePanel_(true),
                        showCameraPanel_(true),
                        showRenderingPanel_(true),
                                                showDemoWindow_(false),
                                                showConsolePanel_(false) {
        postProcessor_ = nullptr;
        // Prefill shader path textboxes with sensible defaults
        std::strncpy(vertShaderPath_, "shaders/post_default.vert", sizeof(vertShaderPath_) - 1);
        vertShaderPath_[sizeof(vertShaderPath_) - 1] = '\0';
        std::strncpy(fragShaderPath_, "shaders/post_default.frag", sizeof(fragShaderPath_) - 1);
        fragShaderPath_[sizeof(fragShaderPath_) - 1] = '\0';

        fpsHistoryMax_ = 128;
        fpsHistory_.assign(fpsHistoryMax_, 0.0f);
        fpsHistoryIndex_ = 0;
        fpsSmoothed_ = 0.0f;
        fpsSmoothingAlpha_ = 0.2f;
        showFpsGraph_ = true;

        wireframeEnabled_ = false;
        cullFaceEnabled_ = false;
        vsyncEnabled_ = true;
        clearColor_[0] = 0.05f; clearColor_[1] = 0.1f; clearColor_[2] = 0.18f;

        movementSmoothingPtr_ = nullptr;
        consoleInput_[0] = '\0';
        consoleItems_.clear();
        stageTimings_.clear();
}

ImGuiDebugger::~ImGuiDebugger() {
    shutdown();
}

bool ImGuiDebugger::initialize(SDL_Window* window, SDL_GLContext glContext) {
    window_ = window;
    glContext_ = glContext;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Set initial VSync state
    SDL_GL_SetSwapInterval(vsyncEnabled_ ? 1 : 0);

    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL2_InitForOpenGL(window, glContext)) {
        return false;
    }

    if (!ImGui_ImplOpenGL2_Init()) {
        ImGui_ImplSDL2_Shutdown();
        return false;
    }

    // Reflect current GL depth-test state so checkbox matches actual renderer
    depthTestEnabled_ = (glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);

    initialized_ = true;
    return true;
}

void ImGuiDebugger::shutdown() {
    if (!initialized_) return;

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    initialized_ = false;
}

void ImGuiDebugger::beginFrame() {
    if (!initialized_) return;

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDebugger::endFrame() {
    if (!initialized_) return;

    ImGui::Render();
}

void ImGuiDebugger::render() {
    if (!initialized_) return;

    // Safety: Only render if we have a valid GL context
    ImDrawData* drawData = ImGui::GetDrawData();
    if (!drawData) return;

    ImGui_ImplOpenGL2_RenderDrawData(drawData);
}

void ImGuiDebugger::drawUI() {
    if (!initialized_) return;

    // Set initial window positions on first frame (ImGui will remember them after)
    static bool firstFrame = true;
    if (firstFrame) {
        firstFrame = false;
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);
    }

    drawPerformancePanel();
    
    ImGui::SetNextWindowPos(ImVec2(370, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 300), ImGuiCond_FirstUseEver);
    drawCameraPanel();
    
    ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(710, 350), ImGuiCond_FirstUseEver);
    drawRenderingPanel();

    // Console disabled by default (hidden). To re-enable call setShow... or
    // change `showConsolePanel_` initialization.
}

void ImGuiDebugger::setCameraPosition(float x, float y, float z) {
    cameraX_ = x;
    cameraY_ = y;
    cameraZ_ = z;
}

void ImGuiDebugger::setCameraRotation(float yaw, float pitch) {
    cameraYaw_ = yaw;
    cameraPitch_ = pitch;
}

void ImGuiDebugger::setFrameStats(float fps, int frameCount, float uptime) {
    fps_ = fps;
    frameCount_ = frameCount;
    uptime_ = uptime;

    // push into circular history
    if (fpsHistoryMax_ > 0) {
        fpsHistory_[fpsHistoryIndex_] = fps;
        fpsHistoryIndex_ = (fpsHistoryIndex_ + 1) % fpsHistoryMax_;
    }

    // exponential smoothing
    if (fpsSmoothed_ <= 0.0f) fpsSmoothed_ = fps;
    else fpsSmoothed_ = fpsSmoothed_ * (1.0f - fpsSmoothingAlpha_) + fps * fpsSmoothingAlpha_;
}

void ImGuiDebugger::setGLInfo(const char* version, const char* renderer) {
    if (version) glVersion_ = version;
    if (renderer) glRenderer_ = renderer;
}

void ImGuiDebugger::setDepthTestEnabled(bool enabled) {
    depthTestEnabled_ = enabled;
    if (depthTestEnabled_) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void ImGuiDebugger::drawPerformancePanel() {
    if (ImGui::Begin("Performance", &showPerformancePanel_)) {
        ClampCurrentWindowToDisplay();
        ImGui::Text("FPS: %.1f", fps_);
        ImGui::SameLine(); ImGui::Text("(smoothed: %.1f)", fpsSmoothed_);
        ImGui::Text("Frame: %d", frameCount_);
        ImGui::Text("Uptime: %.1f seconds", uptime_);

        // Compute min/avg/max from history for display and autoscaled plot
        float minF = FLT_MAX, maxF = 0.0f, sumF = 0.0f;
        int n = 0;
        for (float v : fpsHistory_) {
            if (v > 0.0f) {
                minF = std::min(minF, v);
                maxF = std::max(maxF, v);
                sumF += v;
                ++n;
            }
        }
        float avgF = (n > 0) ? (sumF / n) : 0.0f;
        if (n == 0) { minF = 0.0f; maxF = 60.0f; }

        ImGui::Text("FPS min/avg/max: %.1f / %.1f / %.1f", minF, avgF, maxF);

        if (showFpsGraph_ && fpsHistoryMax_ > 0) {
            float plotMin = std::max(0.0f, minF - 5.0f);
            float plotMax = std::max(maxF + 5.0f, 60.0f);
            ImGui::PlotLines("FPS History", fpsHistory_.data(), fpsHistoryMax_, fpsHistoryIndex_, nullptr, plotMin, plotMax, ImVec2(0,80));
        }

        ImGui::Separator();
        ImGui::Text("GPU: %s", glRenderer_.c_str());
        ImGui::Text("OpenGL: %s", glVersion_.c_str());

        // Per-stage timings (ms)
        if (!stageTimings_.empty()) {
            ImGui::Separator();
            ImGui::Text("Stage timings (ms):");
            for (const auto &p : stageTimings_) {
                ImGui::BulletText("%s: %.3f ms", p.first.c_str(), p.second);
            }
        }

        ImGui::Separator();
        ImGui::SliderFloat("FPS Smooth Alpha", &fpsSmoothingAlpha_, 0.0f, 1.0f);
        ImGui::Checkbox("Show FPS Graph", &showFpsGraph_);
    }
    ImGui::End();
}

void ImGuiDebugger::drawCameraPanel() {
    if (ImGui::Begin("Camera", &showCameraPanel_)) {
        ClampCurrentWindowToDisplay();
        ImGui::Text("Position:");
        ImGui::BulletText("X: %.2f", cameraX_);
        ImGui::BulletText("Y: %.2f", cameraY_);
        ImGui::BulletText("Z: %.2f", cameraZ_);
        
        const float kDegreesPerRadian = 180.0f / 3.14159265358979f;
        ImGui::Separator();
        ImGui::Text("Rotation:");
        ImGui::BulletText("Yaw: %.1f°", cameraYaw_ * kDegreesPerRadian);
        ImGui::BulletText("Pitch: %.1f°", cameraPitch_ * kDegreesPerRadian);
    }
    ImGui::End();
}

void ImGuiDebugger::drawRenderingPanel() {
    if (ImGui::Begin("Rendering", &showRenderingPanel_)) {
        ClampCurrentWindowToDisplay();
        bool enabled = depthTestEnabled_;
        if (ImGui::Checkbox("Depth Test", &enabled)) {
            setDepthTestEnabled(enabled);
        }
        ImGui::Text("Depth Test: %s", depthTestEnabled_ ? "Enabled" : "Disabled");

        if (ImGui::Checkbox("Face Culling", &cullFaceEnabled_)) {
            if (cullFaceEnabled_) glEnable(GL_CULL_FACE);
            else glDisable(GL_CULL_FACE);
        }

        if (ImGui::Checkbox("Wireframe", &wireframeEnabled_)) {
            if (wireframeEnabled_) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        ImGui::ColorEdit3("Clear Color", clearColor_);
        if (ImGui::Button("Apply Clear Color")) {
            glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], 1.0f);
        }

        if (ImGui::Checkbox("VSync", &vsyncEnabled_)) {
            int result = SDL_GL_SetSwapInterval(vsyncEnabled_ ? 1 : 0);
            if (result != 0) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "(Driver may not support)");
            }
        }

        ImGui::Separator();
        ImGui::Text("Post-Process Shader");
        if (postProcessor_) {
            bool ppEnabled = postProcessor_->isEnabled();
            if (ImGui::Checkbox("Enable Postprocess", &ppEnabled)) {
                postProcessor_->setEnabled(ppEnabled);
            }

            // Shader path input
            ImGui::InputText("Vertex Shader", vertShaderPath_, sizeof(vertShaderPath_));
            ImGui::InputText("Fragment Shader", fragShaderPath_, sizeof(fragShaderPath_));
            if (ImGui::Button("Load Shader")) {
                bool ok = postProcessor_->loadShaderFiles(std::string(vertShaderPath_), std::string(fragShaderPath_));
                postProcessLog_ = postProcessor_->getLastLog();
                if (!ok) {
                    // keep log for user
                }
            }
            if (!postProcessLog_.empty()) {
                ImGui::TextWrapped("%s", postProcessLog_.c_str());
            }

            // Effects
            const char* effects[] = { "None", "Grayscale", "Invert", "Sepia" };
            int cur = static_cast<int>(postProcessor_->effect());
            if (ImGui::Combo("Effect", &cur, effects, IM_ARRAYSIZE(effects))) {
                postProcessor_->setEffect(static_cast<engine::renderer::PostProcessor::Effect>(cur));
            }
            // Movement smoothing control (active if attached)
            if (movementSmoothingPtr_) {
                ImGui::Separator();
                ImGui::Text("Movement");
                ImGui::SliderFloat("Movement Smoothing", movementSmoothingPtr_, 0.0f, 50.0f);
            }
        } else {
            ImGui::Text("PostProcessor not attached to debugger.");
        }
    }
    ImGui::End();
}

void ImGuiDebugger::drawConsolePanel() {
    if (ImGui::Begin("Console", &showConsolePanel_)) {
        ClampCurrentWindowToDisplay();

        // Log area
        ImGui::BeginChild("ConsoleScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& line : consoleItems_) {
            ImGui::TextWrapped("%s", line.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        // Input
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
        if (ImGui::InputText("Input", consoleInput_, IM_ARRAYSIZE(consoleInput_), flags)) {
            std::string cmd(consoleInput_);
            if (!cmd.empty()) {
                addConsoleLog(std::string("> ") + cmd);
                if (commandCallback_) commandCallback_(cmd);
            }
            consoleInput_[0] = '\0';
        }

        ImGui::SameLine();
        if (ImGui::Button("Submit")) {
            std::string cmd(consoleInput_);
            if (!cmd.empty()) {
                addConsoleLog(std::string("> ") + cmd);
                if (commandCallback_) commandCallback_(cmd);
            }
            consoleInput_[0] = '\0';
        }
    }
    ImGui::End();
}

void ImGuiDebugger::addConsoleLog(const std::string& msg) {
    consoleItems_.push_back(msg);
    if (consoleItems_.size() > 256) consoleItems_.erase(consoleItems_.begin());
}

} // namespace debug
} // namespace engine
