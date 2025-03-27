#include "camera.hpp"
#include "input.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "stb_image.h"
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <complex>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void DrawScene(Camera *cam);
void CreateModel(char *path, Shader *newShader);
void processInput(GLFWwindow *window, Camera *cam);
void mouse_callback(GLFWwindow *window);
bool filled = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int screenWidth = 800;
int screenHeight = 600;
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 sunRot = glm::vec3(0.0f, 0.0f, 0.0f);
std::vector<Model> models;
Camera *mainCamera;
float frameTimeSum = 0.0f;
int frameCount = 0;
float lastX = 0.0f;
float lastY = 0.0f;
int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(screenWidth, screenHeight, "Pete's graphics", NULL, NULL);

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
  // glfwSetCursorPosCallback(window, mouse_callback);
  glfwSwapInterval(0);
  InputHandler input(window);

  Camera cam(&input, (float)screenWidth / screenHeight, glm::vec3(0.0f, 0.0f, 10.0f));
  mainCamera = &cam;
  stbi_set_flip_vertically_on_load(true);
  Shader litShader("X:/repos/learnOpenGL/src/shaders/defaultshader.vs",
                   "X:/repos/learnOpenGL/src/shaders/defaultshader.fs");

  Shader debugShader("X:/repos/learnOpenGL/src/shaders/debugshader.vs",
                     "X:/repos/learnOpenGL/src/shaders/debugshader.fs");

  CreateModel("X:/Repos/learnOpenGL/src/Resources/backpack/backpack.obj", &litShader);
  CreateModel("X:/Repos/learnOpenGL/src/Resources/backpack/backpack.obj", &litShader);
  CreateModel("X:/Repos/learnOpenGL/src/Resources/backpack/backpack.obj", &litShader);
  CreateModel("X:/Repos/learnOpenGL/src/Resources/backpack/backpack.obj", &litShader);

  models[1].position = glm::vec3(20.0f, 4.0f, 23.0f);
  models[2].position = glm::vec3(23.0f, -5.0f, -20.0f);
  models[3].position = glm::vec3(-20.0f, 0.0f, 0.0f);
  models[3].scale = glm::vec3(0.5f, 0.5f, 0.5f);

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

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    input.processInput();
    cam.Update(deltaTime);

    DrawScene(mainCamera);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void DrawScene(Camera *cam)
{
  glClearColor(0.45f, 0.45f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (Model model : models)
  {
    model.Draw(cam, &sunRot);
  }
}

void CreateModel(char *path, Shader *newShader)
{
  Model newModel(path, newShader);
  models.push_back(newModel);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
  screenWidth = width;
  screenHeight = height;
  mainCamera->aspectRatio = (float)screenWidth / screenHeight;
}