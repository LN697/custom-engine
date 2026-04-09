#pragma once

namespace engine {

class Time {
public:
    Time();
    void tick(float delta);
    void increment_frame();
    int get_frame_count_and_reset();
    float uptime() const;

private:
    float uptime_;
    int frame_count_;
};

} // namespace engine
