#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "j17.bezier.h"
#include "j13.human.h"
#include "teapot_loader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void init_sphere(float**, int*, int*);
void renderScene(const Shader& shader, unsigned int sphereVAO, int nSphereVert, unsigned int cubeVAO, Human& human, float t, CubicBezierCrv& launchSpline, int state);
void init_teapot();
void draw_teapot();
glm::vec3 getSplinePoint(CubicBezierCrv& spline, float t);

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 1000;

Camera camera(glm::vec3(0.0f, 36.0f, 26.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -65.0f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int planeVAO, planeVBO;
unsigned int skyboxVAO, skyboxVBO;
unsigned int teapotVAO, teapotVBO;
int teapotVertexCount = 0;
unsigned int cubeVAO, cubeVBO;

glm::vec3 lightPos(0.0f, 25.0f, -5.0f);
glm::vec3 light2Pos(10.0f, 15.0f, 10.0f);

unsigned int tex_floor, cubemapTexture;

enum GameState { READY, LAUNCHING, PLAYING, RETURNING };
GameState gameState = READY;
float stateTimer = 0.0f;

glm::vec3 ballStartPos(12.5f, 0.8f, 16.0f);
glm::vec3 ballPosition = ballStartPos;
glm::vec3 ballVelocity(0.0f, 0.0f, 0.0f);
glm::vec3 pinballGravity(0.0f, 0.0f, 18.0f);
const float RADIUS = 0.4f;

glm::vec3 humanPos(12.5f, 0.0f, 17.0f);
float humanAngle = 0.0f;
CubicBezierCrv humanReturnSpline;
CubicBezierCrv launchSpline;

float leftFlipperAngle = -30.0f;
float rightFlipperAngle = 30.0f;
bool isLeftPressed = false;
bool isRightPressed = false;

glm::vec3 leftFlipperPos(-6.0f, 0.8f, 15.0f);
glm::vec3 rightFlipperPos(2.0f, 0.8f, 15.0f);

struct Bumper {
    glm::vec3 pos;
    float radius;
    float power;
    unsigned int tex;
    bool isTeapot;
    int satelliteCount;
};
std::vector<Bumper> bumpers;

int main()
{
    srand((unsigned int)time(NULL));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2171272 Kim Dohyun", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    Shader shader("shader.vs", "shader.fs");

    float* sphereVertices = nullptr;
    int nSphereVert = 0, nSphereIndices = 0;
    init_sphere(&sphereVertices, &nSphereVert, &nSphereIndices);
    unsigned int sphereVAO, sphereVBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, nSphereVert * 8 * sizeof(float), sphereVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    float planeVertices[] = {
         15.0f, 0.0f,  20.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -15.0f, 0.0f,  20.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -15.0f, 0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         15.0f, 0.0f,  20.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
        -15.0f, 0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         15.0f, 0.0f, -20.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, -1.0f
    };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4); glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    float cubeVertices[] = {
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  1.0f, 1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 1.0f,
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 1.0f,
         0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,  1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,  0.0f, 1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    tex_floor = loadTexture("../textures/wall.jpg");

    std::vector<std::string> faces{
        "../textures/skybox/right.png", "../textures/skybox/left.png",
        "../textures/skybox/top.png",   "../textures/skybox/bottom.png",
        "../textures/skybox/front.png",  "../textures/skybox/back.png"
    };
    cubemapTexture = loadCubemap(faces);

    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 1);
    shader.setInt("skybox", 3);

    init_teapot();
    Human human;

    launchSpline.P[0] = glm::vec3(12.5f, 0.8f, 16.0f);
    launchSpline.P[1] = glm::vec3(12.5f, 0.8f, -10.0f);
    launchSpline.P[2] = glm::vec3(12.5f, 0.8f, -17.5f);
    launchSpline.P[3] = glm::vec3(-2.0f, 0.8f, -17.5f);
    launchSpline.P[4] = glm::vec3(-12.5f, 0.8f, -17.5f);
    launchSpline.P[5] = glm::vec3(-12.5f, 0.8f, -10.0f);
    launchSpline.P[6] = glm::vec3(-12.5f, 0.8f, 0.0f);

    bumpers.push_back({ glm::vec3(-2.0f, 0.8f, -3.0f), 2.5f, 20.0f, 0, false, 3 });
    bumpers.push_back({ glm::vec3(-9.0f, 0.8f, -9.0f), 1.5f, 15.0f, 0, true, 0 });
    bumpers.push_back({ glm::vec3(5.0f, 0.8f, -10.0f), 1.8f, 16.0f, 0, false, 2 });
    bumpers.push_back({ glm::vec3(-10.0f, 0.8f, 1.0f), 1.2f, 12.0f, 0, false, 1 });
    bumpers.push_back({ glm::vec3(6.0f, 0.8f, 2.0f), 1.4f, 14.0f, 0, true, 0 });
    bumpers.push_back({ glm::vec3(-6.0f, 0.8f, 8.0f), 0.9f, 10.0f, 0, false, 0 });
    bumpers.push_back({ glm::vec3(2.0f, 0.8f, 8.0f), 1.0f, 11.0f, 0, false, 2 });

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (gameState == READY)
        {
            ballPosition = ballStartPos;
            ballVelocity = glm::vec3(0.0f);
            humanPos = glm::vec3(12.5f, 0.0f, 17.0f);
            humanAngle = 0.0f;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                gameState = LAUNCHING;
                stateTimer = 0.0f;
            }
        }
        else if (gameState == LAUNCHING)
        {
            stateTimer += deltaTime * 2.2f;
            if (stateTimer >= 3.99f) {
                gameState = PLAYING;
                ballPosition = getSplinePoint(launchSpline, 3.99f);
                ballVelocity = glm::vec3(0.0f, 0.0f, 15.0f);
                humanPos = glm::vec3(0.0f, 0.0f, 18.5f);
                humanAngle = 0.0f;
            }
            else {
                ballPosition = getSplinePoint(launchSpline, stateTimer);
                ballVelocity = glm::vec3(0.0f);

                float progress = std::min(stateTimer / 3.99f, 1.0f);
                glm::vec3 hStart(12.5f, 0.0f, 17.0f);
                glm::vec3 hEnd(0.0f, 0.0f, 18.5f);
                humanPos = glm::mix(hStart, hEnd, progress);
                glm::vec3 hDir = glm::normalize(hEnd - hStart);
                humanAngle = atan2(hDir.x, hDir.z);
            }
        }
        else if (gameState == PLAYING)
        {
            ballVelocity += pinballGravity * deltaTime;
            ballPosition += ballVelocity * deltaTime;

            if (ballPosition.x < -13.5f + RADIUS) {
                ballPosition.x = -13.5f + RADIUS;
                ballVelocity.x *= -0.7f;
            }
            if (ballPosition.x > 13.5f - RADIUS) {
                ballPosition.x = 13.5f - RADIUS;
                ballVelocity.x *= -0.7f;
            }
            if (ballPosition.z < -18.5f + RADIUS) {
                ballPosition.z = -18.5f + RADIUS;
                ballVelocity.z *= -0.7f;
            }

            if (ballPosition.z >= -16.5f && ballPosition.z <= 7.5f) {
                if (ballPosition.x > 9.5f - RADIUS && ballPosition.x < 10.5f) {
                    ballPosition.x = 9.5f - RADIUS;
                    ballVelocity.x *= -0.7f;
                }
                else if (ballPosition.x < 11.5f + RADIUS && ballPosition.x > 10.5f) {
                    ballPosition.x = 11.5f + RADIUS;
                    ballVelocity.x *= -0.7f;
                }
            }

            if (ballPosition.z > -17.5f && ballPosition.z < -16.5f + RADIUS) {
                if (ballPosition.x > 9.5f && ballPosition.x < 11.5f) {
                    ballPosition.z = -16.5f + RADIUS;
                    if (ballVelocity.z > 0) ballVelocity.z *= -0.7f;
                }
            }

            if (ballPosition.z >= 7.0f && ballPosition.z <= 15.0f) {
                float t = (ballPosition.z - 7.0f) / 8.0f;
                float leftX = -14.0f + t * 8.0f;
                if (ballPosition.x < leftX + RADIUS + 0.3f && ballPosition.x > leftX - 2.0f) {
                    ballPosition.x = leftX + RADIUS + 0.3f;
                    glm::vec2 n = glm::normalize(glm::vec2(1.0f, -1.0f));
                    float dot = ballVelocity.x * n.x + ballVelocity.z * n.y;
                    if (dot < 0) {
                        ballVelocity.x -= 2.0f * dot * n.x;
                        ballVelocity.z -= 2.0f * dot * n.y;
                        ballVelocity *= 0.7f;
                    }
                }

                float rightX = 10.0f - t * 8.0f;
                if (ballPosition.x > rightX - RADIUS - 0.3f && ballPosition.x < rightX + 2.0f) {
                    ballPosition.x = rightX - RADIUS - 0.3f;
                    glm::vec2 n = glm::normalize(glm::vec2(-1.0f, -1.0f));
                    float dot = ballVelocity.x * n.x + ballVelocity.z * n.y;
                    if (dot < 0) {
                        ballVelocity.x -= 2.0f * dot * n.x;
                        ballVelocity.z -= 2.0f * dot * n.y;
                        ballVelocity *= 0.7f;
                    }
                }
            }

            for (auto& b : bumpers) {
                if (glm::distance(ballPosition, b.pos) < RADIUS + b.radius) {
                    glm::vec3 n = glm::normalize(ballPosition - b.pos);
                    ballVelocity = n * b.power;
                    ballPosition = b.pos + n * (RADIUS + b.radius + 0.1f);
                }

                if (b.satelliteCount > 0) {
                    for (int i = 0; i < b.satelliteCount; ++i) {
                        float orbitAngle = currentFrame * 2.0f + (i * 6.283185f / b.satelliteCount);
                        float orbitRadius = b.radius + 1.2f;
                        float satRadius = b.radius * 0.3f;

                        glm::vec3 satPos = b.pos + glm::vec3(cos(orbitAngle) * orbitRadius, 0.0f, -sin(orbitAngle) * orbitRadius);

                        if (glm::distance(ballPosition, satPos) < RADIUS + satRadius) {
                            glm::vec3 n = glm::normalize(ballPosition - satPos);
                            ballVelocity = n * (b.power * 0.7f);
                            ballPosition = satPos + n * (RADIUS + satRadius + 0.1f);
                        }
                    }
                }
            }

            if (ballPosition.z > 14.0f && ballPosition.z < 17.5f) {
                float lAngle = glm::radians(leftFlipperAngle);
                glm::vec2 lStart(-6.0f, 15.0f);
                glm::vec2 lDir(cos(lAngle), -sin(lAngle));
                glm::vec2 bPos(ballPosition.x, ballPosition.z);
                glm::vec2 vL = bPos - lStart;
                float tL = glm::dot(vL, lDir);

                if (tL > -0.5f && tL < 3.5f) {
                    glm::vec2 closest = lStart + lDir * tL;
                    float dist = glm::length(bPos - closest);
                    if (dist < RADIUS + 0.6f) {
                        glm::vec2 normal = bPos - closest;
                        glm::vec2 fixedNormal(lDir.y, -lDir.x);
                        if (glm::length(normal) > 0.0001f) {
                            normal = glm::normalize(normal);
                            if (glm::dot(normal, fixedNormal) < 0) normal = -normal;
                        }
                        else {
                            normal = fixedNormal;
                        }

                        ballPosition.x = closest.x + normal.x * (RADIUS + 0.6f);
                        ballPosition.z = closest.y + normal.y * (RADIUS + 0.6f);

                        if (isLeftPressed && ballVelocity.z > 0) {
                            ballVelocity.x = normal.x * 45.0f + 5.0f;
                            ballVelocity.z = normal.y * 45.0f;
                        }
                        else {
                            float dot = ballVelocity.x * normal.x + ballVelocity.z * normal.y;
                            if (dot < 0) {
                                ballVelocity.x -= 1.0f * dot * normal.x;
                                ballVelocity.z -= 1.0f * dot * normal.y;
                            }
                        }
                    }
                }

                float rAngle = glm::radians(rightFlipperAngle);
                glm::vec2 rStart(2.0f, 15.0f);
                glm::vec2 rDir(-cos(rAngle), sin(rAngle));
                glm::vec2 bPos2(ballPosition.x, ballPosition.z);
                glm::vec2 vR = bPos2 - rStart;
                float tR = glm::dot(vR, rDir);

                if (tR > -0.5f && tR < 3.5f) {
                    glm::vec2 closest = rStart + rDir * tR;
                    float dist = glm::length(bPos2 - closest);
                    if (dist < RADIUS + 0.6f) {
                        glm::vec2 normal = bPos2 - closest;
                        glm::vec2 fixedNormal(-rDir.y, rDir.x);
                        if (glm::length(normal) > 0.0001f) {
                            normal = glm::normalize(normal);
                            if (glm::dot(normal, fixedNormal) < 0) normal = -normal;
                        }
                        else {
                            normal = fixedNormal;
                        }

                        ballPosition.x = closest.x + normal.x * (RADIUS + 0.6f);
                        ballPosition.z = closest.y + normal.y * (RADIUS + 0.6f);

                        if (isRightPressed && ballVelocity.z > 0) {
                            ballVelocity.x = normal.x * 45.0f - 5.0f;
                            ballVelocity.z = normal.y * 45.0f;
                        }
                        else {
                            float dot = ballVelocity.x * normal.x + ballVelocity.z * normal.y;
                            if (dot < 0) {
                                ballVelocity.x -= 1.0f * dot * normal.x;
                                ballVelocity.z -= 1.0f * dot * normal.y;
                            }
                        }
                    }
                }
            }

            humanPos = glm::vec3(0.0f, 0.0f, 18.5f);
            humanAngle = 0.0f;

            if (ballPosition.z > 19.5f) {
                gameState = RETURNING;
                stateTimer = 0.0f;
                humanReturnSpline.P[0] = humanPos;
                humanReturnSpline.P[1] = glm::vec3(ballPosition.x, 0.0f, ballPosition.z);
                humanReturnSpline.P[2] = glm::vec3(ballPosition.x, 0.0f, ballPosition.z);
                humanReturnSpline.P[3] = glm::vec3(0.0f, 0.0f, 19.0f);
                humanReturnSpline.P[4] = glm::vec3(12.5f, 0.0f, 19.0f);
                humanReturnSpline.P[5] = glm::vec3(12.5f, 0.0f, 17.0f);
                humanReturnSpline.P[6] = glm::vec3(12.5f, 0.0f, 17.0f);
            }
        }
        else if (gameState == RETURNING)
        {
            stateTimer += deltaTime * 1.3f;
            if (stateTimer >= 3.99f) {
                gameState = READY;
            }
            else {
                glm::vec3 nextPos = getSplinePoint(humanReturnSpline, glm::min(stateTimer + 0.1f, 3.99f));
                humanPos = getSplinePoint(humanReturnSpline, stateTimer);
                glm::vec3 dir = glm::normalize(nextPos - humanPos);

                if (glm::length(dir) > 0.001f) {
                    humanAngle = atan2(dir.x, dir.z);
                }
                if (stateTimer < 1.33f) {
                    ballVelocity = glm::vec3(0.0f);
                }
                else {
                    ballPosition = humanPos + glm::vec3(0.0f, 1.8f, 0.0f) + dir * 1.5f;
                    ballVelocity = glm::vec3(0.0f);
                }
            }
        }

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 50.0f;
        lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        shader.use();
        shader.setBool("isDepthPass", true);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderScene(shader, sphereVAO, nSphereVert, cubeVAO, human, currentFrame, launchSpline, gameState);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shader.setBool("isDepthPass", false);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("light2Pos", light2Pos);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient", glm::vec3(0.4f));
        shader.setVec3("light.diffuse", glm::vec3(0.8f));
        shader.setVec3("light.specular", glm::vec3(1.0f));
        shader.setVec3("light2.position", light2Pos);
        shader.setVec3("light2.ambient", glm::vec3(0.1f));
        shader.setVec3("light2.diffuse", glm::vec3(0.5f));
        shader.setVec3("light2.specular", glm::vec3(0.6f));

        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        renderScene(shader, sphereVAO, nSphereVert, cubeVAO, human, currentFrame, launchSpline, gameState);

        glDepthFunc(GL_LEQUAL);
        shader.setBool("isSkybox", true);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        view = view * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        shader.setBool("isSkybox", false);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO); glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &skyboxVAO); glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &teapotVAO); glDeleteBuffers(1, &teapotVBO);
    glDeleteVertexArrays(1, &cubeVAO); glDeleteBuffers(1, &cubeVBO);
    delete[] sphereVertices;

    glfwTerminate();
    return 0;
}

