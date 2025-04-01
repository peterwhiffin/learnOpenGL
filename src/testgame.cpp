#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <complex>
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

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void DrawScene(bool sunCam);
void DrawCubeScene(Shader *shader);
void DrawShadowScene(Shader *shader);
Model *CreateModel(char *path, Shader *newShader);
void processInput(GLFWwindow *window, Camera *cam);
void mouse_callback(GLFWwindow *window);
unsigned int loadTexture(char const *path);
unsigned int loadCubemap(std::vector<std::string> faces);

Camera *mainCamera;
Shader *skyboxShader;
std::map<Shader, std::vector<Model *>> shaderGroups;
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 sunRot = glm::vec3(-60.0f, 160.0f, -1.0f);
glm::mat4 lightProjection, lightView;
glm::mat4 lightSpaceMatrix;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;

int screenWidth = 800;
int screenHeight = 600;
int frameCount = 0;

unsigned int skyboxVAO;
unsigned int skyboxVBO;
unsigned int rbo;
unsigned int fullscreenTexture;
unsigned int cubemapTexture;
unsigned int uniformBlockIndexDepth;
unsigned int uniformBlockIndexLit;
unsigned int uboMatrices;
unsigned int cubeVAO, cubeVBO;
unsigned int planeVAO, planeVBO;

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_SAMPLES, 16);

  GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Pete's graphics", NULL, NULL);

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
  Shader skyboxShaderObj("X:/Repos/learnOpenGL/src/shaders/skyboxshader.vs", "X:/Repos/learnOpenGL/src/shaders/skyboxshader.fs");
  Shader litShader("X:/Repos/learnOpenGL/src/shaders/defaultshader.vs", "X:/Repos/learnOpenGL/src/shaders/defaultshader.fs");
  Shader simpleDepthShader("X:/Repos/learnOpenGL/src/shaders/simpledepthshader.vs", "X:/Repos/learnOpenGL/src/shaders/simpledepthshader.fs");

  skyboxShader = &skyboxShaderObj;
  skyboxShaderObj.use();
  skyboxShaderObj.setInt("skybox", 0);

  screenShader.use();
  screenShader.setInt("screenTexture", 0);

  depthShader.use();
  depthShader.setInt("texture1", 0);

  litShader.use();
  litShader.setInt("material.depthMap", 2);
  litShader.setInt("material.diffuse", 0);
  litShader.setInt("material.specular", 1);
  litShader.setVec3("dirLight.ambient", glm::vec3(0.14f));
  litShader.setVec3("dirLight.diffuse", glm::vec3(0.87f));
  litShader.setVec3("dirLight.specular", glm::vec3(0.77f));
  litShader.setVec3("dirLight.direction", sunRot);
  litShader.setFloat("material.shininess", 32.0f);
  unsigned int pavementTex = loadTexture("X:/Repos/learnOpenGL/src/Resources/pavement/pavement_02_diff_1k.jpg");
  unsigned int metalTex = loadTexture("X:/Repos/learnOpenGL/src/Resources/metal/metal_plate_02_diff_1k.jpg");

  Model *sponza = CreateModel("X:/Repos/learnOpenGL/src/Resources/sponza/sponza.obj", &litShader);
  sponza->position = glm::vec3(0.0f, 0.0f, 0.0f);
  sponza->scale = glm::vec3(0.01f);

  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 1.0f,

      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, 1.0f};

  float quadVertices[] = {// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                          // positions   // texCoords
                          -1.0f, 1.0f, 0.0f, 1.0f,
                          -1.0f, -1.0f, 0.0f, 0.0f,
                          1.0f, -1.0f, 1.0f, 0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,
                          1.0f, -1.0f, 1.0f, 0.0f,
                          1.0f, 1.0f, 1.0f, 1.0f};

  // float cubeVertices[] = {
  //     // Back face
  //     -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
  //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
  //     0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
  //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
  //     -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
  //     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
  //     // Front face
  //     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
  //     0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // bottom-right
  //     0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
  //     0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
  //     -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // top-left
  //     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
  //     // Left face
  //     -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
  //     -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-left
  //     -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
  //     -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
  //     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
  //     -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
  //                                      // Right face
  //     0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
  //     0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
  //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
  //     0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
  //     0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
  //     0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-left
  //     // Bottom face
  //     -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
  //     0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  // top-left
  //     0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
  //     0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
  //     -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
  //     -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
  //     // Top face
  //     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
  //     0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
  //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-right
  //     0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
  //     -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
  //     -0.5f, 0.5f, 0.5f, 0.0f, 0.0f   // bottom-left
  // };

  float cubeVertices[] = {
      // back face
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
      1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
      1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
      -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
      -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
      // front face
      -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
      1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
      1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
      1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
      -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
      -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
      // left face
      -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
      -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
      -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
      -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
      -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
      -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                          // right face
      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
      1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
      1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
      1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
      1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
      // bottom face
      -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
      1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
      1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
      1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
      -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
      -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
      // top face
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
      1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
      1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
      1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
      -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
      -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
  };
  // float planeVertices[] = {
  //     5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,
  //     0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

  //     5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f,
  //     0.0f, 2.0f, 5.0f, -0.5f, -5.0f, 2.0f, 2.0f};

  float planeVertices[] = {
      // positions            // normals         // texcoords
      25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
      -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,

      25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
      -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
      25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f};

  uniformBlockIndexDepth = glad_glGetUniformBlockIndex(depthShader.ID, "Matrices");
  uniformBlockIndexLit = glad_glGetUniformBlockIndex(litShader.ID, "Matrices");

  glUniformBlockBinding(depthShader.ID, uniformBlockIndexDepth, 0);
  glUniformBlockBinding(litShader.ID, uniformBlockIndexLit, 0);

  glGenBuffers(1, &uboMatrices);
  glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
  glBindVertexArray(0);

  // glGenVertexArrays(1, &cubeVAO);
  // glGenBuffers(1, &cubeVBO);
  // glBindVertexArray(cubeVAO);
  // glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
  // glEnableVertexAttribArray(0);
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  // glEnableVertexAttribArray(2);
  // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  // glBindVertexArray(0);

  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  // fill buffer
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
  // link vertex attributes
  glBindVertexArray(cubeVAO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // glGenVertexArrays(1, &planeVAO);
  // glGenBuffers(1, &planeVBO);
  // glBindVertexArray(planeVAO);
  // glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
  // glEnableVertexAttribArray(0);
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  // glEnableVertexAttribArray(2);
  // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  // glBindVertexArray(0);

  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
  glBindVertexArray(0);

  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &fullscreenTexture);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fullscreenTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, 800, 600, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fullscreenTexture, 0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 800, 600);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  unsigned int intermediateFBO;
  glGenFramebuffers(1, &intermediateFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
  // create a color attachment texture
  unsigned int screenTexture;
  glGenTextures(1, &screenTexture);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0); // we only need a color buffer

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  std::vector<std::string> faces{
      "X:/Repos/learnOpenGL/src/Resources/skybox/px.png",
      "X:/Repos/learnOpenGL/src/Resources/skybox/nx.png",
      "X:/Repos/learnOpenGL/src/Resources/skybox/py.png",
      "X:/Repos/learnOpenGL/src/Resources/skybox/ny.png",
      "X:/Repos/learnOpenGL/src/Resources/skybox/pz.png",
      "X:/Repos/learnOpenGL/src/Resources/skybox/nz.png",
  };

  const unsigned int SHADOW_WIDTH = 8129 * 2, SHADOW_HEIGHT = 8129 * 2;
  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);
  // create depth texture
  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  cubemapTexture = loadCubemap(faces);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LESS);
  // glEnable(GL_STENCIL_TEST);
  // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  //  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  glEnable(GL_MULTISAMPLE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    input.processInput();

    float near_plane = 0.1f, far_plane = 180.5f;
    // lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    lightView = glm::lookAt(sunRot, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
    // render scene from light's point of view
    simpleDepthShader.use();
    simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, woodTexture);
    // renderScene(simpleDepthShader);

    // simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    //  DrawCubeScene(&simpleDepthShader);
    DrawShadowScene(&simpleDepthShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport
    glViewport(0, 0, screenWidth, screenHeight);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_DEPTH_TEST);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, metalTex);

    // DrawScene();
    litShader.use();
    litShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    // DrawCubeScene(&litShader);

    DrawScene(input.jump);

    // glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    // glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST);
    // glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);

    // screenShader.use();

    // glBindVertexArray(quadVAO);
    // glActiveTexture(GL_TEXTURE0);

    // glBindTexture(GL_TEXTURE_2D, screenTexture);
    // glDrawArrays(GL_TRIANGLES, 0, 6);

    mainCamera->Update(deltaTime);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void DrawCubeScene(Shader *shader)
{
  shader->use();
  view = mainCamera->GetViewMatrix();
  projection = mainCamera->GetProjection();
  viewPos = mainCamera->Position;

  glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glm::mat4 model = glm::mat4(1.0f);
  shader->setVec3("dirLight.direction", sunRot);
  shader->setMat4("model", model);
  shader->setVec3("viewPos", viewPos);
  shader->setMat4("normalMat", glm::transpose(glm::inverse(glm::mat3(model))));
  glBindVertexArray(planeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(cubeVAO);
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
  model = glm::scale(model, glm::vec3(0.5f));
  shader->setMat4("model", model);
  shader->setMat4("normalMat", glm::transpose(glm::inverse(glm::mat3(model))));
  glDrawArrays(GL_TRIANGLES, 0, 36);
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
  model = glm::scale(model, glm::vec3(0.5f));
  shader->setMat4("model", model);
  shader->setMat4("normalMat", glm::transpose(glm::inverse(glm::mat3(model))));
  glDrawArrays(GL_TRIANGLES, 0, 36);
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.25));
  shader->setMat4("model", model);
  shader->setMat4("normalMat", glm::transpose(glm::inverse(glm::mat3(model))));
  glDrawArrays(GL_TRIANGLES, 0, 36);

  //////sky/////////////
  // glDepthFunc(GL_LEQUAL);
  // skyboxShader->use();

  // view = glm::mat4(glm::mat3(view));

  // skyboxShader->setMat4("view", view);
  // skyboxShader->setMat4("projection", projection);

  // glBindVertexArray(skyboxVAO);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  // glDrawArrays(GL_TRIANGLES, 0, 36);
  // glDepthFunc(GL_LESS);
}

