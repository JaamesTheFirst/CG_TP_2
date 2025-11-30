

#include "CG_TP_2.h"

#include "Model.hpp"
#include "ShaderProgram.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace {

struct CameraController {
    float distance = 160.0f;
    float yaw = glm::radians(45.0f);
    float pitch = glm::radians(12.0f);
    bool dragging = false;
    double lastX = 0.0;
    double lastY = 0.0;
};

void ErrorCallback(int code, const char* description) {
    std::cerr << "[GLFW] Error " << code << ": " << description << std::endl;
}

void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

bool InitGLFW() {
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, 4);
    return true;
}

bool InitGLEW() {
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    // GLEW can emit a benign error on init; clear it.
    glGetError();
    if (glewError != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: "
                  << reinterpret_cast<const char*>(glewGetErrorString(glewError)) << std::endl;
        return false;
    }
    return true;
}

CameraController* GetCamera(GLFWwindow* window) {
    return static_cast<CameraController*>(glfwGetWindowUserPointer(window));
}

void ScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    if (auto* camera = GetCamera(window)) {
        camera->distance -= static_cast<float>(yoffset) * 8.0f;
        camera->distance = std::clamp(camera->distance, 20.0f, 400.0f);
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
    }
    auto* camera = GetCamera(window);
    if (!camera) {
        return;
    }

    if (action == GLFW_PRESS) {
        camera->dragging = true;
        glfwGetCursorPos(window, &camera->lastX, &camera->lastY);
    } else if (action == GLFW_RELEASE) {
        camera->dragging = false;
    }
}

void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* camera = GetCamera(window);
    if (!camera || !camera->dragging) {
        return;
    }

    double dx = xpos - camera->lastX;
    double dy = ypos - camera->lastY;
    camera->lastX = xpos;
    camera->lastY = ypos;

    camera->yaw += static_cast<float>(dx) * 0.005f;
    camera->pitch += static_cast<float>(dy) * 0.005f;
    camera->pitch = std::clamp(camera->pitch, -1.2f, 1.2f);
}

void UpdateCameraFromKeyboard(GLFWwindow* window, CameraController& camera, float deltaTime) {
    const float orbitSpeed = 1.5f;
    const float zoomSpeed = 120.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.yaw -= orbitSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.yaw += orbitSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.pitch -= orbitSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.pitch += orbitSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera.distance += zoomSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera.distance -= zoomSpeed * deltaTime;
    }

    camera.pitch = std::clamp(camera.pitch, -1.2f, 1.2f);
    camera.distance = std::clamp(camera.distance, 20.0f, 400.0f);
}

} // namespace

int main() {
    if (!InitGLFW()) {
        return EXIT_FAILURE;
    }

    const int initialWidth = 1280;
    const int initialHeight = 720;
    GLFWwindow* window = glfwCreateWindow(initialWidth, initialHeight, "UFO Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
    glfwSwapInterval(1);

    if (!InitGLEW()) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    CameraController camera;
    glfwSetWindowUserPointer(window, &camera);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);

    std::cout << "Controls: drag with LMB to orbit, scroll/Q/E to zoom, WASD/arrow keys to adjust view.\n";

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    const std::filesystem::path shaderRoot = std::filesystem::path(PROJECT_SOURCE_DIR) / "assets" / "shaders";
    ShaderProgram shaderProgram;
    std::string shaderError;
    if (!shaderProgram.LoadFromFiles(shaderRoot / "object.vert", shaderRoot / "object.frag", &shaderError)) {
        std::cerr << shaderError << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    const std::filesystem::path ufoPath = std::filesystem::path(PROJECT_SOURCE_DIR) / "UFO" / "Low_poly_UFO.obj";
    Model ufoModel;
    std::string modelError;
    if (!ufoModel.LoadFromObj(ufoPath, &modelError)) {
        std::cerr << modelError << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glm::vec3 lightDir = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f));
    glm::vec3 lightColor(1.0f, 0.96f, 0.86f);
    glm::vec3 ambientColor(0.08f, 0.08f, 0.14f);

    float previousTime = static_cast<float>(glfwGetTime());

    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        UpdateCameraFromKeyboard(window, camera, deltaTime);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = width > 0 && height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;

        glViewport(0, 0, width, height);
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 target(0.0f, 15.0f, 0.0f);
        glm::vec3 cameraOffset;
        cameraOffset.x = camera.distance * std::cos(camera.pitch) * std::sin(camera.yaw);
        cameraOffset.y = camera.distance * std::sin(camera.pitch);
        cameraOffset.z = camera.distance * std::cos(camera.pitch) * std::cos(camera.yaw);
        glm::vec3 cameraPos = target + cameraOffset;

        glm::mat4 view = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 500.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, currentTime * 0.15f, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.4f));
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

        shaderProgram.Use();
        shaderProgram.SetMat4("uModel", model);
        shaderProgram.SetMat4("uView", view);
        shaderProgram.SetMat4("uProjection", projection);
        shaderProgram.SetMat3("uNormalMatrix", normalMatrix);
        shaderProgram.SetVec3("uLightDir", lightDir);
        shaderProgram.SetVec3("uLightColor", lightColor);
        shaderProgram.SetVec3("uAmbientColor", ambientColor);
        shaderProgram.SetVec3("uCameraPos", cameraPos);
        shaderProgram.SetInt("uDiffuseMap", 0);

        ufoModel.Draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ufoModel.Destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
