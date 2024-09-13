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

        static VKEditor *Init()
        {
            vke_render::RenderEnvironment *environment = vke_render::RenderEnvironment::GetInstance();
            instance = new VKEditor();
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
            engine->Update();
            UIRenderer::Update();
        };

        static void Dispose()
        {
            // instance->engine->Dispose();
            UIRenderer::Dispose();
            delete instance;
        }
    };

}

#endif