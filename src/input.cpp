#include "input.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glm/ext/vector_float2.hpp"
#include "glm/geometric.hpp"

InputHandler::InputHandler(GLFWwindow* newWindow) : window(newWindow), moveInput(0.0f, 0.0f), mouseX(0), mouseY(0), oldMouseX(0), oldMouseY(0) {
}

void InputHandler::processInput() {
  handleMouse();
  handleKeys();
}

void InputHandler::handleKeys() {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  moveInput.x = 0.0f;
  moveInput.y = 0.0f;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    moveInput.y += 1.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    moveInput.y -= 1.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    moveInput.x += 1.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    moveInput.x -= 1.0f;
  }
}
void InputHandler::handleMouse() {
  double xpos = 0;
  double ypos = 0;

  glfwGetCursorPos(window, &xpos, &ypos);

  mouseX = xpos - oldMouseX;
  mouseY = oldMouseY - ypos;

  oldMouseX = xpos;
  oldMouseY = ypos;
}
