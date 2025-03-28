#include "camera.hpp"
#include "glm/ext/matrix_float4x4.hpp"
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
#include <map>
#include <string>
#include <vector>
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void DrawScene();
void CreateModel(char *path, Shader *newShader);
void processInput(GLFWwindow *window, Camera *cam);
unsigned int loadTexture(char const *path);
void mouse_callback(GLFWwindow *window);
bool filled = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int screenWidth = 800;
int screenHeight = 600;
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 sunRot = glm::vec3(0.0f, 0.0f, 48.0f);
std::map<Shader, std::vector<Model>> shaderGroups;
Camera *mainCamera;
float frameTimeSum = 0.0f;
int frameCount = 0;
float lastX = 0.0f;
float lastY = 0.0f;
unsigned int defaultTex = 0;
float elapsedTime = 0.0f;
int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight,
                                        "Pete's graphics", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glViewport(0, 0, screenWidth, screenHeight);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSwapInterval(1);

  InputHandler input(window);

  Camera cam(&input, (float)screenWidth / screenHeight, 0.1f, 1000.0f,
             glm::vec3(0.0f, 0.0f, 10.0f));
  mainCamera = &cam;
  stbi_set_flip_vertically_on_load(true);
  // Shader litShader("X:/repos/learnOpenGL/src/shaders/defaultshader.vs",
  //                 "X:/repos/learnOpenGL/src/shaders/defaultshader.fs");
  // Shader debugShader("X:/repos/learnOpenGL/src/shaders/debugshader.vs",
  //                  "X:/repos/learnOpenGL/src/shaders/debugshader.fs");
  Shader depthShader("X:/repos/learnOpenGL/src/shaders/depthshader.vs",
                     "X:/repos/learnOpenGL/src/shaders/depthshader.fs");
  Shader singleColorShader(
      "X:/repos/learnOpenGL/src/shaders/depthshader.vs",
      "X:/repos/learnOpenGL/src/shaders/singlecolorshader.fs");
  // CreateModel("X:/Repos/learnOpenGL/src/Resources/M4/ddm4 v7.obj",
  //            &depthShader);
  // CreateModel("X:/Repos/learnOpenGL/src/Resources/backpack/backpack.obj",
  //           &depthShader);

  // shaderGroups[depthShader][1].position = glm::vec3(20.0f, 4.0f, 23.0f);

  // glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
  // glm::vec3 diffuseColor = lightColor * glm::vec3(2.0f);
  // glm::vec3 ambientColor = diffuseColor * glm::vec3(0.7f);
  // glm::vec3 specularColor = diffuseColor * glm::vec3(10.0f, 10.0f, 10.0f);

  // litShader.use();
  // litShader.setFloat("material.shininess", 32.0f);
  // litShader.setInt("material.diffuse", 0);
  // litShader.setInt("material.specular", 1);
  //
  // litShader.setVec3("dirLight.ambient", ambientColor);
  // litShader.setVec3("dirLight.diffuse", diffuseColor);
  // litShader.setVec3("dirLight.specular", specularColor);
  // litShader.setVec3("dirLight.direction", sunRot);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  // glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LESS);
  // glEnable(GL_BLEND);
  // glEnable(GL_STENCIL_TEST);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  unsigned int pavementTex =
      loadTexture("X:/Repos/learnOpenGL/src/Resources/pavement/"
                  "pavement_02_diff_1k.jpg");
  unsigned int metalTex =
      loadTexture("X:/Repos/learnOpenGL/src/Resources/metal/"
                  "metal_plate_02_diff_1k.jpg");
  float cubeVertices[] = {
      // positions          // texture Coords
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};
  float planeVertices[] = {
      // positions          // texture Coords (note we set these
      // higher than 1
      // (together with GL_REPEAT as texture wrapping mode). this
      // will cause the
      // floor texture to repeat)
      5.0f, -0.5f, 5.0f,  2.0f,  0.0f,  -5.0f, -0.5f, 5.0f,
      0.0f, 0.0f,  -5.0f, -0.5f, -5.0f, 0.0f,  2.0f,

      5.0f, -0.5f, 5.0f,  2.0f,  0.0f,  -5.0f, -0.5f, -5.0f,
      0.0f, 2.0f,  5.0f,  -0.5f, -5.0f, 2.0f,  2.0f};

  unsigned int cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glBindVertexArray(0);
  // plane VAO
  unsigned int planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glBindVertexArray(0);

  depthShader.use();
  depthShader.setInt("texture1", 0);
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    input.processInput();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //    DrawScene();
    singleColorShader.use();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = cam.GetViewMatrix();
    glm::mat4 projection = cam.GetProjection();
    singleColorShader.setMat4("view", view);
    singleColorShader.setMat4("projection", projection);
    depthShader.use();

    glStencilMask(0x00);
    glBindVertexArray(planeVAO);
    glBindTexture(GL_TEXTURE_2D, pavementTex);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.0f));
    model = glm::scale(model, glm::vec3(16.0f));
    depthShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    //    depthShader.setInt("texture1", 0);
    view = cam.GetViewMatrix();
    projection = cam.GetProjection();
    depthShader.setMat4("view", view);
    depthShader.setMat4("projection", projection);
    // cubes
    glBindVertexArray(cubeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -8.0f));
    model = glm::scale(model, glm::vec3(8.0f));
    depthShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(16.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(8.0f));
    depthShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    singleColorShader.use();
    glBindVertexArray(cubeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, metalTex);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -8.0f));
    model = glm::scale(model, glm::vec3(9.0f));
    depthShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(16.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(9.0f));
    depthShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glEnable(GL_DEPTH_TEST);
    // floor
    cam.Update(deltaTime);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void DrawScene() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (const auto &pair : shaderGroups) {
    pair.first.use();
    pair.first.setMat4("projection", mainCamera->GetProjection());
    pair.first.setMat4("view", mainCamera->GetViewMatrix());
    pair.first.setVec3("viewPos", mainCamera->Position);
    for (Model m : pair.second) {
      m.Draw(&pair.first);
    }
  }
}

void CreateModel(char *path, Shader *newShader) {
  Model newModel(path);
  shaderGroups[*newShader].push_back(newModel);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  screenWidth = width;
  screenHeight = height;
  mainCamera->aspectRatio = (float)screenWidth / screenHeight;
}

unsigned int loadTexture(char const *path) {

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

  if (data) {
    GLenum pixelFormat;

    if (nrChannels == 1) {

      pixelFormat = GL_RED;
    } else if (nrChannels == 3) {
      pixelFormat = GL_RGB;
    } else if (nrChannels == 4) {
      pixelFormat = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, width, height, 0, pixelFormat,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
  }

  stbi_image_free(data);
  return textureID;
}
