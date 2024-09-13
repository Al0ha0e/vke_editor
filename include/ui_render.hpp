#ifndef UI_RENDER_H
#define UI_RENDER_H

#include <event.hpp>
#include <render/subpass.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace vke_editor
{
    class UIRenderer
    {
    private:
        static UIRenderer *instance;
        UIRenderer() = default;
        ~UIRenderer() {}

    public:
        vke_render::RenderContext uiRenderContext;
        vke_render::RenderContext engineRenderContext;
        uint32_t currentFrame;

        vke_common::EventHub<vke_render::RenderContext> resizeEventHub;

        static UIRenderer *GetInstance()
        {
            if (instance == nullptr)
                throw std::runtime_error("Renderer not initialized!");
            return instance;
        }

        static UIRenderer *Init(vke_render::RenderContext *ctx)
        {
            instance = new UIRenderer();

            instance->uiRenderContext = *ctx;
            instance->currentFrame = 0;
            vke_render::RenderEnvironment *environment = vke_render::RenderEnvironment::GetInstance();
            instance->environment = environment;

            ctx->resizeEventHub->AddEventListener(instance,
                                                  vke_common::EventHub<vke_render::RenderContext>::callback_t(OnWindowResize));
            instance->engineRenderContext = vke_render::RenderContext{
                ctx->width,
                ctx->height,
                ctx->imageCnt,
                ctx->colorFormat,
                ctx->depthFormat,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                &(instance->imageViews),
                nullptr,
                &(instance->resizeEventHub),
                AcquireNextImage,
                Present};

            instance->createCommandPool();
            instance->createCommandBuffers();
            instance->engineRenderContext.commandBuffers = &(instance->commandBuffers);
            instance->createSyncObjects();
            instance->createImages();
            instance->createImageViews();
            instance->createSamplers();

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
            //  Setup Dear ImGui style
            ImGui::StyleColorsDark();
            // ImGui::StyleColorsLight();

            VkDescriptorPoolSize pool_sizes[] =
                {
                    {VK_DESCRIPTOR_TYPE_SAMPLER, 10},
                    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
                    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10}};
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 100;
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            vkCreateDescriptorPool(environment->logicalDevice, &pool_info, nullptr, &instance->uiDescriptorPool);
            instance->createRenderPass();
            instance->createFramebuffers();

            ImGui_ImplGlfw_InitForVulkan(environment->window, true);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = environment->vkinstance;
            init_info.PhysicalDevice = environment->physicalDevice;
            init_info.Device = environment->logicalDevice;
            init_info.QueueFamily = vke_render::RenderEnvironment::FindQueueFamilies(environment->physicalDevice).graphicsAndComputeFamily.value();
            init_info.Queue = environment->graphicsQueue;
            init_info.PipelineCache = nullptr;
            init_info.DescriptorPool = instance->uiDescriptorPool;
            init_info.RenderPass = instance->renderPass;
            init_info.Subpass = 0;
            init_info.MinImageCount = environment->imageCnt;
            init_info.ImageCount = environment->imageCnt;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = nullptr;
            init_info.CheckVkResultFn = nullptr;
            ImGui_ImplVulkan_Init(&init_info);
            instance->createGUIHandles();
        }

        static void Dispose()
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            VkDevice logicalDevice = instance->environment->logicalDevice;
            vkDestroyRenderPass(logicalDevice, instance->renderPass, nullptr);
            vkDestroyDescriptorPool(logicalDevice, instance->uiDescriptorPool, nullptr);
            for (auto sampler : instance->samplers)
                vkDestroySampler(logicalDevice, sampler, nullptr);
            instance->cleanup();
            for (size_t i = 0; i < instance->engineRenderContext.imageCnt; i++)
            {
                vkDestroySemaphore(logicalDevice, instance->renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(logicalDevice, instance->inFlightFences[i], nullptr);
            }
            vkDestroyCommandPool(logicalDevice, instance->commandPool, nullptr);
            delete instance;
        }

        static void OnWindowResize(void *listener, vke_render::RenderContext *ctx)
        {
            instance->recreate(ctx);
            instance->resizeEventHub.DispatchEvent(&(instance->engineRenderContext));
        }

        static void Update()
        {
            instance->render();
            instance->currentFrame = (instance->currentFrame + 1) % instance->uiRenderContext.imageCnt;
        }

        static uint32_t AcquireNextImage(uint32_t currentFrame)
        {
            currentFrame = instance->currentFrame;
            vkWaitForFences(instance->environment->logicalDevice, 1, &instance->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
            vkResetFences(instance->environment->logicalDevice, 1, &instance->inFlightFences[currentFrame]);
            return instance->currentFrame;
        }

        static void Present(uint32_t currentFrame,
                            uint32_t imageIndex,
                            std::vector<VkSemaphore> &waitSemaphores,
                            std::vector<VkPipelineStageFlags> &waitStages)
        {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = waitSemaphores.size();
            submitInfo.pWaitSemaphores = waitSemaphores.data();
            submitInfo.pWaitDstStageMask = waitStages.data();
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &instance->commandBuffers[currentFrame];
            VkSemaphore signalSemaphores[] = {instance->renderFinishedSemaphores[imageIndex]};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(instance->environment->graphicsQueue, 1, &submitInfo, instance->inFlightFences[imageIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to submit draw command buffer!");
            }
        }

    private:
        VkDescriptorPool uiDescriptorPool;
        vke_render::RenderEnvironment *environment;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        std::vector<VkImage> colorImages;
        std::vector<VkDeviceMemory> colorImageMemories;
        std::vector<VkImageView> colorImageViews;
        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemories;
        std::vector<VkImageView> depthImageViews;
        std::vector<std::vector<VkImageView>> imageViews;
        std::vector<VkSampler> samplers;
        std::vector<VkDescriptorSet> imguiHandles;

        VkRenderPass renderPass;
        std::vector<VkFramebuffer> frameBuffers;

        void cleanup();
        void recreate(vke_render::RenderContext *ctx);
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();
        void createImages();
        void createImageViews();
        void createRenderPass();
        void createSamplers();
        void createFramebuffers();
        void createGUIHandles();
        void render();
    };
}

#endif