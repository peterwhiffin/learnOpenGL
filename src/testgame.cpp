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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "camera.hpp"
#include "input.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void DrawScene(bool sunCam, Shader *altShader);
void DrawCubeScene(Shader *shader);
void DrawShadowScene(Shader *shader);
Model *CreateModel(char *path, Shader *newShader);
void processInput(GLFWwindow *window, Camera *cam);
void mouse_callback(GLFWwindow *window);
unsigned int loadTexture(char const *path);
unsigned int loadCubemap(std::vector<std::string> faces);

Camera *mainCamera;
Shader *skyboxShader;
InputHandler *playerInput;
std::map<Shader, std::vector<Model *>> shaderGroups;
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);
glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 sunPos = glm::vec3(-3.0f, 30.0f, -2.0f);
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

enum ControlMode
{
  CAM,
  UI
};

ControlMode controlMode = UI;

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // glfwWindowHint(GLFW_SAMPLES, 4);

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
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSwapInterval(0);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  stbi_set_flip_vertically_on_load(false);

  unsigned int defaultSpecTex;
  glGenTextures(1, &defaultSpecTex);
  unsigned char blackPixel[3] = {0, 0, 0};
  glBindTexture(GL_TEXTURE_2D, defaultSpecTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, blackPixel);

  InputHandler input(window);
  playerInput = &input;
  mainCamera = new Camera(&input, (float)screenWidth / screenHeight, 0.1f, 10000.0f, glm::vec3(0.0f, 0.0f, 10.0f));

  unsigned int pavementTex = loadTexture("../Resources/pavement/pavement_02_diff_1k.jpg");
  unsigned int metalTex = loadTexture("../Resources/metal/metal_plate_02_diff_1k.jpg");
  unsigned int blueNoise = loadTexture("../Resources/noise/bluenoise/64_64/HDR_L_0.png");

  Shader depthShader("../src/shaders/depthshader.vs", "../src/shaders/depthshader.fs");
  Shader singleColorShader("../src/shaders/depthshader.vs", "../src/shaders/singlecolorshader.fs");
  Shader screenShader("../src/shaders/screenshader.vs", "../src/shaders/screenshader.fs");
  Shader skyboxShaderObj("../src/shaders/skyboxshader.vs", "../src/shaders/skyboxshader.fs");
  Shader litShader("../src/shaders/defaultshader.vs", "../src/shaders/defaultshader.fs");
  Shader simpleDepthShader("../src/shaders/simpledepthshader.vs", "../src/shaders/simpledepthshader.fs");

  skyboxShader = &skyboxShaderObj;
  skyboxShaderObj.use();
  skyboxShaderObj.setInt("skybox", 0);
  skyboxShader->setVec3("color", glm::vec3(1, 1, 1));

  screenShader.use();
  screenShader.setInt("screenTexture", 0);

  depthShader.use();
  depthShader.setInt("texture1", 0);

  litShader.use();
  litShader.setInt("material.texture_diffuse0", 0);
  litShader.setInt("material.texture_specular0", 1);
  litShader.setInt("material.depthMap", 2);
  litShader.setVec3("dirLight.ambient", glm::vec3(0.01f));
  litShader.setVec3("dirLight.diffuse", glm::vec3(.98f));
  litShader.setVec3("dirLight.specular", glm::vec3(0.37f));
  litShader.setVec3("dirLight.direction", sunPos);
  litShader.setFloat("material.shininess", 32.0f);
  // uniform vec3 fogColor;
  // uniform float fogStart;
  // uniform float fogEnd;
  litShader.setVec3("fogColor", glm::vec3(1.0f));
  litShader.setFloat("fogStart", 1.0f);
  litShader.setFloat("fogEnd", 50.0f);
  litShader.setInt("material.blueNoise", 3);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, blueNoise);
  glActiveTexture(GL_TEXTURE0);

  Model *sponza = CreateModel("../Resources/sponza/sponza.obj", &litShader);
  //  Model *japanStreet = CreateModel("../Resources/JapanStreet/gltf/scene.gltf", &litShader);
  // Model *sponza = CreateModel("../Resources/main1_sponza/NewSponza_Main_Zup_003.fbx", &litShader);
  //    Model *curtains = CreateModel("../Resources/main1_sponza/NewSponza_Curtains_FBX_ZUp.fbx", &litShader);

  sponza->position = glm::vec3(0.0f, 0.0f, 0.0f);
  sponza->scale = glm::vec3(0.01f);
  // japanStreet->position = glm::vec3(0.0f, 0.0f, 0.0f);
  // japanStreet->scale = glm::vec3(1.0f);
  //  curtains->position = glm::vec3(0.0f, 0.0f, 0.0f);
  //  curtains->scale = glm::vec3(0.01f);

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

  uniformBlockIndexDepth = glGetUniformBlockIndex(depthShader.ID, "Matrices");
  uniformBlockIndexLit = glGetUniformBlockIndex(litShader.ID, "Matrices");

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

  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &fullscreenTexture);
  glBindTexture(GL_TEXTURE_2D, fullscreenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fullscreenTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  }

  std::vector<std::string> faces{
      "../Resources/skybox/px.png",
      "../Resources/skybox/nx.png",
      "../Resources/skybox/py.png",
      "../Resources/skybox/ny.png",
      "../Resources/skybox/pz.png",
      "../Resources/skybox/nz.png",
  };

  const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
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
  float borderColor[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  cubemapTexture = loadCubemap(faces);

  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LESS);
  // glEnable(GL_STENCIL_TEST);
  // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  //  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  // glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  // glEnable(GL_MULTISAMPLE);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, depthMap);

  // slope scale offset bias
  glEnable(GL_POLYGON_OFFSET_FILL);
  float factor = 1.0f;
  float units = 1.0f;
  glPolygonOffset(factor, units);

  bool showDemoWindow = true;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    input.processInput();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(&showDemoWindow);

    glEnable(GL_DEPTH_TEST);
    float near_plane = 1.0f, far_plane = 85.0f;

    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    lightView = glm::lookAt(sunPos + mainCamera->Position, mainCamera->Position, glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    simpleDepthShader.use();
    simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    DrawShadowScene(&simpleDepthShader);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_BACK);
    litShader.use();

    glViewport(0, 0, screenWidth, screenHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    litShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    litShader.setMat4("u_invLightViewProj", glm::inverse(lightSpaceMatrix));

    DrawScene(input.jump, &simpleDepthShader);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
    // glBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    screenShader.use();
    screenShader.setVec2("screenResolution", glm::vec2(screenWidth, screenHeight));
    if (input.debug)
    {
      screenShader.SetBool("useAlpha", false);
    }
    else
    {
      screenShader.SetBool("useAlpha", true);
    }
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, fullscreenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (!input.jump)
    {
      if (input.rightClick)
      {
        if (controlMode != CAM)
        {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          controlMode = CAM;
        }

        mainCamera->Update(deltaTime);
      }
      else
      {
        if (controlMode != UI)
        {
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
          controlMode = UI;
        }
      }
    }

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void DrawScene(bool sunCam, Shader *altShader)
{

  projection = mainCamera->GetProjection();
  viewPos = mainCamera->Position;
  glm::mat4 invViewProj = glm::inverse(projection * view);

  glDepthFunc(GL_LEQUAL);
  skyboxShader->use();

  view = glm::mat4(glm::mat3(view));

  skyboxShader->setMat4("view", view);
  skyboxShader->setMat4("projection", projection);

  glBindVertexArray(skyboxVAO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthFunc(GL_LESS);

  view = mainCamera->GetViewMatrix();
  if (sunCam)
  {
    projection = lightProjection;
    view = lightView;

    sunPos.x += playerInput->moveInput.x * deltaTime * 10.1f;
    sunPos.z += playerInput->moveInput.y * deltaTime * 10.1f;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // glActiveTexture(GL_TEXTURE3);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  // glActiveTexture(GL_TEXTURE0);

  for (const auto &pair : shaderGroups)
  {
    pair.first.use();
    pair.first.setVec3("dirLight.direction", sunPos);
    pair.first.setVec3("viewPos", viewPos);
    pair.first.setMat4("u_invViewProj", invViewProj);

    for (Model *m : pair.second)
    {
      m->Draw(&pair.first);
    }
  }
}

void DrawShadowScene(Shader *shader)
{
  for (const auto &pair : shaderGroups)
  {
    shader->use();

    for (Model *m : pair.second)
    {
      m->Draw(shader);
    }
  }
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

  stbi_set_flip_vertically_on_load(false);

  return textureID;
}
