#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace engine {
namespace renderer { class PostProcessor; }
}

namespace engine {
namespace debug {

class ImGuiDebugger {
public:
    ImGuiDebugger();
    ~ImGuiDebugger();

    bool initialize(SDL_Window* window, SDL_GLContext glContext);
    void shutdown();
    void beginFrame();
    void endFrame();
    void render();
    void drawUI();  // Draw all debug UI panels

    // Update camera debug info
    void setCameraPosition(float x, float y, float z);
    void setCameraRotation(float yaw, float pitch);
    void setFrameStats(float fps, int frameCount, float uptime);
    void setGLInfo(const char* version, const char* renderer);
    void setDepthTestEnabled(bool enabled);

    // Hook to control a post-process pass
    void setPostProcessor(engine::renderer::PostProcessor* pp) { postProcessor_ = pp; }

    // Attach a pointer to the engine's movement smoothing variable so the UI can edit it
    void attachMovementSmoothing(float* ptr) { movementSmoothingPtr_ = ptr; }

    // Debug features control
    void setShowPerformancePanel(bool show) { showPerformancePanel_ = show; }
    void setShowCameraPanel(bool show) { showCameraPanel_ = show; }
    void setShowRenderingPanel(bool show) { showRenderingPanel_ = show; }
    void setShowDemoWindow(bool show) { showDemoWindow_ = show; }

    bool isShowingPerformancePanel() const { return showPerformancePanel_; }
    bool isShowingCameraPanel() const { return showCameraPanel_; }
    bool isShowingRenderingPanel() const { return showRenderingPanel_; }

private:
    void drawPerformancePanel();
    void drawCameraPanel();
    void drawRenderingPanel();

    // State
    bool initialized_;
    SDL_Window* window_;
    SDL_GLContext glContext_;

    // Camera data
    float cameraX_, cameraY_, cameraZ_;
    float cameraYaw_, cameraPitch_;

    // Frame stats
    float fps_;
    int frameCount_;
    float uptime_;
    std::string glVersion_;
    std::string glRenderer_;
    bool depthTestEnabled_;

    // UI visibility
    bool showPerformancePanel_;
    bool showCameraPanel_;
    bool showRenderingPanel_;
    bool showDemoWindow_;
    
    // Post-processor integration
    engine::renderer::PostProcessor* postProcessor_;
    char vertShaderPath_[260];
    char fragShaderPath_[260];
    std::string postProcessLog_;

    // FPS history and smoothing
    std::vector<float> fpsHistory_;
    int fpsHistoryIndex_;
    int fpsHistoryMax_;
    float fpsSmoothed_;
    float fpsSmoothingAlpha_;
    bool showFpsGraph_;

    // Rendering toggles
    bool wireframeEnabled_;
    bool cullFaceEnabled_;
    bool vsyncEnabled_;
    float clearColor_[3];
    
    // Movement smoothing pointer (owned by Engine)
    float* movementSmoothingPtr_;
};

} // namespace debug
} // namespace engine
