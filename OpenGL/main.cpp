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

#include "j13.human.h" 
#include "teapot_loader.h"
#include "j17.bezier.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void init_sphere(float**, int*, int*);
void renderScene(const Shader& shader, unsigned int sphereVAO, int nSphereVert, Human& human, float t, CubicBezierCrv& spline);

void init_teapot();
void draw_teapot();
glm::vec3 getSplinePoint(CubicBezierCrv& spline, float t);

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 15.0f, 35.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int planeVAO, planeVBO;
unsigned int skyboxVAO, skyboxVBO;

unsigned int teapotVAO, teapotVBO;
int teapotVertexCount = 0;

glm::vec3 lightPos(-4.0f, 20.0f, -1.0f);
glm::vec3 light2Pos(4.0f, 15.0f, 5.0f);

unsigned int tex_sun, tex_mercury, tex_venus, tex_earth, tex_mars, tex_jupiter, tex_saturn, tex_uranus, tex_neptune;
unsigned int tex_floor, tex_floor_normal, cubemapTexture;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2171272 Kim Dohyun - Integrated Graphics Project", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("shader.vs", "shader.fs");

    CubicBezierCrv spline;

    float* sphereVertices = nullptr;
    int nSphereVert = 0, nSphereIndices = 0;
    init_sphere(&sphereVertices, &nSphereVert, &nSphereIndices);

    unsigned int sphereVAO, sphereVBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, nSphereVert * 8 * sizeof(float), sphereVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    float planeVertices[] = {
         25.0f, -0.01f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.01f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.01f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.01f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.01f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.01f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    tex_sun = loadTexture("../textures/solarsystem/2k_sun.jpg");
    tex_mercury = loadTexture("../textures/solarsystem/2k_mercury.jpg");
    tex_venus = loadTexture("../textures/solarsystem/2k_venus_atmosphere.jpg");
    tex_earth = loadTexture("../textures/solarsystem/2k_earth_daymap.jpg");
    tex_mars = loadTexture("../textures/solarsystem/2k_mars.jpg");
    tex_jupiter = loadTexture("../textures/solarsystem/2k_jupiter.jpg");
    tex_saturn = loadTexture("../textures/solarsystem/2k_saturn.jpg");
    tex_uranus = loadTexture("../textures/solarsystem/2k_uranus.jpg");
    tex_neptune = loadTexture("../textures/solarsystem/2k_neptune.jpg");
    tex_floor = loadTexture("../textures/wood.jpg");
    tex_floor_normal = loadTexture("../textures/wood_normal.jpg");

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
    shader.setInt("normalMap", 2);
    shader.setInt("skybox", 3);

    init_teapot();

    Human human;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 40.0f;
        lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        shader.use();
        shader.setBool("isDepthPass", true);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        renderScene(shader, sphereVAO, nSphereVert, human, currentFrame, spline);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shader.setBool("isDepthPass", false);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("light2Pos", light2Pos);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        shader.setVec3("light.position", lightPos);
        shader.setVec3("light.ambient", glm::vec3(0.2f));
        shader.setVec3("light.diffuse", glm::vec3(0.8f));
        shader.setVec3("light.specular", glm::vec3(1.0f));

        shader.setVec3("light2.position", light2Pos);
        shader.setVec3("light2.ambient", glm::vec3(0.1f));
        shader.setVec3("light2.diffuse", glm::vec3(0.4f));
        shader.setVec3("light2.specular", glm::vec3(0.5f));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, tex_floor_normal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        renderScene(shader, sphereVAO, nSphereVert, human, currentFrame, spline);

        glDepthFunc(GL_LEQUAL);
        shader.use();
        shader.setBool("isSkybox", true);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
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

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &teapotVAO);
    glDeleteBuffers(1, &teapotVBO);
    delete[] sphereVertices;

    glfwTerminate();
    return 0;
}

