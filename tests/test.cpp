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
float time_prev, time_delta;
vke_common::TransformParameter camParam(glm::vec3(-5.0f, 4.0f, 10.0f), glm::vec3(1), glm::quat(1.0, 0.0, 0.0, 0.0));
vke_common::GameObject *camp = nullptr;
vke_common::GameObject *objp = nullptr, *obj2p = nullptr;

void processInput(GLFWwindow *window, vke_common::GameObject *target, vke_common::GameObject *obj);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

int main()
{
    engine = vke_editor::VKEditor::Init(WIDTH, HEIGHT);
    vke_render::RenderEnvironment *environment = vke_render::RenderEnvironment::GetInstance();

    std::unique_ptr<vke_common::GameObject> cameraGameObj = std::make_unique<vke_common::GameObject>(camParam);
    camp = cameraGameObj.get();
    cameraGameObj->AddComponent(std::make_unique<vke_component::Camera>(105, WIDTH, HEIGHT, 0.01, 1000, camp));

    vke_common::ResourceManager *manager = vke_common::ResourceManager::GetInstance();

    std::unique_ptr<vke_common::Scene> scene = std::make_unique<vke_common::Scene>();
    scene->AddObject(std::move(cameraGameObj));

    vke_common::SceneManager::SetCurrentScene(std::move(scene));

    // glfwSetInputMode(engine->environment->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(environment->window, mouse_callback);
    glfwSetFramebufferSizeCallback(environment->window, vke_common::Engine::OnWindowResize);

    while (!glfwWindowShouldClose(environment->window))
    {
        glfwPollEvents();

        float now = glfwGetTime();
        if (time_prev == 0)
            time_prev = glfwGetTime();
        time_delta = now - time_prev;
        time_prev = now;
        // std::cout << 1 / time_delta << std::endl;

        processInput(environment->window, camp, objp);

        engine->Update();
    }
    vkDeviceWaitIdle(environment->logicalDevice);

    vke_common::Engine::Dispose();
    return 0;
}

#define CHECK_KEY(x) if (glfwGetKey(window, x) == GLFW_PRESS)

float moveSpeed = 2.5f;
float rotateSpeed = 1.0f;

void processInput(GLFWwindow *window, vke_common::GameObject *target, vke_common::GameObject *obj)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    CHECK_KEY(GLFW_KEY_W)
    {
        target->TranslateLocal(glm::vec3(0, 0, -moveSpeed * time_delta));
    }
    CHECK_KEY(GLFW_KEY_A)
    {
        target->TranslateLocal(glm::vec3(-moveSpeed * time_delta, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_S)
    {
        target->TranslateLocal(glm::vec3(0, 0, moveSpeed * time_delta));
    }
    CHECK_KEY(GLFW_KEY_D)
    {
        target->TranslateLocal(glm::vec3(moveSpeed * time_delta, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_0)
    {
        vke_common::SceneManager::SaveScene("./tests/scene/test_scene.json");
    }
    CHECK_KEY(GLFW_KEY_1)
    {
        obj->RotateLocal(rotateSpeed * time_delta, glm::vec3(1, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_2)
    {
        obj->RotateLocal(rotateSpeed * time_delta, glm::vec3(0, 1, 0));
    }
    CHECK_KEY(GLFW_KEY_3)
    {
        obj->RotateLocal(rotateSpeed * time_delta, glm::vec3(0, 0, 1));
    }
    CHECK_KEY(GLFW_KEY_4)
    {
        obj->RotateGlobal(rotateSpeed * time_delta, glm::vec3(1, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_5)
    {
        obj->RotateGlobal(rotateSpeed * time_delta, glm::vec3(0, 1, 0));
    }
    CHECK_KEY(GLFW_KEY_6)
    {
        obj->RotateGlobal(rotateSpeed * time_delta, glm::vec3(0, 0, 1));
    }
    CHECK_KEY(GLFW_KEY_8)
    {
        obj->Scale(glm::vec3(0.2 * time_delta, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_9)
    {
        obj->Scale(glm::vec3(-0.2 * time_delta, 0, 0));
    }

    CHECK_KEY(GLFW_KEY_T)
    {
        obj2p->TranslateGlobal(glm::vec3(moveSpeed * time_delta, 0, 0));
        // obj2p->RotateLocal(rotateSpeed * time_delta, glm::vec3(1, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_Y)
    {
        obj2p->TranslateGlobal(glm::vec3(0, moveSpeed * time_delta, 0));
        // obj2p->RotateLocal(rotateSpeed * time_delta, glm::vec3(0, 1, 0));
    }
    CHECK_KEY(GLFW_KEY_U)
    {
        obj2p->TranslateGlobal(glm::vec3(0, 0, moveSpeed * time_delta));
        // obj2p->RotateLocal(rotateSpeed * time_delta, glm::vec3(0, 0, 1));
    }
    CHECK_KEY(GLFW_KEY_G)
    {
        obj2p->RotateGlobal(rotateSpeed * time_delta, glm::vec3(1, 0, 0));
    }
    CHECK_KEY(GLFW_KEY_H)
    {
        obj2p->RotateGlobal(rotateSpeed * time_delta, glm::vec3(0, 1, 0));
    }
    CHECK_KEY(GLFW_KEY_J)
    {
        obj2p->RotateGlobal(rotateSpeed * time_delta, glm::vec3(0, 0, 1));
    }
    static bool pressed = false;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS && !pressed)
    {
        pressed = true;
        obj2p->SetParent(obj2p->parent ? nullptr : objp);
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE)
    {
        pressed = false;
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static bool firstMouse = true;
    static float lastX, lastY;
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camp->RotateGlobal(-xoffset * rotateSpeed * time_delta, glm::vec3(0.0f, 1.0f, 0.0f));
    camp->RotateLocal(yoffset * rotateSpeed * time_delta, glm::vec3(1.0f, 0.0f, 0.0f));
}