void renderScene(const Shader& shader, unsigned int sphereVAO, int nSphereVert, unsigned int cubeVAO, Human& human, float t, CubicBezierCrv& launchSpline, int state)
{
    shader.setBool("isFloor", true);
    shader.setBool("isTeapot", false);
    shader.setBool("isNormalMapped", false);
    shader.setBool("isSolidColor", false);
    shader.setVec3("material.ambient", glm::vec3(1.0f));
    shader.setVec3("material.diffuse", glm::vec3(1.0f));
    shader.setVec3("material.specular", glm::vec3(0.2f));
    shader.setFloat("material.shininess", 16.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_floor);
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader.setBool("isFloor", false);
    shader.setBool("isNormalMapped", false);
    shader.setBool("isSolidColor", true);
    shader.setVec3("solidColor", glm::vec3(0.3f, 0.3f, 0.8f));

    glm::mat4 leftOuterWall = glm::translate(glm::mat4(1.0f), glm::vec3(-14.5f, 1.0f, 0.0f));
    leftOuterWall = glm::scale(leftOuterWall, glm::vec3(1.0f, 2.0f, 40.0f));
    shader.setMat4("model", leftOuterWall);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::mat4 rightOuterWall = glm::translate(glm::mat4(1.0f), glm::vec3(14.5f, 1.0f, 0.0f));
    rightOuterWall = glm::scale(rightOuterWall, glm::vec3(1.0f, 2.0f, 40.0f));
    shader.setMat4("model", rightOuterWall);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::mat4 topWall = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -19.5f));
    topWall = glm::scale(topWall, glm::vec3(30.0f, 2.0f, 1.0f));
    shader.setMat4("model", topWall);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::mat4 rightInnerWall = glm::translate(glm::mat4(1.0f), glm::vec3(10.5f, 1.0f, -4.5f));
    rightInnerWall = glm::scale(rightInnerWall, glm::vec3(1.0f, 2.0f, 23.0f));
    shader.setMat4("model", rightInnerWall);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::mat4 leftFunnel = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 1.0f, 11.0f));
    leftFunnel = glm::rotate(leftFunnel, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    leftFunnel = glm::scale(leftFunnel, glm::vec3(1.0f, 2.0f, 11.31f));
    shader.setMat4("model", leftFunnel);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glm::mat4 rightFunnel = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 1.0f, 11.0f));
    rightFunnel = glm::rotate(rightFunnel, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rightFunnel = glm::scale(rightFunnel, glm::vec3(1.0f, 2.0f, 11.31f));
    shader.setMat4("model", rightFunnel);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    for (auto& b : bumpers) {
        glm::mat4 bModel = glm::mat4(1.0f);
        bModel = glm::translate(bModel, b.pos);

        if (b.isTeapot) {
            bModel = glm::scale(bModel, glm::vec3(b.radius * 2.4f));
            bModel = glm::rotate(bModel, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            shader.setMat4("model", bModel);

            shader.setBool("isSolidColor", true);
            shader.setVec3("solidColor", glm::vec3(0.9f));
            shader.setBool("isTeapot", true);
            shader.setBool("useReflection", true);
            shader.setVec3("material.ambient", glm::vec3(0.2f));
            shader.setVec3("material.diffuse", glm::vec3(0.5f));
            shader.setVec3("material.specular", glm::vec3(1.0f));
            shader.setFloat("material.shininess", 128.0f);

            draw_teapot();

            shader.setBool("isTeapot", false);
            shader.setBool("useReflection", false);
            shader.setBool("isSolidColor", false);
        }
        else {
            bModel = glm::scale(bModel, glm::vec3(b.radius));
            shader.setMat4("model", bModel);

            shader.setBool("isSolidColor", true);
            shader.setVec3("solidColor", glm::vec3(0.9f));
            shader.setBool("useReflection", true);

            glBindVertexArray(sphereVAO);
            glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

            shader.setBool("useReflection", false);
            shader.setBool("isSolidColor", false);
        }

        if (b.satelliteCount > 0) {
            for (int i = 0; i < b.satelliteCount; ++i) {
                float orbitAngle = t * 2.0f + (i * 6.283185f / b.satelliteCount);
                float orbitRadius = b.radius + 1.2f;
                float rotationAngle = t * 5.0f;
                float satRadius = b.radius * 0.3f;

                glm::mat4 satModel = glm::mat4(1.0f);
                satModel = glm::translate(satModel, b.pos);
                satModel = glm::rotate(satModel, orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                satModel = glm::translate(satModel, glm::vec3(orbitRadius, 0.0f, 0.0f));
                satModel = glm::rotate(satModel, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
                satModel = glm::scale(satModel, glm::vec3(satRadius));

                shader.setMat4("model", satModel);
                shader.setBool("isSolidColor", true);

                if (i % 3 == 0) shader.setVec3("solidColor", glm::vec3(0.4f, 0.8f, 0.4f));
                else if (i % 3 == 1) shader.setVec3("solidColor", glm::vec3(0.8f, 0.4f, 0.4f));
                else shader.setVec3("solidColor", glm::vec3(0.4f, 0.4f, 0.8f));

                shader.setBool("useReflection", true);
                glBindVertexArray(sphereVAO);
                glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

                shader.setBool("useReflection", false);
                shader.setBool("isSolidColor", false);
            }
        }
    }

    shader.setBool("isSolidColor", true);
    shader.setVec3("solidColor", glm::vec3(1.0f, 0.2f, 0.2f));
    glm::mat4 lFlip = glm::translate(glm::mat4(1.0f), leftFlipperPos);
    lFlip = glm::rotate(lFlip, glm::radians(leftFlipperAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lFlip = glm::translate(lFlip, glm::vec3(1.5f, 0.0f, 0.0f));
    lFlip = glm::scale(lFlip, glm::vec3(4.0f, 1.0f, 0.5f));
    shader.setMat4("model", lFlip);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.setVec3("solidColor", glm::vec3(0.0f, 0.8f, 1.0f));
    glm::mat4 rFlip = glm::translate(glm::mat4(1.0f), rightFlipperPos);
    rFlip = glm::rotate(rFlip, glm::radians(rightFlipperAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    rFlip = glm::translate(rFlip, glm::vec3(-1.5f, 0.0f, 0.0f));
    rFlip = glm::scale(rFlip, glm::vec3(4.0f, 1.0f, 0.5f));
    shader.setMat4("model", rFlip);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.setBool("isSolidColor", true);
    shader.setVec3("solidColor", glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setBool("useReflection", true);
    glm::mat4 ballModelObj = glm::mat4(1.0f);
    ballModelObj = glm::translate(ballModelObj, ballPosition);
    ballModelObj = glm::scale(ballModelObj, glm::vec3(RADIUS));
    shader.setMat4("model", ballModelObj);
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
    shader.setBool("useReflection", false);

    if (state == RETURNING || state == READY || state == LAUNCHING || state == PLAYING) {
        shader.setBool("isSolidColor", true);
        shader.setVec3("solidColor", glm::vec3(1.0f, 0.9f, 0.1f));
        shader.setVec3("material.ambient", glm::vec3(0.2f));
        shader.setVec3("material.diffuse", glm::vec3(0.8f));
        shader.setVec3("material.specular", glm::vec3(0.5f));
        shader.setFloat("material.shininess", 16.0f);
        glm::mat4 humanModelObj = glm::mat4(1.0f);
        humanModelObj = glm::translate(humanModelObj, humanPos);
        humanModelObj = glm::translate(humanModelObj, glm::vec3(0.0f, 2.0f, 0.0f));
        humanModelObj = glm::rotate(humanModelObj, humanAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        humanModelObj = glm::scale(humanModelObj, glm::vec3(0.4f));

        float animSpeed = 0.0f;
        if (state == RETURNING || state == LAUNCHING) animSpeed = 14.0f;

        float anim = fmod(t * animSpeed, 4.0f);
        int step = (int)anim;
        float alpha = anim - step;

        if (state == READY || state == PLAYING) human.MixPose(base, base, 0.0f);
        else if (step == 0) human.MixPose(base, walk1, alpha);
        else if (step == 1) human.MixPose(walk1, base, alpha);
        else if (step == 2) human.MixPose(base, walk2, alpha);
        else                human.MixPose(walk2, base, alpha);

        human.DrawHuman(const_cast<Shader&>(shader), sphereVAO, nSphereVert, humanModelObj);
        shader.setBool("isSolidColor", false);
    }
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        leftFlipperAngle = std::min(leftFlipperAngle + 620.0f * deltaTime, 30.0f);
        isLeftPressed = true;
    }
    else {
        leftFlipperAngle = std::max(leftFlipperAngle - 620.0f * deltaTime, -30.0f);
        isLeftPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        rightFlipperAngle = std::max(rightFlipperAngle - 620.0f * deltaTime, -30.0f);
        isRightPressed = true;
    }
    else {
        rightFlipperAngle = std::min(rightFlipperAngle + 620.0f * deltaTime, 30.0f);
        isRightPressed = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn); float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
    lastX = xpos; lastY = ypos;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll(static_cast<float>(yoffset)); }

unsigned int loadTexture(char const* path) {
    unsigned int textureID; glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB; if (nrComponents == 1) format = GL_RED; else if (nrComponents == 4) format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID; glGenTextures(1, &textureID); glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) { glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); stbi_image_free(data); }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return textureID;
}

void init_sphere(float** vertices, int* nVert, int* nIndices) {
    int segments = 40; std::vector<float> vData; float pi = acosf(-1.0f);
    for (int i = 0; i < segments; ++i) {
        float u = (float)i / segments * 2.0f * pi; float du = 1.0f / segments * 2.0f * pi;
        for (int j = 0; j < segments; ++j) {
            float v = -pi / 2.0f + (float)j / segments * pi; float dv = 1.0f / segments * pi;
            float x1 = cosf(v) * cosf(u); float y1 = cosf(v) * sinf(u); float z1 = sinf(v);
            float x2 = cosf(v) * cosf(u + du); float y2 = cosf(v) * sinf(u + du); float z2 = sinf(v);
            float x3 = cosf(v + dv) * cosf(u); float y3 = cosf(v + dv) * sinf(u); float z3 = sinf(v + dv);
            float x4 = cosf(v + dv) * cosf(u + du); float y4 = cosf(v + dv) * sinf(u + du); float z4 = sinf(v + dv);
            vData.push_back(x1); vData.push_back(y1); vData.push_back(z1); vData.push_back(x1); vData.push_back(y1); vData.push_back(z1); vData.push_back((float)i / segments); vData.push_back((float)j / segments);
            vData.push_back(x2); vData.push_back(y2); vData.push_back(z2); vData.push_back(x2); vData.push_back(y2); vData.push_back(z2); vData.push_back((float)(i + 1) / segments); vData.push_back((float)j / segments);
            vData.push_back(x3); vData.push_back(y3); vData.push_back(z3); vData.push_back(x3); vData.push_back(y3); vData.push_back(z3); vData.push_back((float)i / segments); vData.push_back((float)(j + 1) / segments);
            vData.push_back(x2); vData.push_back(y2); vData.push_back(z2); vData.push_back(x2); vData.push_back(y2); vData.push_back(z2); vData.push_back((float)(i + 1) / segments); vData.push_back((float)j / segments);
            vData.push_back(x4); vData.push_back(y4); vData.push_back(z4); vData.push_back(x4); vData.push_back(y4); vData.push_back(z4); vData.push_back((float)(i + 1) / segments); vData.push_back((float)(j + 1) / segments);
            vData.push_back(x3); vData.push_back(y3); vData.push_back(z3); vData.push_back(x3); vData.push_back(y3); vData.push_back(z3); vData.push_back((float)i / segments); vData.push_back((float)(j + 1) / segments);
        }
    }
    *nVert = segments * segments * 6; *vertices = new float[vData.size()]; std::copy(vData.begin(), vData.end(), *vertices);
}

void init_teapot() {
    std::vector<float> data; Teapot teapot("teapot.vbo", data, 8);
    if (!data.empty()) {
        teapotVertexCount = teapot.nVertNum; glGenVertexArrays(1, &teapotVAO); glGenBuffers(1, &teapotVBO);
        glBindVertexArray(teapotVAO); glBindBuffer(GL_ARRAY_BUFFER, teapotVBO); glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); glBindVertexArray(0);
    }
}

void draw_teapot() { if (teapotVertexCount > 0) { glBindVertexArray(teapotVAO); glDrawArrays(GL_TRIANGLES, 0, teapotVertexCount); glBindVertexArray(0); } }

glm::vec3 getSplinePoint(CubicBezierCrv& spline, float t) {
    int i = (int)t; float u = t - i;
    if (i < 0) { i = 0; u = 0.0f; } if (i > 3) { i = 3; u = 1.0f; }
    glm::vec3 p0 = spline.P[i]; glm::vec3 p1 = spline.P[i + 1]; glm::vec3 p2 = spline.P[i + 2]; glm::vec3 p3 = spline.P[i + 3];
    float u2 = u * u; float u3 = u2 * u;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3);
}