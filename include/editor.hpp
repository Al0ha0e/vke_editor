#ifndef VKE_EDITOR_H
#define VKE_EDITOR_H

#include <engine.hpp>
#include <time.hpp>
#include <input.hpp>
#include <ui_render.hpp>
#include <project.hpp>
#include <component/camera.hpp>

namespace vke_editor
{

    enum GameObjectPreset
    {
        OBJECT_EMPTY,
        OBJECT_PLANE,
        OBJECT_CUBE,
        OBJECT_SPHERE,
        OBJECT_CYLINDER,
        OBJECT_MONKEY
    };

    class VKEditor
    {
    private:
        static VKEditor *instance;
        VKEditor()
            : rightClickedInSceneWindow(false), sceneCameraRotateSpeed(1.5f), sceneCameraMoveSpeed(2.5f), selectedObject(nullptr),
              project(nullptr) {}
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
        vke_common::AssetManager *assetManager;
        vke_common::SceneManager *sceneManager;
        std::unique_ptr<Project> project;

        static VKEditor *Init()
        {
            vke_render::RenderEnvironment *environment = vke_render::RenderEnvironment::GetInstance();
            instance = new VKEditor();
            instance->inputManager = vke_common::InputManager::GetInstance();
            instance->timeManager = vke_common::TimeManager::GetInstance();
            vke_common::EventSystem::AddEventListener(vke_common::EVENT_MOUSE_CLICK, instance, vke_common::EventCallback(OnMouseClick));

            std::cout << "INI0\n";
            UIRenderer::Init(&(environment->rootRenderContext), std::function<void()>(handleGUILogic));
            std::cout << "INI1\n";

            std::vector<vke_render::PassType> passes = {
                vke_render::BASE_RENDERER,
                vke_render::OPAQUE_RENDERER};
            std::vector<std::unique_ptr<vke_render::SubpassBase>> customPasses;
            std::vector<vke_render::RenderPassInfo> customPassInfo;

            instance->engine = vke_common::Engine::Init(&(UIRenderer::GetInstance()->engineRenderContext),
                                                        passes, customPasses, customPassInfo);
            instance->assetManager = vke_common::AssetManager::GetInstance();
            instance->sceneManager = vke_common::SceneManager::GetInstance();
            // vke_common::SceneManager::LoadScene("./tests/scene/test_physx_scene.json");
            // instance->sceneCamera = instance->sceneManager->currentScene->objects[1].get();

            instance->sceneCamera = instance->createCameraObject();
            instance->tmpCamera = std::unique_ptr<vke_common::GameObject>(instance->sceneCamera);

            return instance;
        }

        void Update()
        {
            if (sceneManager->currentScene != nullptr)
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
        vke_common::GameObject *sceneCamera;
        std::unique_ptr<vke_common::GameObject> tmpCamera;
        vke_common::GameObject *selectedObject;

        void handleEditorLogic();
        void showInitWindow();
        void showMainMenuBar();
        void showHierarchy();
        void drawHierarchyList(vke_common::GameObject *object, ImGuiTreeNodeFlags commonFlags, bool &showRightMenu);
        void showInspector();
        void showAssets();
        static void handleGUILogic();

        void showComponents();
        void showCamera();
        void showRenderableObject();
        void showRigidbody();

        void createProject(std::string &path);
        void loadProject(std::string &path);
        void saveProject();
        void createScene();
        void loadScene(std::string &path);
        void saveScene();
        void registerSceneCamera(vke_common::Scene *scene);
        vke_common::GameObject *createCameraObject();

        void createGameObject(GameObjectPreset preset);
    };
}

#endif