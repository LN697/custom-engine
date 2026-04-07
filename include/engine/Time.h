#pragma once

namespace engine {

class Time {
public:
    Time();
    void tick(float delta);
    void incrementFrame();
    int getFrameCountAndReset();
    float uptime() const;

private:
    float uptime_;
    int frameCount_;
};

} // namespace engine
