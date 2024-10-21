#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <nfd.h>

#include <stack>

#include <editor.hpp>
#include <component/renderable_object.hpp>

namespace vke_editor
{
    VKEditor *VKEditor::instance;

    void VKEditor::handleEditorLogic()
    {
        auto mousePos = inputManager->mousePos;
        if (rightClickedInSceneWindow)
        {
            float deltaTime = timeManager->deltaTime;
            float xoffset = mousePos.x - prevMousePos.x;
            float yoffset = prevMousePos.y - mousePos.y;
            prevMousePos = mousePos;

            sceneCamera->RotateGlobal(-xoffset * sceneCameraRotateSpeed * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
            sceneCamera->RotateLocal(yoffset * sceneCameraRotateSpeed * deltaTime, glm::vec3(1.0f, 0.0f, 0.0f));

            if (inputManager->KeyPressed(vke_common::KEY_W))
            {
                sceneCamera->TranslateLocal(glm::vec3(0, 0, -sceneCameraMoveSpeed * deltaTime));
            }
            if (inputManager->KeyPressed(vke_common::KEY_A))
            {
                sceneCamera->TranslateLocal(glm::vec3(-sceneCameraMoveSpeed * deltaTime, 0, 0));
            }
            if (inputManager->KeyPressed(vke_common::KEY_S))
            {
                sceneCamera->TranslateLocal(glm::vec3(0, 0, sceneCameraMoveSpeed * deltaTime));
            }
            if (inputManager->KeyPressed(vke_common::KEY_D))
            {
                sceneCamera->TranslateLocal(glm::vec3(sceneCameraMoveSpeed * deltaTime, 0, 0));
            }
        }
    }

    void VKEditor::showInitWindow()
    {
        ImGui::Begin("Project");
        ImGui::Text("This is some useful text.");
        if (ImGui::Button("Open Project"))
        {
            nfdchar_t *path = NULL;
            nfdresult_t result = NFD_PickFolder(NULL, &path);
            // TODO Check Path
            if (result == NFD_OKAY)
            {
                puts("Success!");
                puts(path);
                std::string pth(path);

                for (int i = 0; i < pth.length(); i++)
                    if (pth[i] == '\\')
                        pth[i] = '/';
                loadProject(pth);
                free(path);
            }
        }

        if (ImGui::Button("Create Project"))
        {
            nfdchar_t *path = NULL;
            nfdresult_t result = NFD_PickFolder(NULL, &path);
            // TODO Check Path
            if (result == NFD_OKAY)
            {
                puts(path);
                std::string pth(path);

                for (int i = 0; i < pth.length(); i++)
                    if (pth[i] == '\\')
                        pth[i] = '/';

                createProject(pth);
                free(path);
            }
        }
        ImGui::End();
    }

    void VKEditor::showMainMenuBar()
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("Project"))
        {
            if (ImGui::MenuItem("Save Project", nullptr, nullptr))
                saveProject();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("New Scene", nullptr, nullptr))
            {
                createScene();
            }
            if (ImGui::MenuItem("Save Scene", nullptr, nullptr))
            {
                saveScene();
            }
            if (ImGui::BeginMenu("New Object"))
            {
                if (ImGui::MenuItem("Empty", nullptr, nullptr))
                    createGameObject(OBJECT_EMPTY);

                if (ImGui::MenuItem("Plane", nullptr, nullptr))
                    createGameObject(OBJECT_PLANE);

                if (ImGui::MenuItem("Cube", nullptr, nullptr))
                    createGameObject(OBJECT_CUBE);

                if (ImGui::MenuItem("Sphere", nullptr, nullptr))
                    createGameObject(OBJECT_SPHERE);

                if (ImGui::MenuItem("Cylinder", nullptr, nullptr))
                    createGameObject(OBJECT_CYLINDER);

                if (ImGui::MenuItem("Monkey", nullptr, nullptr))
                    createGameObject(OBJECT_MONKEY);

                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Assets"))
        {
            std::string name = "AAA";
            std::string path = "BBB";

            if (ImGui::MenuItem("Create Texture", nullptr, nullptr))
                assetManager->CreateTextureAsset(name, path);
            if (ImGui::MenuItem("Create VFShader", nullptr, nullptr))
                assetManager->CreateVFShaderAsset(name, path);
            if (ImGui::MenuItem("Create ComputeShader", nullptr, nullptr))
                assetManager->CreateComputeShaderAsset(name, path);
            if (ImGui::MenuItem("Create Material", nullptr, nullptr))
                assetManager->CreateMaterialAsset(name, path);
            if (ImGui::MenuItem("Create PhysicalMaterial", nullptr, nullptr))
                assetManager->CreatePhysicsMaterialAsset(name, path);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    void VKEditor::drawHierarchyList(vke_common::GameObject *object, ImGuiTreeNodeFlags commonFlags, bool &showRightMenu)
    {
        auto handleMouse = [&]()
        {
            if (ImGui::IsItemClicked())
                selectedObject = object;
            if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered())
            {
                selectedObject = object;
                showRightMenu = true;
            }
        };

        ImGuiTreeNodeFlags currentFlags = commonFlags;
        if (object == selectedObject)
            currentFlags |= ImGuiTreeNodeFlags_Selected;
        if (object->children.size() == 0)
        {
            currentFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(object->name, currentFlags);
            handleMouse();
        }
        else
        {
            if (ImGui::TreeNodeEx(object->name, currentFlags))
            {
                handleMouse();
                for (auto &kv : object->children)
                    drawHierarchyList(kv.second, commonFlags, showRightMenu);
                ImGui::TreePop();
            }
            else
            {
                handleMouse();
            }
        }
    }

    void VKEditor::showHierarchy()
    {
        vke_common::Scene *scene = sceneManager->currentScene.get();

        ImGuiTreeNodeFlags commonFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

        ImGui::Begin("Hierarchy");

        bool showRightMenu = false;
        for (auto &kv : scene->objects)
        {
            if (kv.second->parent == nullptr)
                drawHierarchyList(kv.second.get(), commonFlags, showRightMenu);
        }

        if (showRightMenu)
            ImGui::OpenPopup("objright");

        if (ImGui::BeginPopup("objright"))
        {
            if (ImGui::BeginMenu("Set Parent"))
            {
                if (ImGui::MenuItem("none", nullptr, nullptr))
                {
                    selectedObject->SetParent(nullptr);
                }
                for (auto &kv : scene->objects)
                {
                    vke_common::GameObject *object = kv.second.get();
                    if (object == selectedObject)
                        continue;

                    if (ImGui::MenuItem(object->name, nullptr, nullptr))
                    {
                        selectedObject->SetParent(object);
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Delete Object"))
            {
                sceneManager->currentScene->RemoveObject(selectedObject->id);
                selectedObject = nullptr;
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void VKEditor::showInspector()
    {
        ImGui::Begin("Inspector");

        if (selectedObject != nullptr)
        {
            int layer_cnt = sceneManager->currentScene->layers.size();
            std::vector<const char *> layers(layer_cnt);
            for (int i = 0; i < layer_cnt; i++)
                layers[i] = sceneManager->currentScene->layers[i].c_str();
            ImGui::Checkbox("static", &(selectedObject->isStatic));
            ImGui::InputText("name", selectedObject->name, 32);
            ImGui::Combo("layer", &(selectedObject->layer), layers.data(), layer_cnt);
            if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                glm::vec3 position = selectedObject->transform.localPosition;
                glm::vec3 rotation = glm::degrees(glm::eulerAngles(selectedObject->transform.localRotation));
                glm::vec3 scale = selectedObject->transform.localScale;

                if (ImGui::InputFloat3("position", glm::value_ptr(position)))
                    selectedObject->SetLocalPosition(position);
                if (ImGui::InputFloat3("rotation", glm::value_ptr(rotation)))
                {
                    selectedObject->SetLocalRotation(glm::quat(glm::radians(rotation)));
                }
                if (ImGui::InputFloat3("scale", glm::value_ptr(scale)))
                    selectedObject->SetLocalScale(scale);
                ImGui::TreePop();
            }

            showComponents();
        }

        ImGui::End();
    }

#define SHOW_ASSETS(cache)                                                                   \
    for (auto &kv : assetManager->cache)                                                     \
    {                                                                                        \
        ImGui::Text((vke_common::AssetTypeToName[kv.second.type] + kv.second.name).c_str()); \
        ImGui::NextColumn();                                                                 \
    }

    void VKEditor::showAssets()
    {
        ImGui::Begin("Assets");

        int width = ImGui::GetWindowSize().x;
        int unitWidth = 128;
        ImGui::Columns(std::max(width / unitWidth, 1), nullptr, false);

        SHOW_ASSETS(textureCache)
        SHOW_ASSETS(meshCache)
        SHOW_ASSETS(vfShaderCache)
        SHOW_ASSETS(computeShaderCache)
        SHOW_ASSETS(materialCache)
        SHOW_ASSETS(physicsMaterialCache)
        SHOW_ASSETS(sceneCache)

        ImGui::End();
    }

    void VKEditor::handleGUILogic()
    {
        if (instance->sceneManager->currentScene != nullptr)
        {
            instance->showMainMenuBar();
            instance->showHierarchy();
            instance->showInspector();
            instance->showAssets();
        }
        else
        {
            instance->showInitWindow();
        }
    }

    void VKEditor::createGameObject(GameObjectPreset preset)
    {
        vke_common::TransformParameter param(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1), glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
        std::unique_ptr<vke_common::GameObject> object = std::make_unique<vke_common::GameObject>(param);

        if (preset != OBJECT_EMPTY)
        {
            std::shared_ptr<vke_render::Material> material = assetManager->LoadMaterial(vke_common::BUILTIN_MATERIAL_DEFAULT_ID);
            std::shared_ptr<const vke_render::Mesh> mesh;
            switch (preset)
            {
            case OBJECT_PLANE:
                mesh = assetManager->LoadMesh(vke_common::BUILTIN_MESH_PLANE_ID);
                break;
            case OBJECT_CUBE:
                mesh = assetManager->LoadMesh(vke_common::BUILTIN_MESH_CUBE_ID);
                break;
            case OBJECT_SPHERE:
                mesh = assetManager->LoadMesh(vke_common::BUILTIN_MESH_SPHERE_ID);
                break;
            case OBJECT_CYLINDER:
                mesh = assetManager->LoadMesh(vke_common::BUILTIN_MESH_CYLINDER_ID);
                break;
            case OBJECT_MONKEY:
                mesh = assetManager->LoadMesh(vke_common::BUILTIN_MESH_MONKEY_ID);
                break;
            }
            object->AddComponent(std::make_unique<vke_component::RenderableObject>(material, mesh, object.get()));
        }
        selectedObject = object.get();
        sceneManager->currentScene->AddObject(std::move(object));
    }

    vke_common::GameObject *VKEditor::createCameraObject()
    {
        vke_common::TransformParameter camParam(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(1), glm::quat(1, 0, 0, 0));
        vke_common::GameObject *cameraGameObj = new vke_common::GameObject(camParam);
        memcpy(cameraGameObj->name, "SceneCam", 9);
        cameraGameObj->layer = 1;
        cameraGameObj->AddComponent(std::make_unique<vke_component::Camera>(105, 800, 600, 0.01, 1000, cameraGameObj));
        return cameraGameObj;
    }

    void VKEditor::registerSceneCamera(vke_common::Scene *scene)
    {
        // TODO delete cameraGameObj
        // TODO ImGuiWindow *sceneWindow = ImGui::FindWindowByName("Scene");
        // vke_common::TransformParameter camParam(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(1), glm::quat(1.0, 0.0, 0.0, 0.0));
        // vke_common::GameObject *cameraGameObj = new vke_common::GameObject(camParam);
        // cameraGameObj->layer = 1;
        // cameraGameObj->AddComponent(std::make_unique<vke_component::Camera>(105, 800, 600, 0.01, 1000, cameraGameObj));
        // sceneCamera = cameraGameObj;
        uint32_t id = ((vke_component::Camera *)(sceneCamera->components[0].get()))->id;
        vke_render::Renderer::SetCurrentCamera(id);
        scene->AddObject(std::unique_ptr<vke_common::GameObject>(sceneCamera));
    }

    void VKEditor::createScene()
    {
        vke_common::AssetHandle id = assetManager->AllocateAssetID(vke_common::ASSET_SCENE);
        std::string name = "scene" + std::to_string(id);
        std::string path = project->projectPath + "/" + name + ".json";
        vke_common::SceneAsset asset(id, name, path);
        assetManager->SetSceneAsset(id, asset);

        std::unique_ptr<vke_common::Scene> scene = std::make_unique<vke_common::Scene>();
        scene->path = path;
        sceneCamera = createCameraObject();
        sceneManager->SetCurrentScene(std::move(scene));
        registerSceneCamera(sceneManager->currentScene.get());
        sceneManager->SaveScene(sceneManager->currentScene->path);
    }

    void VKEditor::saveScene()
    {
        vke_common::SceneManager::SaveScene(sceneManager->currentScene->path);
    }

    void VKEditor::loadScene(std::string &path)
    {
        sceneManager->LoadScene(path);
        sceneCamera = createCameraObject();
        registerSceneCamera(sceneManager->currentScene.get());
    }

    void VKEditor::createProject(std::string &path)
    {
        project = std::make_unique<Project>(path);
        assetManager->ClearAssetLUT();
        createScene();
        saveProject();
    }

    void VKEditor::loadProject(std::string &path)
    {
        project = std::make_unique<Project>(path, assetManager->LoadJSON(path + "/project.json"));
        assetManager->ClearAssetLUT();
        assetManager->LoadAssetLUT(project->assetLUTPath);
        vke_common::SceneAsset *sceneAsset = assetManager->GetSceneAsset(1);
        loadScene(sceneAsset->path);
    }

    void VKEditor::saveProject()
    {
        assetManager->SaveAssetLUT(project->assetLUTPath);
        saveScene();
        std::ofstream ofs(project->projectPath + "/project.json");
        ofs << project->ToJSON();
        ofs.close();
    }
};