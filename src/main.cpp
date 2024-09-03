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
#include <queue>
#include <unordered_set >

#include <window.hpp>
#include <cstdio>
#include <shader.hpp>
#include <camera.hpp>
#include <chunk.hpp>
#include <random>
#include <iostream>

#include <thread>
#include <mutex>
#include <condition_variable>

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
bool wrFrm = false;

void processInput(GLFWwindow* window);
void viewportSizeChanged(GLFWwindow* window, int width, int height);
void mouseUpdate(GLFWwindow* window, double xpos, double ypos);

std::unordered_map<glm::ivec3, std::vector<unsigned int>, vecKeyTrait, vecKeyTrait> chunks;
std::unordered_set<glm::ivec3, vecKeyTrait, vecKeyTrait> processingChunks;
std::queue<glm::ivec3> chunksToLoad;

std::mutex loadChunkMutex;
bool stopWork = false;
std::condition_variable mutex_condition;

Camera playerCam;

int workerCount = (std::thread::hardware_concurrency() <= 1 ? 1 : std::thread::hardware_concurrency()) - 1;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
}

void ChunkUpdate()
{
    while (true)
    {
        glm::ivec3 chunkToLoad;
        {
            std::unique_lock<std::mutex> lock(loadChunkMutex);
            mutex_condition.wait(lock, []
                {
                    return !chunksToLoad.empty() || stopWork;
                });

            if (stopWork) return;
            chunkToLoad = chunksToLoad.front();
            chunksToLoad.pop();
        }

        std::vector<unsigned int> chunkInfo = loadChunk(chunkToLoad);

        {
            std::unique_lock<std::mutex> lock(loadChunkMutex);
            chunks[chunkToLoad] = std::move(chunkInfo);
            processingChunks.erase(processingChunks.find(chunkToLoad));
        }
    }
}

int main(int argc, char* argv[]) {

    std::cout << "WorkerCount: " << workerCount << '\n';
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
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    float frame[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frame), &frame, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // create compute output texture
    GLuint outTexture;
    glCreateTextures(GL_TEXTURE_2D, 1, &outTexture);

    glTextureParameteri(outTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(outTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(outTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(outTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTextureStorage2D(outTexture, 1, GL_RGBA32F, 1920, 1080);

    glBindImageTexture(0, outTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindTextureUnit(0, outTexture);

    Shader myShader = Shader("../../src/Shaders/vertex.glsl", "../../src/Shaders/fragment.glsl");
    Shader compute = Shader("../../src/Shaders/compute.glsl");

    glfwSetInputMode(myWin.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    glm::mat4 perspective = glm::perspective(glm::radians(90.0f), (float)1920 / (float)1080, 0.01f, 1000.0f);
    glm::mat4 model = glm::mat4(1.0);

    glm::vec3 lastCamVoxSpace = glm::vec3((int)(playerCam.position.x / 16), 0, (int)(playerCam.position.z / 16));

    model = glm::scale(model, glm::vec3(8));

    int count = 0;

    std::cout << "Starting chunk loading thread..." << std::endl;

    std::cout << "Starting program loop...\n";
    while (!myWin.getWindowCloseState())
    {
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.4f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        compute.use();
        glDispatchCompute((unsigned int)ceil(1920 / 8), (unsigned int)ceil(1080 / 4), 1);

        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glm::vec3 camVoxelSpace = glm::vec3((int)(playerCam.position.x / 16), 0, (int)(playerCam.position.z / 16));       

        playerCam.Update();

        compute.setMat4("InvView", glm::inverse(playerCam.lookat));
        compute.setMat4("InvPerspective",glm::inverse(perspective));
        compute.setVec3("camPos", playerCam.position);

        myShader.use();

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        lastCamVoxSpace = camVoxelSpace;

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

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        if (!wrFrm)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
           
        wrFrm = !wrFrm;            
    }

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