#pragma once

#include <functional>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <mutex>

namespace engine {

// Small, header-only EventBus for decoupled messaging between systems.
class EventBus {
public:
    template<typename Event>
    using Handler = std::function<void(const Event&)>;

    template<typename Event>
    void subscribe(Handler<Event> handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& vec = handlers_[std::type_index(typeid(Event))];
        // wrap handler to accept void*
        vec.emplace_back([h = std::move(handler)](const void* e){ h(*static_cast<const Event*>(e)); });
    }

    template<typename Event>
    void publish(const Event& ev) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(std::type_index(typeid(Event)));
        if (it == handlers_.end()) return;
        for (const auto& fn : it->second) fn(&ev);
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> handlers_;
};

} // namespace engine
