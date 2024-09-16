#include <editor.hpp>
#include <imgui_internal.h>

namespace vke_editor
{
    VKEditor *VKEditor::instance;

    void VKEditor::handleEditorLogic()
    {
        auto mousePos = inputManager->mousePos;
        if (rightClickedInSceneWindow)
        {
            float deltaTime = timeManager->deltaTime;
            float xoffset = prevMousePos.x - mousePos.x;
            float yoffset = prevMousePos.y - mousePos.y;
            prevMousePos = mousePos;
            vke_common::GameObject *camp = vke_common::SceneManager::GetInstance()->currentScene->objects[1].get();

            camp->RotateGlobal(-xoffset * sceneCameraRotateSpeed * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
            camp->RotateLocal(yoffset * sceneCameraRotateSpeed * deltaTime, glm::vec3(1.0f, 0.0f, 0.0f));

            if (inputManager->KeyPressed(vke_common::KEY_W))
            {
                camp->TranslateLocal(glm::vec3(0, 0, -sceneCameraMoveSpeed * deltaTime));
            }
            if (inputManager->KeyPressed(vke_common::KEY_A))
            {
                camp->TranslateLocal(glm::vec3(sceneCameraMoveSpeed * deltaTime, 0, 0));
            }
            if (inputManager->KeyPressed(vke_common::KEY_S))
            {
                camp->TranslateLocal(glm::vec3(0, 0, sceneCameraMoveSpeed * deltaTime));
            }
            if (inputManager->KeyPressed(vke_common::KEY_D))
            {
                camp->TranslateLocal(glm::vec3(-sceneCameraMoveSpeed * deltaTime, 0, 0));
            }
        }
    }
};