void renderScene(const Shader& shader, unsigned int sphereVAO, int nSphereVert, Human& human, float t, CubicBezierCrv& spline)
{
    shader.setBool("isFloor", true);
    shader.setBool("isTeapot", false);
    shader.setBool("isNormalMapped", true);
    shader.setBool("useReflection", false);
    shader.setBool("isCartoon", false);
    shader.setBool("useMultipleLights", true);
    shader.setBool("isStar", false);

    shader.setVec3("material.ambient", glm::vec3(1.0f));
    shader.setVec3("material.diffuse", glm::vec3(1.0f));
    shader.setVec3("material.specular", glm::vec3(0.2f));
    shader.setFloat("material.shininess", 32.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_floor);
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader.setBool("isFloor", false);
    shader.setBool("isNormalMapped", false);

    glm::mat4 baseSunModel = glm::mat4(1.0f);
    baseSunModel = glm::translate(baseSunModel, glm::vec3(0.0f, 6.0f, 0.0f));

    shader.setBool("isStar", true);
    glm::mat4 sunModel = glm::rotate(baseSunModel, t * glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    sunModel = glm::scale(sunModel, glm::vec3(2.5f));
    shader.setMat4("model", sunModel);
    glBindTexture(GL_TEXTURE_2D, tex_sun);
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);
    shader.setBool("isStar", false);

    glm::mat4 mercuryModel = glm::rotate(baseSunModel, t * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mercuryModel = glm::translate(mercuryModel, glm::vec3(4.0f, 0.0f, 0.0f));
    mercuryModel = glm::rotate(mercuryModel, t * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    mercuryModel = glm::scale(mercuryModel, glm::vec3(0.3f));
    shader.setMat4("model", mercuryModel);
    glBindTexture(GL_TEXTURE_2D, tex_mercury);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 venusModel = glm::rotate(baseSunModel, t * glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    venusModel = glm::translate(venusModel, glm::vec3(6.0f, 0.0f, 0.0f));
    venusModel = glm::rotate(venusModel, t * glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    venusModel = glm::scale(venusModel, glm::vec3(0.6f));
    shader.setMat4("model", venusModel);
    glBindTexture(GL_TEXTURE_2D, tex_venus);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 earthModel = glm::rotate(baseSunModel, t * glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    earthModel = glm::translate(earthModel, glm::vec3(8.5f, 0.0f, 0.0f));
    glm::mat4 earthRenderModel = glm::rotate(earthModel, t * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    earthRenderModel = glm::scale(earthRenderModel, glm::vec3(0.7f));
    shader.setMat4("model", earthRenderModel);
    glBindTexture(GL_TEXTURE_2D, tex_earth);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 moonModel = glm::rotate(earthModel, t * glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    moonModel = glm::translate(moonModel, glm::vec3(1.5f, 0.0f, 0.0f));
    moonModel = glm::scale(moonModel, glm::vec3(0.2f));
    shader.setMat4("model", moonModel);
    glBindTexture(GL_TEXTURE_2D, tex_mercury);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 marsModel = glm::rotate(baseSunModel, t * glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    marsModel = glm::translate(marsModel, glm::vec3(11.0f, 0.0f, 0.0f));
    marsModel = glm::rotate(marsModel, t * glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    marsModel = glm::scale(marsModel, glm::vec3(0.5f));
    shader.setMat4("model", marsModel);
    glBindTexture(GL_TEXTURE_2D, tex_mars);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 jupiterModel = glm::rotate(baseSunModel, t * glm::radians(12.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    jupiterModel = glm::translate(jupiterModel, glm::vec3(14.5f, 0.0f, 0.0f));
    jupiterModel = glm::rotate(jupiterModel, t * glm::radians(25.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    jupiterModel = glm::scale(jupiterModel, glm::vec3(1.4f));
    shader.setMat4("model", jupiterModel);
    glBindTexture(GL_TEXTURE_2D, tex_jupiter);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 saturnModel = glm::rotate(baseSunModel, t * glm::radians(9.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    saturnModel = glm::translate(saturnModel, glm::vec3(18.0f, 0.0f, 0.0f));
    saturnModel = glm::rotate(saturnModel, t * glm::radians(22.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    saturnModel = glm::scale(saturnModel, glm::vec3(1.2f));
    shader.setMat4("model", saturnModel);
    glBindTexture(GL_TEXTURE_2D, tex_saturn);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 uranusModel = glm::rotate(baseSunModel, t * glm::radians(6.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    uranusModel = glm::translate(uranusModel, glm::vec3(21.0f, 0.0f, 0.0f));
    uranusModel = glm::rotate(uranusModel, t * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    uranusModel = glm::scale(uranusModel, glm::vec3(0.9f));
    shader.setMat4("model", uranusModel);
    glBindTexture(GL_TEXTURE_2D, tex_uranus);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    glm::mat4 neptuneModel = glm::rotate(baseSunModel, t * glm::radians(4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    neptuneModel = glm::translate(neptuneModel, glm::vec3(24.0f, 0.0f, 0.0f));
    neptuneModel = glm::rotate(neptuneModel, t * glm::radians(12.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    neptuneModel = glm::scale(neptuneModel, glm::vec3(0.8f));
    shader.setMat4("model", neptuneModel);
    glBindTexture(GL_TEXTURE_2D, tex_neptune);
    glDrawArrays(GL_TRIANGLES, 0, nSphereVert);

    shader.setBool("isTeapot", true);
    shader.setBool("useReflection", true);
    shader.setVec3("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.setVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
    shader.setVec3("material.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("material.shininess", 64.0f);

    glm::mat4 teapotModel = glm::mat4(1.0f);
    teapotModel = glm::translate(teapotModel, glm::vec3(0.0f, 0.5f, -6.0f));
    teapotModel = glm::scale(teapotModel, glm::vec3(0.8f));
    shader.setMat4("model", teapotModel);
    draw_teapot();
    shader.setBool("isTeapot", false);
    shader.setBool("useReflection", false);

    shader.setBool("isCartoon", true);

    float splineT = fmod(t * 0.2f, 4.0f);
    glm::vec3 humanPos = getSplinePoint(spline, splineT);

    float delta = 0.01f;
    glm::vec3 nextPos = getSplinePoint(spline, fmod(splineT + delta, 4.0f));
    glm::vec3 humanTangent = glm::normalize(nextPos - humanPos);

    float angle = atan2(humanTangent.x, humanTangent.z);

    glm::mat4 humanModel = glm::mat4(1.0f);
    humanModel = glm::translate(humanModel, humanPos);
    humanModel = glm::translate(humanModel, glm::vec3(0.0f, 1.8f, 0.0f));
    humanModel = glm::rotate(humanModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    humanModel = glm::scale(humanModel, glm::vec3(0.25f));

    float anim = fmod(t * 4.0f, 4.0f);
    int step = (int)anim;
    float alpha = anim - step;

    if (step == 0)      human.MixPose(base, walk1, alpha);
    else if (step == 1) human.MixPose(walk1, base, alpha);
    else if (step == 2) human.MixPose(base, walk2, alpha);
    else                human.MixPose(walk2, base, alpha);

    human.DrawHuman(const_cast<Shader&>(shader), sphereVAO, nSphereVert, humanModel);
    shader.setBool("isCartoon", false);
}

void init_teapot()
{
    std::vector<float> data;
    Teapot teapot("../teapot.vbo", data, 8);

    if (data.empty()) {
        std::cout << "ERROR: teapot.vbo 데이터를 찾을 수 없습니다!" << std::endl;
        teapotVertexCount = 0;
        return;
    }

    teapotVertexCount = teapot.nVertNum;
    glGenVertexArrays(1, &teapotVAO);
    glGenBuffers(1, &teapotVBO);

    glBindVertexArray(teapotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, teapotVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

    glBindVertexArray(0);
}

void draw_teapot()
{
    if (teapotVertexCount > 0)
    {
        glBindVertexArray(teapotVAO);
        glDrawArrays(GL_TRIANGLES, 0, teapotVertexCount);
        glBindVertexArray(0);
    }
}

glm::vec3 getSplinePoint(CubicBezierCrv& spline, float t)
{
    int i = (int)t;
    float u = t - i;

    if (i < 0) { i = 0; u = 0.0f; }
    if (i > 3) { i = 3; u = 1.0f; }

    glm::vec3 p0 = spline.P[i];
    glm::vec3 p1 = spline.P[i + 1];
    glm::vec3 p2 = spline.P[i + 2];
    glm::vec3 p3 = spline.P[i + 3];

    float u2 = u * u;
    float u3 = u2 * u;

    glm::vec3 res = 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * u +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3
        );

    return res;
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)      format = GL_RED;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void init_sphere(float** vertices, int* nVert, int* nIndices)
{
    int segments = 40;
    std::vector<float> vData;
    float pi = acosf(-1.0f);

    for (int i = 0; i < segments; ++i) {
        float u = (float)i / segments * 2.0f * pi;
        float du = 1.0f / segments * 2.0f * pi;
        for (int j = 0; j < segments; ++j) {
            float v = -pi / 2.0f + (float)j / segments * pi;
            float dv = 1.0f / segments * pi;

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
    *nVert = segments * segments * 6;
    *vertices = new float[vData.size()];
    std::copy(vData.begin(), vData.end(), *vertices);
}