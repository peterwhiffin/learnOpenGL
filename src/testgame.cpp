#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "camera.hpp"
#include "input.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void DrawScene();
Model* CreateModel(char* path, Shader* newShader);
void processInput(GLFWwindow* window, Camera* cam);
void mouse_callback(GLFWwindow* window);
unsigned int loadTexture(char const* path);

Camera* mainCamera;
std::map<Shader, std::vector<Model*>> shaderGroups;
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 sunRot = glm::vec3(0.0f, 0.0f, 48.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;

int screenWidth = 800;
int screenHeight = 600;
int frameCount = 0;

unsigned int rbo;
unsigned int texture;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Pete's graphics", NULL, NULL);

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
  glfwSwapInterval(0);

  stbi_set_flip_vertically_on_load(true);

  InputHandler input(window);
  mainCamera = new Camera(&input, (float)screenWidth / screenHeight, 0.1f, 10000.0f, glm::vec3(0.0f, 0.0f, 10.0f));

  Shader depthShader("X:/repos/learnOpenGL/src/shaders/depthshader.vs", "X:/repos/learnOpenGL/src/shaders/depthshader.fs");
  Shader singleColorShader("X:/repos/learnOpenGL/src/shaders/depthshader.vs", "X:/repos/learnOpenGL/src/shaders/singlecolorshader.fs");
  Shader screenShader("X:/Repos/learnOpenGL/src/shaders/screenshader.vs", "X:/Repos/learnOpenGL/src/shaders/screenshader.fs");

  screenShader.use();
  screenShader.setInt("screenTexture", 0);

  depthShader.use();
  depthShader.setInt("texture1", 0);

  unsigned int pavementTex = loadTexture("X:/Repos/learnOpenGL/src/Resources/pavement/pavement_02_diff_1k.jpg");
  unsigned int metalTex = loadTexture("X:/Repos/learnOpenGL/src/Resources/metal/metal_plate_02_diff_1k.jpg");

  Model* sponza = CreateModel("X:/Repos/learnOpenGL/src/Resources/sponza/sponza.obj", &depthShader);
  sponza->position = glm::vec3(0.0f, 0.0f, 0.0f);
  sponza->scale = glm::vec3(0.01f);

  float quadVertices[] = {// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                          // positions   // texCoords
                          -1.0f, 1.0f, 0.0f, 1.0f,
                          -1.0f, -1.0f, 0.0f, 0.0f,
                          1.0f, -1.0f, 1.0f, 0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,
                          1.0f, -1.0f, 1.0f, 0.0f,
                          1.0f, 1.0f, 1.0f, 1.0f};

  float cubeVertices[] = {
      // Back face
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // Bottom-left
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    // top-right
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,   // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    // top-right
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // bottom-left
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,   // top-left
      // Front face
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,    // top-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,    // top-right
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,   // top-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-left
      // Left face
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-right
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-left
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-left
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-right
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-right
                                        // Right face
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,     // top-left
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,   // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,    // top-right
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,   // bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,     // top-left
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f,    // bottom-left
      // Bottom face
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // top-right
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f,   // top-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,    // bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,    // bottom-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-right
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // top-right
      // Top face
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // bottom-right
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f    // bottom-left
  };
  float planeVertices[] = {
      5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,
      0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

      5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f,
      0.0f, 2.0f, 5.0f, -0.5f, -5.0f, 2.0f, 2.0f};

  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glBindVertexArray(0);

  unsigned int cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);

  unsigned int planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);

  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    input.processInput();
    //
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //
    // glm::mat4 model = glm::mat4(1.0f);
    // glm::mat4 view = mainCamera->GetViewMatrix();
    // glm::mat4 projection = mainCamera->GetProjection();
    //
    // singleColorShader.use();
    // singleColorShader.setMat4("view", view);
    // singleColorShader.setMat4("projection", projection);
    //
    // depthShader.use();
    // depthShader.setMat4("view", view);
    // depthShader.setMat4("projection", projection);
    //
    // glStencilMask(0x00);
    //
    // glBindVertexArray(planeVAO);
    // glBindTexture(GL_TEXTURE_2D, pavementTex);
    // model = glm::translate(model, glm::vec3(0.0f, 4.0f, 0.0f));
    // model = glm::scale(model, glm::vec3(16.0f));
    // depthShader.setMat4("model", model);
    // depthShader.setVec2("tiling", glm::vec2(12.0f, 12.0f));
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    //
    // glStencilFunc(GL_ALWAYS, 1, 0xFF);
    // glStencilMask(0xFF);
    //
    // depthShader.setVec2("tiling", glm::vec2(1.0f, 1.0f));
    //
    // glBindVertexArray(cubeVAO);
    // glBindTexture(GL_TEXTURE_2D, metalTex);
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -8.0f));
    // model = glm::scale(model, glm::vec3(8.0f));
    // depthShader.setMat4("model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(16.0f, 0.0f, 0.0f));
    // model = glm::scale(model, glm::vec3(8.0f));
    // depthShader.setMat4("model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    // glStencilMask(0x00);
    // glDisable(GL_DEPTH_TEST);
    //
    // singleColorShader.use();
    //
    // glBindVertexArray(cubeVAO);
    // glBindTexture(GL_TEXTURE_2D, metalTex);
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -8.0f));
    // model = glm::scale(model, glm::vec3(9.0f));
    // singleColorShader.setMat4("model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(16.0f, 0.0f, 0.0f));
    // model = glm::scale(model, glm::vec3(9.0f));
    // singleColorShader.setMat4("model", model);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // glStencilMask(0xFF);
    // glStencilFunc(GL_ALWAYS, 1, 0xFF);
    // glEnable(GL_DEPTH_TEST);
    //

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    DrawScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader.use();

    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    mainCamera->Update(deltaTime);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void DrawScene() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  for (const auto& pair : shaderGroups) {
    pair.first.use();
    pair.first.setMat4("projection", mainCamera->GetProjection());
    pair.first.setMat4("view", mainCamera->GetViewMatrix());
    pair.first.setVec3("viewPos", mainCamera->Position);

    for (Model* m : pair.second) {
      m->Draw(&pair.first);
    }
  }
}

Model* CreateModel(char* path, Shader* newShader) {
  Model* newModel = new Model(path);
  shaderGroups[*newShader].push_back(newModel);
  return newModel;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
  screenWidth = width;
  screenHeight = height;
  mainCamera->aspectRatio = (float)screenWidth / screenHeight;
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
}

unsigned int loadTexture(char const* path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrChannels;
  unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

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
    glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
  }

  stbi_image_free(data);
  return textureID;
}
