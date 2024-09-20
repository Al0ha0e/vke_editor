#include <editor.hpp>
#include <component/renderable_object.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <stack>

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

    void VKEditor::showMainMenuBar()
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("Scene"))
        {
            if (ImGui::MenuItem("New Scene", nullptr, nullptr))
            {
                // TODO
            }
            if (ImGui::MenuItem("Load Scene", nullptr, nullptr))
            {
                // TODO
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

        std::string name = std::to_string(object->id);
        ImGuiTreeNodeFlags currentFlags = commonFlags;
        if (object == selectedObject)
            currentFlags |= ImGuiTreeNodeFlags_Selected;
        if (object->children.size() == 0)
        {
            currentFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGui::TreeNodeEx(name.c_str(), currentFlags);
            handleMouse();
        }
        else
        {
            if (ImGui::TreeNodeEx(name.c_str(), currentFlags))
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
                    std::string name = std::to_string(object->id);
                    if (ImGui::MenuItem(name.c_str(), nullptr, nullptr))
                    {
                        selectedObject->SetParent(object);
                    }
                }
                ImGui::EndMenu();
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
        }

        ImGui::End();
    }

    void VKEditor::showAssets()
    {
        ImGui::Begin("Assets");
        ImGui::Text("This is some useful text.");
        ImGui::End();
    }

    void VKEditor::handleGUILogic()
    {
        instance->showMainMenuBar();
        instance->showHierarchy();
        instance->showInspector();
        instance->showAssets();
    }

    void VKEditor::createGameObject(GameObjectPreset preset)
    {
        vke_common::TransformParameter param(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1), glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
        std::unique_ptr<vke_common::GameObject> object = std::make_unique<vke_common::GameObject>(param);

        if (preset != OBJECT_EMPTY)
        {
            std::shared_ptr<vke_render::Material> material = resourceManager->LoadMaterial("./tests/material/mat1.json");
            std::shared_ptr<const vke_render::Mesh> mesh;
            switch (preset)
            {
            case OBJECT_PLANE:
                mesh = resourceManager->LoadMesh(vke_common::BuiltinPlanePath);
                break;
            case OBJECT_CUBE:
                mesh = resourceManager->LoadMesh(vke_common::BuiltinCubePath);
                break;
            case OBJECT_SPHERE:
                mesh = resourceManager->LoadMesh(vke_common::BuiltinSpherePath);
                break;
            case OBJECT_CYLINDER:
                mesh = resourceManager->LoadMesh(vke_common::BuiltinCylinderPath);
                break;
            case OBJECT_MONKEY:
                mesh = resourceManager->LoadMesh(vke_common::BuiltinMonkeyPath);
                break;
            }
            object->AddComponent(std::make_unique<vke_component::RenderableObject>(material, mesh, object.get()));
        }
        selectedObject = object.get();
        sceneManager->currentScene->AddObject(std::move(object));
    }

    void VKEditor::saveScene()
    {
        vke_common::SceneManager::SaveScene(sceneManager->currentScene->path);
    }
};