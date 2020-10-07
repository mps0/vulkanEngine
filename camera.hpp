#ifndef CAMERA_HPP 
#define CAMERA_HPP 

#include<glm/glm.hpp>
#include <glm/ext.hpp>

class Camera{
    public:
    Camera(glm::mat4* view, glm::vec3 forward = glm::vec3(0.f, 0.f, -1.f), glm::vec3 up = glm::vec3(0.f, 1.f, 0.f), glm::vec3 pos = glm::vec3(0.f, 0.f, 0.f));

    void updateView();

    void moveForward();

    private:
    glm::mat4* view;
    glm::vec3 forward, up, pos;
    float sensitivity = 0.01f;

};







#endif
