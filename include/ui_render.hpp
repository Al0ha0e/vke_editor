#ifndef UI_RENDER_H
#define UI_RENDER_H

#include <event.hpp>
#include <render/subpass.hpp>
#include <render/environment.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace vke_editor
{
    class UIRenderer : public vke_render::SubpassBase
    {
    public:
        ~UIRenderer()
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            vkDestroyDescriptorPool(environment->logicalDevice, uiDescriptorPool, nullptr);
        }

        void Init(int subpassID, VkRenderPass renderPass) override
        {
            environment = vke_render::RenderEnvironment::GetInstance();
            SubpassBase::Init(subpassID, renderPass);

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            // ImGui::StyleColorsLight();

            VkDescriptorPoolSize pool_sizes[] =
                {
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
                };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1;
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            vkCreateDescriptorPool(environment->logicalDevice, &pool_info, nullptr, &uiDescriptorPool);

            ImGui_ImplGlfw_InitForVulkan(environment->window, true);
            init_info = {};
            init_info.Instance = environment->vkinstance;
            init_info.PhysicalDevice = environment->physicalDevice;
            init_info.Device = environment->logicalDevice;
            init_info.QueueFamily = vke_render::RenderEnvironment::FindQueueFamilies(environment->physicalDevice).graphicsAndComputeFamily.value();
            init_info.Queue = environment->graphicsQueue;
            init_info.PipelineCache = nullptr;
            init_info.DescriptorPool = uiDescriptorPool;
            init_info.RenderPass = renderPass;
            init_info.Subpass = subpassID;
            init_info.MinImageCount = 2;
            init_info.ImageCount = vke_render::Renderer::GetInstance()->imageCount;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = nullptr;
            init_info.CheckVkResultFn = nullptr;
            ImGui_ImplVulkan_Init(&init_info);
        }

        void RegisterCamera(VkBuffer buffer) override {}

        void Render(VkCommandBuffer commandBuffer) override
        {
            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Hello, world!");
            ImGui::Text("This is some useful text.");
            ImGui::End();

            ImGui::Render();
            ImDrawData *draw_data = ImGui::GetDrawData();
            ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
        }

    private:
        int resizeListenerID;
        VkDescriptorPool uiDescriptorPool;
        vke_render::RenderEnvironment *environment;
        ImGui_ImplVulkan_InitInfo init_info;
    };

}

#endif