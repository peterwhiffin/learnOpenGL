#include <iostream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/rotate_vector.hpp>
#include <functional>

constexpr int ITERATIONS = 100000000;

// Method 1: Quaternion-Vector Multiplication
glm::vec3 GetForward_M1(const glm::quat &rotation)
{
    return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
}

// Method 2: Precompute 3x3 Rotation Matrix
glm::vec3 GetForward_M2(const glm::quat &rotation)
{
    glm::mat3 rotMatrix = glm::mat3_cast(rotation);
    return rotMatrix * glm::vec3(0.0f, 0.0f, -1.0f);
}

// Method 3: Direct Extraction from Quaternion Components
glm::vec3 GetForward_M3(const glm::quat &rotation)
{
    return glm::vec3(
        2.0f * (rotation.x * rotation.z + rotation.w * rotation.y),
        2.0f * (rotation.y * rotation.z - rotation.w * rotation.x),
        1.0f - 2.0f * (rotation.x * rotation.x + rotation.y * rotation.y));
}

// Method 4: User's Original Method
glm::vec3 GetForward_M4(const glm::quat &rotation)
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

    return glm::vec3(
        1.0f - (num5 + num6),
        num7 - num12,
        num8 + num11);
}

void Benchmark(const std::string &name, const std::function<glm::vec3(const glm::quat &)> &func)
{
    glm::quat rotation = glm::quat(glm::vec3(0.3f, 0.5f, 0.2f));
    glm::vec3 result;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i)
    {
        result = func(rotation);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << name << ": " << duration.count() << " seconds\n";
}

int main()
{
    std::cout << "Benchmarking quaternion forward vector computation with " << ITERATIONS << " iterations\n";
    // Benchmark("Quaternion * Vector", GetForward_M1);
    // Benchmark("Precomputed Matrix", GetForward_M2);
    Benchmark("Direct Extraction", GetForward_M3);
    Benchmark("User's Original Method", GetForward_M4);
    return 0;
}