void DrawScene(bool sunCam)
{
  view = mainCamera->GetViewMatrix();
  projection = mainCamera->GetProjection();
  viewPos = mainCamera->Position;

  if (sunCam)
  {
    projection = lightProjection;
    view = lightView;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  for (const auto &pair : shaderGroups)
  {
    pair.first.use();
    pair.first.setVec3("dirLight.direction", sunRot);
    // pair.first.setMat4("projection", projection);
    // pair.first.setMat4("view", view);
    pair.first.setVec3("viewPos", viewPos);

    for (Model *m : pair.second)
    {
      m->Draw(&pair.first);
    }
  }

  glDepthFunc(GL_LEQUAL);
  skyboxShader->use();

  view = glm::mat4(glm::mat3(view));

  skyboxShader->setMat4("view", view);
  skyboxShader->setMat4("projection", projection);

  glBindVertexArray(skyboxVAO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthFunc(GL_LESS);
}

void DrawShadowScene(Shader *shader)
{
  // view = mainCamera->GetViewMatrix();
  // projection = mainCamera->GetProjection();
  // viewPos = mainCamera->Position;

  // glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  // glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
  // glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
  // glBindBuffer(GL_UNIFORM_BUFFER, 0);

  for (const auto &pair : shaderGroups)
  {
    shader->use();
    // shader->setVec3("dirLight.direction", sunRot);
    //  pair.first.setMat4("projection", projection);
    //  pair.first.setMat4("view", view);
    // shader->setVec3("viewPos", viewPos);

    for (Model *m : pair.second)
    {
      m->Draw(shader);
    }
  }

  // glDepthFunc(GL_LEQUAL);
  // skyboxShader->use();

  // view = glm::mat4(glm::mat3(view));

  // skyboxShader->setMat4("view", view);
  // skyboxShader->setMat4("projection", projection);

  // glBindVertexArray(skyboxVAO);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  // glDrawArrays(GL_TRIANGLES, 0, 36);
  // glDepthFunc(GL_LESS);
}

Model *CreateModel(char *path, Shader *newShader)
{
  Model *newModel = new Model(path);
  shaderGroups[*newShader].push_back(newModel);
  return newModel;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
  screenWidth = width;
  screenHeight = height;
  mainCamera->aspectRatio = (float)screenWidth / screenHeight;
  glBindTexture(GL_TEXTURE_2D, fullscreenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
}

unsigned int loadTexture(char const *path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

  if (data)
  {
    GLenum pixelFormat;

    if (nrChannels == 1)
    {
      pixelFormat = GL_RED;
    }
    else if (nrChannels == 3)
    {
      pixelFormat = GL_RGB;
    }
    else if (nrChannels == 4)
    {
      pixelFormat = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
  }

  stbi_image_free(data);
  return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
  stbi_set_flip_vertically_on_load(false);
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      GLenum pixelFormat;

      if (nrChannels == 1)
      {
        pixelFormat = GL_RED;
      }
      else if (nrChannels == 3)
      {
        pixelFormat = GL_RGB;
      }
      else if (nrChannels == 4)
      {
        pixelFormat = GL_RGBA;
      }

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, pixelFormat, width, height, 0, pixelFormat, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }
    else
    {
      std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  stbi_set_flip_vertically_on_load(true);

  return textureID;
}
