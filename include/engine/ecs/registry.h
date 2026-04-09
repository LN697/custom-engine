#pragma once

#include "engine/ecs/entity.h"
#include "engine/ecs/components.h"
#include <unordered_map>
#include <memory>
#include <any>
#include <vector>

namespace engine {
namespace ecs {

class Registry {
public:
    Registry();
    ~Registry();

    Entity create();
    void destroy(Entity entity);

    template<typename T, typename... Args>
    T& emplace(Entity entity, Args&&... args) {
        auto& type_vec = get_or_create_storage<T>();
        T component(std::forward<Args>(args)...);
        type_vec.emplace_back(entity, component);
        return std::any_cast<T&>(type_vec.back().second);
    }

    template<typename T>
    T* try_get(Entity entity) {
        auto it = storages_.find(typeid(T).hash_code());
        if (it == storages_.end()) return nullptr;
        
        auto& type_vec = it->second;
        for (auto& pair : type_vec) {
            if (pair.first == entity) {
                return &std::any_cast<T&>(pair.second);
            }
        }
        return nullptr;
    }

    template<typename T, typename Func>
    void view(Func&& func) {
        auto it = storages_.find(typeid(T).hash_code());
        if (it == storages_.end()) return;
        
        auto& type_vec = it->second;
        for (const auto& entity : entities_) {
            if (valid(entity)) {
                auto* component = try_get<T>(entity);
                if (component) {
                    func(entity, *component);
                }
            }
        }
    }

    bool valid(Entity e) const;
    void clear();

private:
    template<typename T>
    std::vector<std::pair<Entity, std::any>>& get_or_create_storage() {
        auto hash = typeid(T).hash_code();
        if (storages_.find(hash) == storages_.end()) {
            storages_[hash] = std::vector<std::pair<Entity, std::any>>();
        }
        return storages_[hash];
    }

    Entity nextId_;
    std::vector<Entity> entities_;
    std::unordered_map<size_t, std::vector<std::pair<Entity, std::any>>> storages_;
};

} // namespace ecs
} // namespace engine
