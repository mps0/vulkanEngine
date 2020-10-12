#ifndef CAMERA_HPP 
#define CAMERA_HPP 

#include<glm/glm.hpp>
#include <glm/ext.hpp>


//assumes up and forward are perpinduclar and normalized
class Camera{
    public:
    Camera(glm::mat4* view, glm::vec3 forward = glm::vec3(0.f, 0.f, -1.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), glm::vec3 pos = glm::vec3(0.f, 0.f, 0.f));

    void updateView();

    void moveForward();
    void moveBackward();
    void moveRight();
    void moveLeft();

    void pitch(int yrel);
    void yaw(int xrel);
    void roll(int xrel);

    private:
    glm::mat4* view;
    glm::vec3 forward, up, pos;
    glm::vec3 u, v, w;
    float sensitivity = 0.1f;
    float mouseSensitivity = 0.01f;

};







#endif
