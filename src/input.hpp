#ifndef INPUT_H
#define INPUT_H
#include "glm/ext/vector_float2.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

  InputHandler(GLFWwindow *window);

  void processInput();
  void handleKeys();
  void handleMouse();
};
#endif
