#include <ui_render.hpp>

namespace vke_editor
{
    UIRenderer *UIRenderer::instance;

    void UIRenderer::cleanup()
    {
        VkDevice logicalDevice = environment->logicalDevice;

        for (auto framebuffer : frameBuffers)
            vkDestroyFramebuffer(environment->logicalDevice, framebuffer, nullptr);
    }

    void UIRenderer::cleanupSceneWindow()
    {
        VkDevice logicalDevice = environment->logicalDevice;

        for (auto handle : instance->imguiHandles)
            ImGui_ImplVulkan_RemoveTexture(handle);

        for (auto imageView : instance->colorImageViews)
            vkDestroyImageView(logicalDevice, imageView, nullptr);

        for (auto image : instance->colorImages)
            vkDestroyImage(logicalDevice, image, nullptr);

        for (auto imageMemory : instance->colorImageMemories)
            vkFreeMemory(logicalDevice, imageMemory, nullptr);

        for (auto imageView : instance->depthImageViews)
            vkDestroyImageView(logicalDevice, imageView, nullptr);

        for (auto image : instance->depthImages)
            vkDestroyImage(logicalDevice, image, nullptr);

        for (auto imageMemory : instance->depthImageMemories)
            vkFreeMemory(logicalDevice, imageMemory, nullptr);
    }

    void UIRenderer::recreate(vke_render::RenderContext *ctx)
    {
        cleanup();
        uiRenderContext = *ctx;
        createFramebuffers();
    }

    void UIRenderer::recreateSceneWindow()
    {
        cleanupSceneWindow();
        createImages();
        createImageViews();
        createGUIHandles();
    }

    void UIRenderer::createCommandPool()
    {
        vke_render::QueueFamilyIndices queueFamilyIndices = vke_render::RenderEnvironment::FindQueueFamilies(environment->physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();
        if (vkCreateCommandPool(environment->logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void UIRenderer::createCommandBuffers()
    {
        commandBuffers.resize(engineRenderContext.imageCnt);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(environment->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void UIRenderer::createSyncObjects()
    {
        VkDevice logicalDevice = environment->logicalDevice;
        uint32_t imageCnt = engineRenderContext.imageCnt;
        renderFinishedSemaphores.resize(imageCnt);
        inFlightFences.resize(imageCnt);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < imageCnt; i++)
        {
            if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void UIRenderer::createImages()
    {
        uint32_t imageCnt = engineRenderContext.imageCnt;
        uint32_t width = engineRenderContext.width;
        uint32_t height = engineRenderContext.height;

        colorImages.resize(imageCnt);
        colorImageMemories.resize(imageCnt);

        for (int i = 0; i < imageCnt; i++)
            vke_render::RenderEnvironment::CreateImage(
                width, height,
                uiRenderContext.colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                colorImages[i], colorImageMemories[i]);

        depthImages.resize(imageCnt);
        depthImageMemories.resize(imageCnt);

        for (int i = 0; i < imageCnt; i++)
            vke_render::RenderEnvironment::CreateImage(
                width, height,
                engineRenderContext.depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImages[i], depthImageMemories[i]);
    }

    void UIRenderer::createImageViews()
    {
        uint32_t imageCnt = engineRenderContext.imageCnt;
        colorImageViews.resize(imageCnt);
        depthImageViews.resize(imageCnt);
        imageViews.resize(imageCnt);

        for (size_t i = 0; i < imageCnt; i++)
        {
            colorImageViews[i] = vke_render::RenderEnvironment::CreateImageView(colorImages[i], engineRenderContext.colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
            depthImageViews[i] = vke_render::RenderEnvironment::CreateImageView(depthImages[i], engineRenderContext.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
            imageViews[i] = {colorImageViews[i], depthImageViews[i]};
        }
    }

    void UIRenderer::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = uiRenderContext.colorFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = uiRenderContext.outColorLayout;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = VkSubpassDescription{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = VkSubpassDependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // VK_PIPELINE_STAGE_NONE;
        dependency.srcAccessMask = VK_ACCESS_NONE;                               // VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(environment->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void UIRenderer::createSamplers()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplers.resize(engineRenderContext.imageCnt);
        for (int i = 0; i < engineRenderContext.imageCnt; i++)
            if (vkCreateSampler(environment->logicalDevice, &samplerInfo, nullptr, &samplers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create texture sampler!");
            }
    }

    void UIRenderer::createGUIHandles()
    {
        imguiHandles.resize(engineRenderContext.imageCnt);
        for (int i = 0; i < engineRenderContext.imageCnt; i++)
            imguiHandles[i] = ImGui_ImplVulkan_AddTexture(samplers[i], colorImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void UIRenderer::createFramebuffers()
    {
        frameBuffers.resize(uiRenderContext.imageCnt);
        for (size_t i = 0; i < uiRenderContext.imageCnt; i++)
        {
            std::vector<VkImageView> attachments = {(*uiRenderContext.imageViews)[i][0]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = uiRenderContext.width;
            framebufferInfo.height = uiRenderContext.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(environment->logicalDevice, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void UIRenderer::showMainMenuBar()
    {
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("SB"))
        {
            ImGui::MenuItem("AAA", nullptr, nullptr);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    void UIRenderer::showHierarchy()
    {
        ImGui::Begin("Hierarchy");
        ImGui::Text("This is some useful text.");
        if (ImGui::Button("Save"))
            std::cout << "???\n";
        ImGui::End();
    }

    void UIRenderer::showScene()
    {
        ImGui::Begin("Scene");
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();
        ImGui::Image((ImTextureID)(imguiHandles[currentFrame]), ImVec2(vMax.x, vMax.y - 30));
        if (!sceneResized && engineRenderContext.width != vMax.x || engineRenderContext.height != vMax.y)
        {
            engineRenderContext.width = vMax.x;
            engineRenderContext.height = vMax.y;
            std::cout << "RESIZED\n";
            sceneResized = true;
        }
        ImGui::End();
    }

    void UIRenderer::showInspector()
    {
        ImGui::Begin("Inspector");
        ImGui::Text("This is some useful text.");
        ImGui::End();
    }

    void UIRenderer::showAssets()
    {
        ImGui::Begin("Assets");
        ImGui::Text("This is some useful text.");
        ImGui::End();
    }

    void UIRenderer::render()
    {
        uint32_t imageIndex = vke_render::RenderEnvironment::AcquireNextImage(currentFrame);
        VkCommandBuffer commandBuffer = (*uiRenderContext.commandBuffers)[currentFrame];
        vkResetCommandBuffer(commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;                  // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = VkExtent2D{uiRenderContext.width, uiRenderContext.height};
        VkClearValue clearValues[1];
        clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        showMainMenuBar();
        showHierarchy();
        showScene();
        showInspector();
        showAssets();

        ImGuiIO &io = ImGui::GetIO();
        ImGui::Render();

        ImDrawData *draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
        std::vector<VkSemaphore> presentWaitSemaphores{renderFinishedSemaphores[currentFrame]};
        std::vector<VkPipelineStageFlags> presentWaitStages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        vke_render::RenderEnvironment::Present(currentFrame, imageIndex, presentWaitSemaphores, presentWaitStages);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        if (sceneResized)
        {
            recreateSceneWindow();
            resizeEventHub.DispatchEvent(&(instance->engineRenderContext));
            sceneResized = false;
        }
    }
}