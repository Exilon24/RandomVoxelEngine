#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <camera.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);

Camera::Camera()
{
    
    right = glm::normalize(glm::cross(up, front));
    localUp = glm::normalize(glm::cross(front, right));
    lookat = glm::mat4(1.0);
}

void Camera::Update()
{
    right = glm::normalize(glm::cross(up, front));
    localUp = glm::normalize(glm::cross(front, right));
    lookat = glm::lookAt(position, position + front, localUp);
}
