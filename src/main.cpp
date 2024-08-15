#define CAM_SPEED 50

#include "camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/geometric.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <perlin.hpp>

#include <vector>
#include <unordered_map>
#include <window.hpp>
#include <cstdio>
#include <shader.hpp>
#include <camera.hpp>
#include <chunk.hpp>
#include <random>
#include <iostream>

// Options
// ----------------------------------------------
int screenHeight = 1920;
int screenWidth = 1080;

float camSpeed = CAM_SPEED;
// ----------------------------------------------

float deltaTime = 1;
float lastFrame = glfwGetTime();
float currentFrame;

float xoffset;
float yoffset;

float pitch, yaw;
float lastX = static_cast<float>(screenWidth / 2), lastY = static_cast<float>(screenHeight / 2);

bool fullscr = false;

void processInput(GLFWwindow* window);
void viewportSizeChanged(GLFWwindow* window, int width, int height);
void mouseUpdate(GLFWwindow* window, double xpos, double ypos);

Camera playerCam;

int main(int argc, char* argv[]) {

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

    constexpr float cubeVerts[] =
    {
    0.0f, 0.0f, 0.0f,      //111 -- 0
    1.0f, 0.0f, 0.0f,     //011 -- 1
    1.0f, 0.0f, 1.0f,    //010 -- 2
    0.0f, 0.0f, 1.0f,     //110 -- 3

    1.0f, 1.0f, 0.0f,    //001 -- 4
    1.0f, 1.0f, 1.0f,   //000 -- 5
    0.0f, 1.0f, 1.0f,    //100 -- 6
    0.0f, 1.0f, 0.0f,     //101 -- 7
    };

    constexpr GLuint indices[] =
    {
        // Top
        0, 1, 2,
        2, 3, 0,

        // front
        3, 2, 5,
        5, 6, 3,

        // right
        7, 0, 3,
        3, 6, 7,

        // back
        4, 1, 0,
        0, 7, 4,

        // left
        5, 2, 1,
        1, 4, 5,

        // bottom
        4, 7, 6,
        6, 5, 4
    };

    GLuint VBO, VAO, EBO;

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), &cubeVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // REST OF CODE

    std::unordered_map<glm::vec3, std::vector<unsigned int>, vecKeyTrait, vecKeyTrait> chunks;

    for (int x = 0; x < 10; x++)
    {
        for (int y = 0; y < 2; y++)
        {
            for (int z = 0; z < 10; z++)
            {
                chunks[glm::vec3(x, y, z)] = loadChunk(glm::vec3(x,y,z));

            }
        }
    }

    GLuint voxelBuffer;
    glGenBuffers(1, &voxelBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, voxelBuffer);

    Shader myShader = Shader("../../src/Shaders/vertex.glsl", "../../src/Shaders/fragment.glsl");

    // Please ignore the 10/10 error handling
    if (glGetError() != GL_NO_ERROR)
    {
        std::cerr << "AN ERROR HAS OCCURED SOMEWHERE!\n";
    }

    myShader.use();

    glfwSetInputMode(myWin.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glm::mat4 perspective = glm::perspective(glm::radians(90.0f), (float)1920 / (float)1080, 0.01f, 1000.0f);
    glm::mat4 model = glm::mat4(1.0);

    model = glm::scale(model, glm::vec3(8));

    int count = 0;

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

        myShader.setMat4("perspective", perspective); // Persp
        myShader.setVec3("camPos", playerCam.position);
        myShader.setMat4("view", playerCam.lookat); // View

        myShader.setMat4("iViewMat", glm::inverse(playerCam.lookat));
        myShader.setMat4("iProjMat", glm::inverse(perspective));

        glBindVertexArray(VAO);

        glm::vec3 camVoxelSpace = glm::vec3((int)(playerCam.position.x / 16), 0, (int)(playerCam.position.z / 16));
        std::cout << camVoxelSpace.x << " " << camVoxelSpace.z << "\n";
        for (int x = -32; x <= 32; x++)
        {
            for (int z = -32; z <= 32; z++)
            {
                glm::vec3 currentPos = camVoxelSpace + glm::vec3(x, 0, z);
                if (chunks.find(currentPos) == chunks.end())
                {
                    chunks[currentPos] = loadChunk(currentPos);
                    camVoxelSpace.y = 1;
                    chunks[currentPos] = loadChunk(currentPos);
                }
            }
        }

        for (auto& currentChunk : chunks)
        {
            if (glm::distance(currentChunk.first, camVoxelSpace) > 5) continue;
            model = glm::mat4(1.0);
            model = glm::scale(model, glm::vec3(16));
            model = glm::translate(model, currentChunk.first);

            myShader.setMat4("model", model);
            myShader.setMat4("iModelMat", glm::inverse(model));
            myShader.setVec3("chunkPos", currentChunk.first);

            glBufferData(GL_SHADER_STORAGE_BUFFER, currentChunk.second.size() * sizeof(unsigned int), &currentChunk.second[0], GL_STATIC_DRAW);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }



        processInput(myWin.getWindow());

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
        else if (!fullscr) {
            std::cout << "Windowed\n";
            glfwSetWindowMonitor(window, NULL, 200, 200, 600, 600, 0);
        }
    }

    camSpeed = CAM_SPEED * deltaTime;
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