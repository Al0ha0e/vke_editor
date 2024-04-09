#ifndef VKE_EDITOR_H
#define VKE_EDITOR_H

#include <engine.hpp>
#include <ui_render.hpp>

namespace vke_editor
{

    class VKEditor
    {
    private:
        static VKEditor *instance;
        VKEditor() {}
        ~VKEditor() {}
        VKEditor(const VKEditor &);
        VKEditor &operator=(const VKEditor);

    public:
        static VKEditor *GetInstance()
        {
            if (instance == nullptr)
                instance = new VKEditor();
            return instance;
        }

        vke_common::Engine *engine;

        static VKEditor *Init(int width, int height)
        {
            instance = new VKEditor();

            std::unique_ptr<vke_editor::UIRenderer> uiRenderer = std::make_unique<vke_editor::UIRenderer>();
            std::vector<vke_render::PassType> passes = {
                vke_render::BASE_RENDERER,
                vke_render::OPAQUE_RENDERER,
                vke_render::CUSTOM_RENDERER};

            std::vector<std::unique_ptr<vke_render::SubpassBase>> customPasses;
            customPasses.push_back(std::move(uiRenderer));

            std::vector<vke_render::RenderPassInfo> customPassInfo{
                vke_render::RenderPassInfo(
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)};

            instance->engine = vke_common::Engine::Init(width, height, passes, customPasses, customPassInfo);

            return instance;
        }

        void Update()
        {
            engine->Update();
        };

        static void Dispose()
        {
            instance->engine->Dispose();
            delete instance;
        }
    };

}

#endif