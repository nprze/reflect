#include "bloom.h"
#include "context.h"
#include "renderer_p/renderer.h"

namespace rfct {
    constexpr uint32_t count = RFCT_FRAMES_IN_FLIGHT + 1;
    constexpr uint32_t bloomMultiply = 3;
    // helper function
    void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
		RFCT_PROFILE_FUNCTION();
        vk::ImageSubresourceRange subresourceRange = {
            vk::ImageAspectFlagBits::eColor,
            0, 1,
            0, 1
        };
        vk::AccessFlags srcAccessMask;
        vk::AccessFlags dstAccessMask;
        vk::PipelineStageFlags srcStage;
        vk::PipelineStageFlags dstStage;

        if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            dstAccessMask = vk::AccessFlagBits::eShaderRead;
            srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
            srcAccessMask = vk::AccessFlagBits::eShaderRead;
            dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            srcStage = vk::PipelineStageFlagBits::eFragmentShader;
            dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        }
        else {
            RFCT_CRITICAL("Unsupported layout transition");
        }

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = subresourceRange;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;

        commandBuffer.pipelineBarrier(
            srcStage,
            dstStage,
            vk::DependencyFlags{},
            nullptr, nullptr,
            barrier
        );
    }

    // pipeline layouts
    layoutTemporaryHolder TresholdPipelineLayout() {
        RFCT_PROFILE_FUNCTION();
        // descriptor set layout
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.bindingCount = 1;
        layoutCreateInfo.pBindings = &layoutBinding;
        vk::DescriptorSetLayout descSetLayout = renderer::getRen().getDevice().createDescriptorSetLayout(layoutCreateInfo);

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.setLayoutCount = 1;
        vk::DescriptorSetLayout dscSetLayouts[] = { descSetLayout };
        pipelineLayoutInfo.pSetLayouts = dscSetLayouts;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        vk::PipelineLayout lay =  renderer::getRen().getDevice().createPipelineLayout(pipelineLayoutInfo);

        layoutTemporaryHolder holder;
        holder.descSet = descSetLayout;
        holder.pipeline = lay;
        return holder;
    }
    
    layoutTemporaryHolder GaussianBlurPipelineLayout() {
        RFCT_PROFILE_FUNCTION();
        // descriptor set layout
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.bindingCount = 1;
        layoutCreateInfo.pBindings = &layoutBinding;

        vk::DescriptorSetLayout descSetLayout = renderer::getRen().getDevice().createDescriptorSetLayout(layoutCreateInfo);

        vk::PushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eFragment;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(gaussianPushConstants);

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.setLayoutCount = 1;
        vk::DescriptorSetLayout dscSetLayouts[] = { descSetLayout };
        pipelineLayoutInfo.pSetLayouts = dscSetLayouts;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        vk::PipelineLayout lay =  renderer::getRen().getDevice().createPipelineLayout(pipelineLayoutInfo);

        layoutTemporaryHolder holder;
        holder.descSet = descSetLayout;
        holder.pipeline = lay;
        return holder;
    }
    
    layoutTemporaryHolder CompositePipelineLayout() {
        RFCT_PROFILE_FUNCTION();
        // descriptor set layout
        vk::DescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = 0;
        layoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutBinding layoutBinding1 = {};
        layoutBinding1.binding = 1;
        layoutBinding1.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        layoutBinding1.descriptorCount = 1;
        layoutBinding1.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { layoutBinding, layoutBinding1 };
        layoutCreateInfo.bindingCount = 2;
        layoutCreateInfo.pBindings = bindings.data();

        vk::DescriptorSetLayout descSetLayout = renderer::getRen().getDevice().createDescriptorSetLayout(layoutCreateInfo);

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.setLayoutCount = 1;
        vk::DescriptorSetLayout dscSetLayouts[] = { descSetLayout };
        pipelineLayoutInfo.pSetLayouts = dscSetLayouts;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        vk::PipelineLayout lay =  renderer::getRen().getDevice().createPipelineLayout(pipelineLayoutInfo);

        layoutTemporaryHolder holder;
        holder.descSet = descSetLayout;
        holder.pipeline = lay;
        return holder;
    }

	bloomSamplerHolder::bloomSamplerHolder() {
        RFCT_PROFILE_FUNCTION();
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;

        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;

        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        m_sampler = renderer::getRen().getDevice().createSamplerUnique(samplerInfo);
	}

    bloomResurcesHolder::bloomResurcesHolder(vk::RenderPass renderPass) 
        : vertexShader("shaders/post_proc/fullscreen_vert.spv"),
        m_imageSampler(),
        m_gaussianPipeline(renderPass, &vertexShader, "shaders/post_proc/gaussian_blur_frag.spv", GaussianBlurPipelineLayout()),
        m_compositePipeline(renderPass, &vertexShader, "shaders/post_proc/composite_frag.spv", CompositePipelineLayout()) {
        RFCT_PROFILE_FUNCTION();
        // descriptor pool
        vk::DescriptorPoolSize poolSize = {};
        poolSize.type = vk::DescriptorType::eCombinedImageSampler;
        poolSize.descriptorCount = count * (1 + 1 + 1 + 2);

        vk::DescriptorPoolCreateInfo poolInfo = {};
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = count * 4;

        m_descriptorPool = renderer::getRen().getDevice().createDescriptorPoolUnique(poolInfo);
        {
            vk::DescriptorSetLayout dsLayout = m_gaussianPipeline.m_descSetLayout;

            std::array<vk::DescriptorSetLayout, count> sets = {};

            for (uint32_t i = 0; i < count; ++i) {
                sets[i] = dsLayout;
            }
            vk::DescriptorSetAllocateInfo allocInfo(
                m_descriptorPool.get(),
                count,
                sets.data()
            );

            m_gaussian1SceneImageDescriptorSet = std::move(renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo));
        }

        {
            vk::DescriptorSetLayout dsLayout = m_gaussianPipeline.m_descSetLayout;
            std::array<vk::DescriptorSetLayout, count> sets = {};

            for (uint32_t i = 0; i < count; ++i) {
                sets[i] = dsLayout;
            }
            vk::DescriptorSetAllocateInfo allocInfo(
                m_descriptorPool.get(),
                count,
                sets.data()
            );

            m_gaussian2SceneImageDescriptorSet = std::move(renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo));
        }

        {
            vk::DescriptorSetLayout dsLayout = m_compositePipeline.m_descSetLayout;
            std::array<vk::DescriptorSetLayout, count> sets = {};

            for (uint32_t i = 0; i < count; ++i) {
                sets[i] = dsLayout;
            }
            vk::DescriptorSetAllocateInfo allocInfo(
                m_descriptorPool.get(),
                count,
                sets.data()
            );

            m_compositeImageDescriptorSet = std::move(renderer::getRen().getDevice().allocateDescriptorSetsUnique(allocInfo));
        }
        updateDescSets();
        // create command buffers
        vk::CommandPoolCreateInfo cmdpoolInfo {
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex()
        };
        m_bloomCommandPool = renderer::getRen().getDevice().createCommandPoolUnique(cmdpoolInfo);

        vk::CommandBufferAllocateInfo allocInfoBloom{ *m_bloomCommandPool, vk::CommandBufferLevel::ePrimary, RFCT_FRAMES_IN_FLIGHT };
        m_bloomCommandBuffer = std::move(renderer::getRen().getDevice().allocateCommandBuffersUnique(allocInfoBloom));
    }

    void bloomResurcesHolder::updateDescSets() {
        RFCT_PROFILE_FUNCTION();
        // update descriptor sets
        for (size_t i = 0; i < count; ++i) {
            {
                // blur 1
                vk::DescriptorImageInfo imageInfo;

                imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                imageInfo.imageView = renderer::getRen().getRenderImagesManager().getSceneImageView(i);
                imageInfo.sampler = m_imageSampler.m_sampler.get();

                vk::WriteDescriptorSet writeDescriptorSet = {};
                writeDescriptorSet.dstSet = m_gaussian1SceneImageDescriptorSet[i].get();
                writeDescriptorSet.dstBinding = 0;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.pImageInfo = &imageInfo;

                renderer::getRen().getDevice().updateDescriptorSets({ writeDescriptorSet }, nullptr);
            }
            {
                // blur 2
                vk::DescriptorImageInfo imageInfo;

                imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                imageInfo.imageView = renderer::getRen().getRenderImagesManager().getBloom2ImageView(i);
                imageInfo.sampler = m_imageSampler.m_sampler.get();

                vk::WriteDescriptorSet writeDescriptorSet = {};
                writeDescriptorSet.dstSet = m_gaussian2SceneImageDescriptorSet[i].get();
                writeDescriptorSet.dstBinding = 0;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.pImageInfo = &imageInfo;

                renderer::getRen().getDevice().updateDescriptorSets({ writeDescriptorSet }, nullptr);
            }
            {
                // composite
                vk::DescriptorImageInfo imageInfo0;
                imageInfo0.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                imageInfo0.imageView = renderer::getRen().getRenderImagesManager().getSceneImageView(i); // Image 0
                imageInfo0.sampler = m_imageSampler.m_sampler.get();

                vk::WriteDescriptorSet writeDescriptorSet0 = {};
                writeDescriptorSet0.dstSet = m_compositeImageDescriptorSet[i].get();
                writeDescriptorSet0.dstBinding = 0;
                writeDescriptorSet0.dstArrayElement = 0;
                writeDescriptorSet0.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                writeDescriptorSet0.descriptorCount = 1;
                writeDescriptorSet0.pImageInfo = &imageInfo0;

                vk::DescriptorImageInfo imageInfo1;
                imageInfo1.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                imageInfo1.imageView = renderer::getRen().getRenderImagesManager().getBloom1ImageView(i); // Image 1
                imageInfo1.sampler = m_imageSampler.m_sampler.get();

                vk::WriteDescriptorSet writeDescriptorSet1 = {};
                writeDescriptorSet1.dstSet = m_compositeImageDescriptorSet[i].get();
                writeDescriptorSet1.dstBinding = 1;
                writeDescriptorSet1.dstArrayElement = 0;
                writeDescriptorSet1.descriptorType = vk::DescriptorType::eCombinedImageSampler;
                writeDescriptorSet1.descriptorCount = 1;
                writeDescriptorSet1.pImageInfo = &imageInfo1;

                std::array<vk::WriteDescriptorSet, 2> writeSets = { writeDescriptorSet0, writeDescriptorSet1 };
                renderer::getRen().getDevice().updateDescriptorSets(writeSets, nullptr);
            }
        }
    }

    void bloomResurcesHolder::blum(frameContext* ctx, frameData& fd, vk::RenderPass renderPass, uint32_t imageIndex) {
        RFCT_PROFILE_FUNCTION();
        recordCommandBuffer(m_bloomCommandBuffer[ctx->frame].get(), renderer::getRen().getRenderImagesManager().getIntermediateClearRenderPass(), ctx->frame, imageIndex);
        fd.m_BloomCommandBuffer = m_bloomCommandBuffer[ctx->frame].get();
    }

    void bloomResurcesHolder::recordCommandBuffer(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, uint32_t imageIndex, uint32_t swapchainImage) {
		RFCT_PROFILE_FUNCTION();
        commandBuffer.reset({});
        vk::CommandBufferBeginInfo beginInfo = {};
        commandBuffer.begin(beginInfo);
        transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getSceneImage(imageIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        {
            // bloom 0 pipeline
            vk::RenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = renderer::getRen().getRenderImagesManager().getBloom2FrameBuffer(imageIndex);
            renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
            renderPassInfo.renderArea.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
            renderPassInfo.clearValueCount = 1;
            vk::ClearValue clearColor = {};
            clearColor.color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
            renderPassInfo.pClearValues = &clearColor;

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

            vk::Viewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width);
            viewport.height = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            commandBuffer.setViewport(0, viewport);
            vk::Rect2D scissor = {};
            scissor.offset = vk::Offset2D{ 0, 0 };
            scissor.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
            commandBuffer.setScissor(0, scissor);

            // Descriptors
            vk::DescriptorSet descSets[] = { m_gaussian1SceneImageDescriptorSet[imageIndex].get() };
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipelineLayout, 0, descSets, {});

            gaussianPushConstants pc;
            pc.dir = glm::vec2(1.f, 0.f);
            pc.res = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width / bloomMultiply;

            commandBuffer.pushConstants(
                m_gaussianPipeline.m_pipelineLayout,
                vk::ShaderStageFlagBits::eFragment,
                0,
                sizeof(gaussianPushConstants),
                &pc
            );

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipeline.get());
            commandBuffer.draw(3, 1, 0, 0);
            commandBuffer.endRenderPass();
        }
        {
            // bloom 1 pipeline
            transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getBloom2Image(imageIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
            //transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getBloom1Image(imageIndex), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);

            vk::RenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = renderer::getRen().getRenderImagesManager().getBloom1FrameBuffer(imageIndex);
            renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
            renderPassInfo.renderArea.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
            renderPassInfo.clearValueCount = 1;
            vk::ClearValue clearColor = {};
            clearColor.color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
            renderPassInfo.pClearValues = &clearColor;

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

            vk::Viewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width);
            viewport.height = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            commandBuffer.setViewport(0, viewport);
            vk::Rect2D scissor = {};
            scissor.offset = vk::Offset2D{ 0, 0 };
            scissor.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
            commandBuffer.setScissor(0, scissor);

            // Descriptors
            vk::DescriptorSet descSets[] = { m_gaussian2SceneImageDescriptorSet[imageIndex].get() };
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipelineLayout, 0, descSets, {});

            gaussianPushConstants pc;
            pc.dir = glm::vec2(0.f, 1.f);
            pc.res = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height / bloomMultiply;

            commandBuffer.pushConstants(
                m_gaussianPipeline.m_pipelineLayout,
                vk::ShaderStageFlagBits::eFragment,
                0,
                sizeof(gaussianPushConstants),
                &pc
            );

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_gaussianPipeline.m_pipeline.get());
            commandBuffer.draw(3, 1, 0, 0);
            commandBuffer.endRenderPass();
        }
        {
            // composite pipeline
            transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getBloom1Image(imageIndex), vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

            vk::RenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.renderPass = renderer::getRen().getRenderImagesManager().getpresentToColorAttachmentRenderPass();
            renderPassInfo.framebuffer = renderer::getRen().getRenderImagesManager().getSwapChainFrameBuffer(swapchainImage);
            renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
            renderPassInfo.renderArea.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
            renderPassInfo.clearValueCount = 1;
            vk::ClearValue clearColor = {};
            clearColor.color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
            renderPassInfo.pClearValues = &clearColor;

            commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

            vk::Viewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width);
            viewport.height = static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            commandBuffer.setViewport(0, viewport);
            vk::Rect2D scissor = {};
            scissor.offset = vk::Offset2D{ 0, 0 };
            scissor.extent = rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent();
            commandBuffer.setScissor(0, scissor);

            // Descriptors
            vk::DescriptorSet descSets[] = { m_compositeImageDescriptorSet[imageIndex].get() };
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_compositePipeline.m_pipelineLayout, 0, descSets, {});

            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_compositePipeline.m_pipeline.get());
            commandBuffer.draw(3, 1, 0, 0);
            commandBuffer.endRenderPass();
        }
        transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getSceneImage(imageIndex), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
        transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getBloom1Image(imageIndex), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
        transitionImageLayout(commandBuffer, renderer::getRen().getRenderImagesManager().getBloom2Image(imageIndex), vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);

        commandBuffer.end();
    }

    void bloomResurcesHolder::onSwapchainExtentChanged() {
		RFCT_PROFILE_FUNCTION();
        updateDescSets();
    }

    postprocPipeline::postprocPipeline(vk::RenderPass renderPass, vulkanShader* shaderRef, const std::string& fragmentShaderPath, layoutTemporaryHolder pipelineLayoutStuff):
        m_vertexShader(shaderRef), 
        m_fragShader(fragmentShaderPath), 
        m_pipelineLayout(pipelineLayoutStuff.pipeline),
        m_descSetLayout(pipelineLayoutStuff.descSet)
    { 
        RFCT_PROFILE_FUNCTION();
        // Shaders
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
        vertShaderStageInfo.module = m_vertexShader->getShaderModule();
        vertShaderStageInfo.pName = "main";

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
        fragShaderStageInfo.module = m_fragShader.getShaderModule();
        fragShaderStageInfo.pName = "main";

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = VK_NULL_HANDLE;
        vertexInputInfo.pVertexAttributeDescriptions = VK_NULL_HANDLE;

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Rasterization State
        vk::PipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;

        rasterizer.cullMode = vk::CullModeFlagBits::eNone;
        rasterizer.frontFace = vk::FrontFace::eClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Multisample State
        vk::PipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        vk::PipelineDepthStencilStateCreateInfo depthStencil = {};

        // Dynamic State
        std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineViewportStateCreateInfo viewportState = {};
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Pipeline
        vk::GraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        m_pipeline = renderer::getRen().getDevice().createGraphicsPipelineUnique({}, pipelineInfo).value;
    }

    postprocPipeline::~postprocPipeline() {
        RFCT_PROFILE_FUNCTION();
        renderer::getRen().getDevice().destroyPipelineLayout(m_pipelineLayout);
        renderer::getRen().getDevice().destroyDescriptorSetLayout(m_descSetLayout);
    }
}