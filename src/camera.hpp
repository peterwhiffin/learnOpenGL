#ifndef CAMERA_H
#define CAMERA_H
#include <glad/glad.h>

#include "input.hpp"
#include <glm/ext/quaternion_float.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum Camera_Movement
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 15.5f;
const float SENSITIVITY = 0.1f;

class Camera
{
public:
  glm::vec3 Position;
  glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float fov;
  float aspectRatio;

  InputHandler *input;
  // constructor with vectors
  Camera(InputHandler *inputHandler, float aspectRatio, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
  {
    Position = position;
    Yaw = yaw;
    Pitch = pitch;
    input = inputHandler;
    this->aspectRatio = aspectRatio;
    fov = 67.0f;
  }

  glm::mat4 GetViewMatrix()
  {
    return glm::lookAt(Position, Position + forward(), up());
  }

  glm::mat4 GetProjection()
  {
    return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
  }

  glm::vec3 right()
  {
    return QuaternionByVector3(rotation, glm::vec3(1.0f, 0.0f, 0.0f));
  }

  glm::vec3 up()
  {
    return QuaternionByVector3(rotation, glm::vec3(0.0f, 1.0f, 0.0f));
  }

  glm::vec3 forward()
  {
    return QuaternionByVector3(rotation, glm::vec3(0.0f, 0.0f, 1.0f));
  }

  glm::vec3 QuaternionByVector3(glm::quat rotation, glm::vec3 point)
  {
    float num = rotation.x + rotation.x;
    float num2 = rotation.y + rotation.y;
    float num3 = rotation.z + rotation.z;

    float num4 = rotation.x * num;
    float num5 = rotation.y * num2;
    float num6 = rotation.z * num3;

    float num7 = rotation.x * num2;
    float num8 = rotation.x * num3;
    float num9 = rotation.y * num3;

    float num10 = rotation.w * num;
    float num11 = rotation.w * num2;
    float num12 = rotation.w * num3;

    glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);

    result.x = (1.0f - (num5 + num6)) * point.x + (num7 - num12) * point.y + (num8 + num11) * point.z;
    result.y = (num7 + num12) * point.x + (1.0f - (num4 + num6)) * point.y + (num9 - num10) * point.z;
    result.z = (num8 - num11) * point.x + (num9 + num10) * point.y + (1.0f - (num4 + num5)) * point.z;

    return result;
  }

  void Update(float deltaTime)
  {
    ProcessMouseMovement(input->mouseX, input->mouseY);
    glm::vec3 moveDir = input->moveInput.y * forward() + input->moveInput.x * right();
    Position += moveDir * MovementSpeed * deltaTime;
  }

  void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
  {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw -= xoffset;
    Pitch -= yoffset;

    if (constrainPitch)
    {
      if (Pitch > 89.0f)
        Pitch = 89.0f;
      if (Pitch < -89.0f)
        Pitch = -89.0f;
    }
    rotation = glm::quat(glm::vec3(glm::radians(Pitch), glm::radians(Yaw), 0.0f));
  }

  void ProcessMouseScroll(float yoffset)
  {
    fov -= (float)yoffset;
    if (fov < 1.0f)
      fov = 1.0f;
    if (fov > 45.0f)
      fov = 45.0f;
  }
};
#endif
