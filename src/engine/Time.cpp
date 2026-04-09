#include "engine/time.h"

namespace engine {

Time::Time() : uptime_(0.0f), frame_count_(0) {}

void Time::tick(float delta) {
    uptime_ += delta;
}

void Time::increment_frame() {
    ++frame_count_;
}

int Time::get_frame_count_and_reset() {
    int v = frame_count_;
    frame_count_ = 0;
    return v;
}

float Time::uptime() const {
    return uptime_;
}

} // namespace engine
