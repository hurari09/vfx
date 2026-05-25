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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void init_sphere(float**, int*, int*);
void renderScene(const Shader& shader, unsigned int sphereVAO, int nSphereVert, Human& human, float t);

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;

Camera camera(glm::vec3(0.0f, 5.0f, 25.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int planeVAO;
glm::vec3 lightPos(-2.0f, 15.0f, -1.0f);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2171272 Kim Dohyun - Final Project", NULL, NULL);
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

    Shader mainShader("shader.vs", "shader.fs");
    Shader simpleDepthShader("80.1.shadow_mapping_depth.vs", "80.1.shadow_mapping_depth.fs");
    Shader skyboxShader("60.1.skybox.vs", "60.1.skybox.fs");

    float planeVertices[] = {
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    unsigned int planeVBO;
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
    glBindVertexArray(0);

    float* sphereVerts = NULL;
    int nSphereVert, nSphereAttr;
    init_sphere(&sphereVerts, &nSphereVert, &nSphereAttr);

    unsigned int sphereVBO, sphereVAO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, nSphereVert * nSphereAttr * sizeof(float), sphereVerts, GL_STATIC_DRAW);
    glBindVertexArray(sphereVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, nSphereAttr * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
    free(sphereVerts);

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    unsigned int woodTexture = loadTexture("..\\textures\\wood2.jpg");
    unsigned int woodNormal = loadTexture("..\\textures\\wood_normal.jpg");

    std::vector<std::string> faces{
        "..\\textures\\right.png", "..\\textures\\left.png",
        "..\\textures\\top.png",   "..\\textures\\bottom.png",
        "..\\textures\\front.png", "..\\textures\\back.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
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

    mainShader.use();
    mainShader.setInt("diffuseTexture", 0);
    mainShader.setInt("shadowMap", 1);
    mainShader.setInt("normalMap", 2);
    mainShader.setInt("skybox", 3);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    Human human;
    static float t = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        t += deltaTime;

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 40.0f;
        lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;

        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        renderScene(simpleDepthShader, sphereVAO, nSphereVert, human, t);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        mainShader.setMat4("projection", projection);
        mainShader.setMat4("view", view);

        mainShader.setVec3("viewPos", camera.Position);
        mainShader.setVec3("lightPos", lightPos);
        mainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, woodNormal);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        renderScene(mainShader, sphereVAO, nSphereVert, human, t);

        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

void renderScene(const Shader& shader, unsigned int sphereVAO, int nSphereVert, Human& human, float t)
{
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    shader.setBool("isFloor", true);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader.setBool("isFloor", false);
    float radius = 8.0f;
    float speed = 0.5f;
    float angle = t * speed;

    glm::vec3 pelvisPos;
    pelvisPos.x = cos(angle) * radius;
    pelvisPos.z = sin(angle) * radius;
    pelvisPos.y = 3.5f;

    model = glm::mat4(1.0f);
    model = glm::translate(model, pelvisPos);
    model = glm::rotate(model, glm::radians(-glm::degrees(angle)), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f));

    float anim = fmod(t * 3.0f, 4.0f);
    int step = (int)anim;
    float alpha = anim - step;

    if (step == 0)      human.MixPose(base, walk1, alpha);
    else if (step == 1) human.MixPose(walk1, base, alpha);
    else if (step == 2) human.MixPose(base, walk2, alpha);
    else                human.MixPose(walk2, base, alpha);

    human.DrawHuman(const_cast<Shader&>(shader), sphereVAO, nSphereVert, model);
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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
    camera.ProcessMouseScroll(yoffset);
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
        std::cout << "Texture failed to load at path: " << path << std::endl;
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
            GLenum format = GL_RGB;
            if (nrChannels == 4) format = GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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

void init_sphere(float** vertices, int* nVert, int* nAttr)
{
    float pi = acosf(-1.0f);
    float pi2 = 2.0f * pi;
    int nu = 40, nv = 20;
    const double du = pi2 / nu;
    const double dv = pi / nv;

    *nVert = (nv - 1) * nu * 6;
    *nAttr = 8;
    *vertices = (float*)malloc(sizeof(float) * (*nVert) * (*nAttr));

    float u, v;
    int k = 0;
    for (v = (-0.5f) * pi + dv; v < 0.5f * pi - dv; v += dv)
    {
        for (u = 0.0f; u < pi2; u += du)
        {
            (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
            (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
            (*vertices)[k++] = u / pi2;                 (*vertices)[k++] = (v + 0.5f * pi) / pi;

            (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
            (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
            (*vertices)[k++] = (u + du) / pi2;           (*vertices)[k++] = (v + 0.5f * pi) / pi;

            (*vertices)[k++] = cosf(v + dv) * cosf(u);   (*vertices)[k++] = cosf(v + dv) * sinf(u);   (*vertices)[k++] = sinf(v + dv);
            (*vertices)[k++] = cosf(v + dv) * cosf(u);   (*vertices)[k++] = cosf(v + dv) * sinf(u);   (*vertices)[k++] = sinf(v + dv);
            (*vertices)[k++] = u / pi2;                 (*vertices)[k++] = (v + dv + 0.5f * pi) / pi;

            (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
            (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
            (*vertices)[k++] = (u + du) / pi2;           (*vertices)[k++] = (v + 0.5f * pi) / pi;

            (*vertices)[k++] = cosf(v + dv) * cosf(u + du); (*vertices)[k++] = cosf(v + dv) * sinf(u + du); (*vertices)[k++] = sinf(v + dv);
            (*vertices)[k++] = cosf(v + dv) * cosf(u + du); (*vertices)[k++] = cosf(v + dv) * sinf(u + du); (*vertices)[k++] = sinf(v + dv);
            (*vertices)[k++] = (u + du) / pi2;           (*vertices)[k++] = (v + dv + 0.5f * pi) / pi;

            (*vertices)[k++] = cosf(v + dv) * cosf(u);   (*vertices)[k++] = cosf(v + dv) * sinf(u);   (*vertices)[k++] = sinf(v + dv);
            (*vertices)[k++] = cosf(v + dv) * cosf(u);   (*vertices)[k++] = cosf(v + dv) * sinf(u);   (*vertices)[k++] = sinf(v + dv);
            (*vertices)[k++] = u / pi2;                 (*vertices)[k++] = (v + dv + 0.5f * pi) / pi;
        }
    }
    for (u = 0.0f; u < pi2; u += du)
    {
        v = 0.5f * pi - dv;
        (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = u / pi2;                 (*vertices)[k++] = (v + 0.5f * pi) / pi;

        (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = (u + du) / pi2;           (*vertices)[k++] = (v + 0.5f * pi) / pi;

        v = 0.5f * pi;
        (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = (u + du) / pi2;           (*vertices)[k++] = 1.0f;

        v = (-0.5f) * pi;
        (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = u / pi2;                 (*vertices)[k++] = 0.0f;

        v = (-0.5f) * pi + dv;
        (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = cosf(v) * cosf(u + du);   (*vertices)[k++] = cosf(v) * sinf(u + du);   (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = (u + du) / pi2;           (*vertices)[k++] = (v + 0.5f * pi) / pi;

        (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = cosf(v) * cosf(u);       (*vertices)[k++] = cosf(v) * sinf(u);       (*vertices)[k++] = sinf(v);
        (*vertices)[k++] = u / pi2;                 (*vertices)[k++] = (v + 0.5f * pi) / pi;
    }
}