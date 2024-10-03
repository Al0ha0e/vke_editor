#include <editor.hpp>

namespace vke_editor
{

#define SHOW_COMPONENT_CASE(type, func) \
    case vke_common::type:              \
        func();                         \
        break;

    void VKEditor::showComponents()
    {
        for (auto &component : selectedObject->components)
        {
            switch (component->type)
            {
                SHOW_COMPONENT_CASE(COMPONENT_CAMERA, showCamera)
                SHOW_COMPONENT_CASE(COMPONENT_RENDERABLE_OBJECT, showRenderableObject)
                SHOW_COMPONENT_CASE(COMPONENT_RIGIDBODY, showRigidbody)
            default:
                break;
            }
        }

        if (ImGui::Button("AddComponent")) // TODO
            ;
    }

    void VKEditor::showCamera()
    {
        if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Camera");
            ImGui::TreePop();
        }
    }

    void VKEditor::showRenderableObject()
    {
        if (ImGui::TreeNodeEx("RenderableObject", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("RenderableObject");
            ImGui::TreePop();
        }
    }

    void VKEditor::showRigidbody()
    {
        if (ImGui::TreeNodeEx("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Rigidbody");
            ImGui::TreePop();
        }
    }
};