#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <engine.hpp>
#include <gameobject.hpp>
#include <component/camera.hpp>
#include <component/renderable_object.hpp>

#include <editor.hpp>
#include <ui_render.hpp>

const uint32_t WIDTH = 1024;
const uint32_t HEIGHT = 768;

vke_editor::VKEditor *engine;

void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mousebutton_callback(GLFWwindow *window, int button, int action, int mods);

GLFWwindow *initWindow(int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    return glfwCreateWindow(width, height, "Vulkan window", nullptr, nullptr);
}

int main()
{
    GLFWwindow *window = initWindow(WIDTH, HEIGHT);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mousebutton_callback);
    vke_common::EventSystem::Init();
    vke_common::InputManager::Init(window);
    vke_common::TimeManager::Init();
    vke_render::RenderEnvironment *environment = vke_render::RenderEnvironment::Init(window);

    engine = vke_editor::VKEditor::Init();

    while (!glfwWindowShouldClose(environment->window))
    {
        glfwPollEvents();
        vke_common::TimeManager::Update();
        processInput(environment->window);

        engine->Update();
    }
    vkDeviceWaitIdle(environment->logicalDevice);

    vke_editor::VKEditor::Dispose();
    vke_render::RenderEnvironment::Dispose();
    vke_common::TimeManager::Dispose();
    vke_common::InputManager::Dispose();
    vke_common::EventSystem::Dispose();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (vke_common::InputManager::GetInstance())
        vke_common::InputManager::CursorPosCallback(xpos, ypos);
}

void mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (vke_common::InputManager::GetInstance())
        vke_common::InputManager::MouseButtonCallback(button, action, mods);
}