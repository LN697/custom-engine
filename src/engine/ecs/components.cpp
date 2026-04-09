#include "engine/ecs/components.h"
#include <glm/gtc/matrix_transform.hpp>

namespace engine {
namespace ecs {

glm::mat4 TransformComponent::get_transform_matrix() const {
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scal = glm::scale(glm::mat4(1.0f), scale);
    return trans * rotZ * rotY * rotX * scal;
}

} // namespace ecs
} // namespace engine
