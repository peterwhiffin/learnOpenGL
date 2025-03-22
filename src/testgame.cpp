#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "shader.hpp"
#include "stb_image.h"
#include "camera.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void createVAO(float *vertices, int vertSize, unsigned int *indices, int indSize, unsigned int *vao, unsigned int *vbo, unsigned int *ebo);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(char const *path);

unsigned int cubeVAO;
unsigned int VBO;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool filled = false;
float viewAngle = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400, lastY = 300;
float yaw = -90, pitch = 0;
float fov = 67.0f;
float cubeSpeed = 1.0f;
glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
glm::vec3 cubePos = glm::vec3(0.0f, 0.0f, 0.0f);
Camera cam(glm::vec3(0.0f, 0.0f, 10.0f));
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

glm::vec3 sunRot = glm::vec3(0.0f, 0.0f, 0.0f);
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Pete's graphics", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(0);

    stbi_set_flip_vertically_on_load(true);
    unsigned int diffuseMap = loadTexture("X:/repos/learnOpenGL/src/resources/container2.png");
    unsigned int specularMap = loadTexture("X:/repos/learnOpenGL/src/resources/container2_specular.png");

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    std::cout << sizeof(float) * 3;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader litShader("X:/repos/learnOpenGL/src/shaders/defaultshader.vs", "X:/repos/learnOpenGL/src/shaders/defaultshader.fs");
    Shader lightSourceShader("X:/repos/learnOpenGL/src/shaders/defaultshader.vs", "X:/repos/learnOpenGL/src/shaders/lightsourceshader.fs");

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f, 0.2f, 2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f, 2.0f, -12.0f),
        glm::vec3(0.0f, 0.0f, -3.0f)};

    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(2.0f);
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.17f);
    glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);

    litShader.use();
    litShader.setFloat("material.shininess", 128.0f);
    litShader.setInt("material.diffuse", 0);
    litShader.setInt("material.specular", 1);

    litShader.setVec3("dirLight.ambient", ambientColor);
    litShader.setVec3("dirLight.diffuse", diffuseColor);
    litShader.setVec3("dirLight.specular", specularColor);

    litShader.setVec3("pointLights[0].position", lightPos);
    litShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    litShader.setVec3("pointLights[2].position", pointLightPositions[2]);
    litShader.setVec3("pointLights[3].position", pointLightPositions[3]);

    litShader.setVec3("pointLights[0].ambient", ambientColor);
    litShader.setVec3("pointLights[0].diffuse", diffuseColor);
    litShader.setVec3("pointLights[0].specular", specularColor);
    litShader.setVec3("pointLights[1].ambient", ambientColor);
    litShader.setVec3("pointLights[1].diffuse", diffuseColor);
    litShader.setVec3("pointLights[1].specular", specularColor);
    litShader.setVec3("pointLights[2].ambient", ambientColor);
    litShader.setVec3("pointLights[2].diffuse", diffuseColor);
    litShader.setVec3("pointLights[2].specular", specularColor);
    litShader.setVec3("pointLights[3].ambient", ambientColor);
    litShader.setVec3("pointLights[3].diffuse", diffuseColor);
    litShader.setVec3("pointLights[3].specular", specularColor);

    litShader.setFloat("pointLights[0].constant", 1.0f);
    litShader.setFloat("pointLights[0].linear", 0.09f);
    litShader.setFloat("pointLights[0].quadratic", 0.032f);
    litShader.setFloat("pointLights[1].constant", 1.0f);
    litShader.setFloat("pointLights[1].linear", 0.09f);
    litShader.setFloat("pointLights[1].quadratic", 0.032f);
    litShader.setFloat("pointLights[2].constant", 1.0f);
    litShader.setFloat("pointLights[2].linear", 0.09f);
    litShader.setFloat("pointLights[2].quadratic", 0.032f);
    litShader.setFloat("pointLights[3].constant", 1.0f);
    litShader.setFloat("pointLights[3].linear", 0.09f);
    litShader.setFloat("pointLights[3].quadratic", 0.032f);

    litShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    litShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
    litShader.setVec3("spotLight.ambient", ambientColor);
    litShader.setVec3("spotLight.diffuse", diffuseColor);
    litShader.setVec3("spotLight.specular", specularColor);
    litShader.setFloat("spotLight.constant", 1.0f);
    litShader.setFloat("spotLight.linear", 0.09f);
    litShader.setFloat("spotLight.quadratic", 0.032f);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = cam.GetViewMatrix();
        projection = glm::perspective(glm::radians(cam.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

        litShader.use();
        litShader.setFloat("time", (float)glfwGetTime());

        litShader.setMat4("projection", projection);
        litShader.setMat4("view", view);
        litShader.setMat4("model", model);

        litShader.setVec3("viewPos", cam.Position);
        litShader.setMat4("normalMat", glm::transpose(glm::inverse(model)));

        litShader.setVec3("dirLight.direction", sunRot);

        litShader.setVec3("spotLight.position", cam.Position);
        litShader.setVec3("spotLight.direction", cam.Front);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::translate(model, glm::vec3(-2.0f, 1.0f, -7.0f));
        litShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        litShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        sunRot.y = glm::min(sunRot.y + .25f * deltaTime, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        sunRot.y = glm::max(sunRot.y - .25f * deltaTime, -1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        sunRot.x = glm::min(sunRot.x + .25f * deltaTime, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        sunRot.x = glm::max(sunRot.x - .25f * deltaTime, -1.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        sunRot = -cam.Front;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    cam.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    cam.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

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