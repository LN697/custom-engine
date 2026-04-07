#include "engine/Time.h"

namespace engine {

Time::Time() : uptime_(0.0f), frameCount_(0) {}

void Time::tick(float delta) {
    uptime_ += delta;
}

void Time::incrementFrame() {
    ++frameCount_;
}

int Time::getFrameCountAndReset() {
    int v = frameCount_;
    frameCount_ = 0;
    return v;
}

float Time::uptime() const {
    return uptime_;
}

} // namespace engine
