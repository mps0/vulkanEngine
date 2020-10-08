#include "camera.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

Camera::Camera(glm::mat4* view, glm::vec3 forward, glm::vec3 up, glm::vec3 pos) : view(view), forward(forward), pos(pos), up(up) {

    updateView();
}

void Camera::updateView() {
    *view = glm::lookAt(pos, pos + forward, up);
}

void Camera::moveForward() {
    pos = pos + sensitivity * forward;
    updateView();
}

void Camera::moveBackward() {
    pos = pos - sensitivity * forward;
    updateView();
}

void Camera::moveRight() {
    glm::vec3 u = glm::cross(forward, up);
    pos = pos + sensitivity * u;
    updateView();
}

void Camera::moveLeft() {
    glm::vec3 u = glm::cross(forward, up);
    pos = pos - sensitivity * u;
    updateView();
}

void Camera::pitch(int yrel) {
    float rotationY = mouseSensitivity * yrel;
    glm::vec3 newForward = cosf(rotationY) * forward + sinf(rotationY) * up;
    up = cosf(rotationY) * up - sinf(rotationY) * forward;
    forward = newForward;

    updateView();
}

void Camera::yaw(int xrel) {
    glm::vec3 u = glm::cross(forward, up);

    float rotationX = -mouseSensitivity * xrel;
    forward = cosf(rotationX) * forward - sinf(rotationX) * u;
}

void Camera::roll(int xrel) {
    glm::vec3 u = glm::cross(forward, up);

    float rotationX = mouseSensitivity * xrel;
    up = cosf(rotationX) * up - sinf(rotationX) * u;
    updateView();
}
