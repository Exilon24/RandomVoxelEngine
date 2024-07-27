// TODO: Use bounding boxes and raymarch inside them. Cull unseen boxes. 


#include "camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <window.hpp>
#include <cstdio>
#include <shader.hpp>
#include <camera.hpp>

#include <iostream>

// Options
// ----------------------------------------------
int screenHeight = 1920;
int screenWidth = 1080;

float camSpeed = 0.02;
// ----------------------------------------------

float deltaTime = 1;
float lastFrame = glfwGetTime();
float currentFrame;

float xoffset;
float yoffset;

float pitch, yaw;
float lastX = static_cast<float>(screenWidth / 2), lastY = static_cast<float>( screenHeight/ 2);

bool fullscr = false;

void processInput(GLFWwindow* window);
void viewportSizeChanged(GLFWwindow* window, int width, int height);
void mouseUpdate(GLFWwindow* window, double xpos, double ypos);
    
Camera playerCam;
glm::vec3 camDirection;
glm::mat3 viewMat;

int main (int argc, char *argv[]) {

    // Create window
    Window myWin = Window(600, 620, "Awesome sauce");
    myWin.makeWindowContextCurrent();
    
    std::cout << "Loading glad...\n";
    // Load glad function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Glad initialized succesfully\n";
    }
    // Set callbacks
    glfwSetFramebufferSizeCallback(myWin.getWindow(), viewportSizeChanged);
    glfwSetCursorPosCallback(myWin.getWindow(), mouseUpdate);  

    glEnable(GL_DEPTH_TEST);

    // INITIALIZE FRAME
    constexpr float cubeVerts[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

    GLuint VBO,VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), &cubeVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        
    // REST OF CODE
    
    std::vector<unsigned int> voxels;
    for (int i = 0; i < 16; i++)
    {
        voxels.push_back((unsigned int) 0xffffffff);
    }


    GLuint voxelBuffer;
    glGenBuffers(1, &voxelBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, voxels.size() * sizeof(unsigned int), &voxels[0], GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voxelBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    Shader myShader = Shader("../../src/Shaders/vertex.glsl", "../../src/Shaders/fragment.glsl");


    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "AN ERROR HAS OCCURED SOMEWHERE!\n";
    }

    // TODO add hasDiff to the material object and add a check for if textures are present; 
    myShader.use();
    myShader.setInt("voxelCount", voxels.size());

    glfwSetInputMode(myWin.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    
    glm::mat4 perspective = glm::perspective(glm::radians(90.0f), (float)1920 / (float)1080, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0);
    glm::mat4 view = glm::mat4(1.0);

    std::cout << "Starting program loop...\n";
    while (!myWin.getWindowCloseState())
    {

        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.4f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        
        playerCam.Update(); 

        myShader.use();
        myShader.setFloat("tickingAway", glfwGetTime());

        myShader.setMat4("model", model);
        myShader.setMat4("perspective", perspective); // Persp
        myShader.setVec3("camPos", playerCam.position);
        myShader.setMat4("camDirection", playerCam.lookat); // View

        processInput(myWin.getWindow());

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glfwSwapBuffers(myWin.getWindow());
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        GLFWmonitor* monitor = glfwGetWindowMonitor(window);
        if (monitor == NULL && !fullscr)
        {
            std::cout << "Fullscr\n";
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            fullscr = true;
        }
        else if (!fullscr){
            std::cout << "Windowed\n";
            glfwSetWindowMonitor(window, NULL, 200, 200, 600, 600, 0);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
        fullscr = false;    

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        playerCam.position += playerCam.front * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        playerCam.position -= playerCam.front * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        playerCam.position += playerCam.right * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        playerCam.position -= playerCam.right * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        playerCam.position += playerCam.localUp * camSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        playerCam.position -= playerCam.localUp * camSpeed;

}

void viewportSizeChanged(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
}

void mouseUpdate(GLFWwindow* window, double xpos, double ypos)
{
    xoffset = xpos - lastX;
    yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;
    
    const float sensitivity = 0.2;// * deltaTime;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch >= 89.9)
        pitch = 89.9;
    else if (pitch <= -89.9)
        pitch = -89.9;

    playerCam.front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    playerCam.front.y = sin(glm::radians(pitch));
    playerCam.front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    playerCam.front = glm::normalize(playerCam.front);


}