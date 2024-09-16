#ifndef VKE_EDITOR_H
#define VKE_EDITOR_H

#include <engine.hpp>
#include <time.hpp>
#include <input.hpp>
#include <ui_render.hpp>

namespace vke_editor
{

    class VKEditor
    {
    private:
        static VKEditor *instance;
        VKEditor() : rightClickedInSceneWindow(false), sceneCameraRotateSpeed(1.5f), sceneCameraMoveSpeed(2.5f) {}
        ~VKEditor() {}
        VKEditor(const VKEditor &);
        VKEditor &operator=(const VKEditor);

    public:
        static VKEditor *GetInstance()
        {
            return instance;
        }

        vke_common::Engine *engine;
        vke_common::InputManager *inputManager;
        vke_common::TimeManager *timeManager;

        static VKEditor *Init()
        {
            vke_render::RenderEnvironment *environment = vke_render::RenderEnvironment::GetInstance();
            instance = new VKEditor();
            instance->inputManager = vke_common::InputManager::GetInstance();
            instance->timeManager = vke_common::TimeManager::GetInstance();
            vke_common::EventSystem::AddEventListener(vke_common::EVENT_MOUSE_CLICK, instance, vke_common::EventCallback(OnMouseClick));

            std::cout << "INI0\n";
            UIRenderer::Init(&(environment->rootRenderContext));
            std::cout << "INI1\n";

            std::vector<vke_render::PassType> passes = {
                vke_render::BASE_RENDERER,
                vke_render::OPAQUE_RENDERER};
            std::vector<std::unique_ptr<vke_render::SubpassBase>> customPasses;
            std::vector<vke_render::RenderPassInfo> customPassInfo;

            instance->engine = vke_common::Engine::Init(&(UIRenderer::GetInstance()->engineRenderContext),
                                                        passes, customPasses, customPassInfo);

            return instance;
        }

        void Update()
        {
            handleEditorLogic();
            engine->Update();
            UIRenderer::Update();
        };

        static void Dispose()
        {
            instance->engine->Dispose();
            UIRenderer::Dispose();
            delete instance;
        }

        static void OnMouseClick(void *listener, void *info)
        {
            vke_common::MouseEventBody *event = (vke_common::MouseEventBody *)info;
            if (event->button == 1)
            {
                if (event->action == 0)
                    instance->rightClickedInSceneWindow = false;
                else
                {
                    ImGuiWindow *sceneWindow = ImGui::FindWindowByName("Scene");
                    auto mousePos = instance->inputManager->mousePos;
                    if (sceneWindow->Pos.x < mousePos.x && mousePos.x < sceneWindow->Pos.x + sceneWindow->Size.x &&
                        sceneWindow->Pos.y < mousePos.y && mousePos.y < sceneWindow->Pos.y + sceneWindow->Size.y)
                    {
                        instance->rightClickedInSceneWindow = true;
                        instance->prevMousePos = mousePos;
                    }
                }
            }
        }

    private:
        bool rightClickedInSceneWindow;
        glm::vec2 prevMousePos;
        float sceneCameraRotateSpeed;
        float sceneCameraMoveSpeed;

        void handleEditorLogic();
    };
}

#endif