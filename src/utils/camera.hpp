#ifndef GL_CAMERA
#define GL_CAMERA

#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Camera
{
public:
    glm::vec3 position = glm::vec3(0.0, 0.0, -5.0);
    glm::vec3 localUp = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 front = glm::vec3(0.0, 0.0, 1.0);
    glm::vec3 right = glm::vec3(0.0, 0.0, 0.0);
    glm::mat4 lookat = glm::mat4(1.0);
    
    Camera();
    void Update();
};

#endif // !GL_CAMERA
