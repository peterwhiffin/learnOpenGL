#ifndef INPUT_H
#define INPUT_H
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glm/ext/vector_float2.hpp"

enum keyState
{
  STARTED,
  PERFORMED,
  CANCELED
};

class InputHandler
{
public:
  GLFWwindow *window;
  glm::vec2 moveInput;
  double mouseX, mouseY;
  float oldMouseX, oldMouseY;
  bool jump;
  bool debug;

  InputHandler(GLFWwindow *window);

  void processInput();
  void handleKeys();
  void handleMouse();
};
#endif
