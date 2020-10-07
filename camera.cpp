#include "camera.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

Camera::Camera(glm::mat4* view, glm::vec3 forward, glm::vec3 up, glm::vec3 pos) : view(view), forward(forward), pos(pos), up(up) {
    *view = glm::lookAt(pos, forward + pos, up);

}

void Camera::updateView() {
    *view = glm::lookAt(pos, pos + forward, up);
}

void Camera::moveForward() {
    pos = pos + sensitivity * forward;
    updateView();
}
