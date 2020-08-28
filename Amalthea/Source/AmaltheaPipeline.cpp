#include "AmaltheaPipeline.h"

#include <algorithm>

void AmaltheaPipeline::Module::Read(uint32 resourceHandle)
{
    dependencies.push_back(resourceHandle);
}

void AmaltheaPipeline::Module::Write(uint32 resourceHandle)
{
    m_pipeline.resourceDependencies.emplace(resourceHandle, weak_from_this());
}

void AmaltheaPipeline::Module::UseRenderTarget(uint32 resourceHandle)
{
    m_pipeline.resourceDependencies.emplace(resourceHandle, weak_from_this());

    m_renderTargets.push_back(resourceHandle);
}

void AmaltheaPipeline::Module::Compile()
{
    if (m_renderTargets.size() > 0)
    {
        m_renderPass = m_amalthea.m_device->CreateRenderPassBuilder();

        std::vector<EuropaAttachmentReference> attachments;

        for (uint32 attachment : m_renderTargets)
        {
            std::shared_ptr<AmaltheaPipelineImage2D> image = std::static_pointer_cast<AmaltheaPipelineImage2D>(m_pipeline.resourceHandles[attachment].lock());

            uint32 target = m_renderPass->AddAttachment(EuropaAttachmentInfo{
                image->format,
                EuropaAttachmentLoadOp::Clear,
                EuropaAttachmentStoreOp::Store,
                EuropaAttachmentLoadOp::DontCare,
                EuropaAttachmentStoreOp::DontCare,
                EuropaImageLayout::Undefined, // FIXME: Figure out the previous layout through dependencies
                image->layout
                });
            attachments.push_back({ target, EuropaImageLayout::ColorAttachment });

            m_frameSize = image->m_size;
        }

        // FIXME: Combine multiple modules into a single render pass as possible
        uint32 mainPass = m_renderPass->AddSubpass(EuropaPipelineBindPoint::Graphics, attachments);
        m_renderPass->AddDependency(EuropaRenderPass::SubpassExternal, mainPass, EuropaPipelineStageColorAttachmentOutput, EuropaAccessNone, EuropaPipelineStageColorAttachmentOutput, EuropaAccessColorAttachmentWrite);
        m_renderPass->CreateRenderpass();
    }
}

EuropaRenderPass* AmaltheaPipeline::Module::GetRenderPass()
{
    return m_renderPass;
}

void AmaltheaPipeline::Module::Execute(AmaltheaFrame& ctx, float time)
{
    if (m_renderPass)
    {
        std::vector<EuropaImageView*> attachmentViews;

        for (uint32 attachment : m_renderTargets)
        {
            std::shared_ptr<AmaltheaPipelineImage2D> image = std::static_pointer_cast<AmaltheaPipelineImage2D>(m_pipeline.resourceHandles[attachment].lock());

            attachmentViews.push_back(image->m_view);
        }

        EuropaFramebufferCreateInfo desc;
        desc.attachments = attachmentViews;
        desc.width = m_frameSize.x;
        desc.height = m_frameSize.y;
        desc.layers = 1;
        desc.renderpass = m_renderPass;

        m_frameBuffer = m_amalthea.m_device->CreateFramebuffer(desc);

        ctx.cmdlist->BeginRenderpass(m_renderPass, m_frameBuffer, glm::ivec2(0), m_frameSize, 1, glm::vec4(0.0, 0.0, 0.0, 1.0));
        m_execute(*this, ctx, time);
        ctx.cmdlist->EndRenderpass();

        m_amalthea.m_destroyQueue->SafeDestroy(m_frameBuffer);
    }
}

AmaltheaPipeline::Module::Module(AmaltheaPipeline& pipeline, Amalthea& amalthea)
    : m_pipeline(pipeline)
    , m_amalthea(amalthea)
    , enable_shared_from_this()
{
}

uint32 AmaltheaPipeline::GetFreeHandle()
{
    return maxHandle++;
}

void AmaltheaPipeline::Execute(uint32 target, AmaltheaFrame& ctx, float time)
{
    auto range = resourceDependencies.equal_range(target);
    for (auto mIter = range.first; mIter != range.second; mIter++)
    {
        std::shared_ptr<AmaltheaPipeline::Module> m = mIter->second.lock();

        for (uint32 resourceHandle : m->dependencies)
        {
            Execute(resourceHandle, ctx, time);
        }

        m->Execute(ctx, time);
    }
}

uint32 AmaltheaPipeline::ResourceImage2D(std::string name, glm::uvec2 size, uint32 numLayers, EuropaImageFormat format, EuropaImageLayout initialLayout, bool isExternal)
{
    std::shared_ptr<AmaltheaPipelineImage2D> res = std::make_shared<AmaltheaPipelineImage2D>();

    res->isExternal = isExternal;
    res->m_size = size;
    res->m_numLayers = numLayers;
    res->format = format;
    res->layout = initialLayout;

    resources.emplace(name, res);

    uint32 handle = GetFreeHandle();
    resourceHandles.emplace(handle, res);

    return handle;
}

void AmaltheaPipeline::AssignResourceImage2D(uint32 handle, EuropaImage* image, EuropaImageView* view)
{
    shared_ptr<AmaltheaPipelineImage2D> res = std::static_pointer_cast<AmaltheaPipelineImage2D>(resourceHandles[handle].lock());

    res->m_image = image;
    res->m_view = view;
}

std::weak_ptr<AmaltheaPipelineResource> AmaltheaPipeline::GetResource(uint32 handle)
{
    return resourceHandles[handle];
}

void AmaltheaPipeline::SetTarget(uint32 handle)
{
    targetHandle = handle;
}

void AmaltheaPipeline::Compile()
{
    for (auto mIter : modules)
    {
        mIter.second->Compile();
    }
}

void AmaltheaPipeline::Execute(AmaltheaFrame& ctx, float time)
{
    for (auto resourceIter : resources)
    {
        shared_ptr<AmaltheaPipelineResource> resource = resourceIter.second;
        type_index type = resource->GetType();

        if (type == typeid(AmaltheaPipelineImage))
        {
#ifdef _DEBUG
            if (!resource->isExternal)
                throw std::runtime_error("Abstract PipelineImage must be external");

            shared_ptr<AmaltheaPipelineImage> image = std::static_pointer_cast<AmaltheaPipelineImage>(resource);
            if (!image->m_image || !image->m_view)
                throw std::runtime_error("Abstract PipelineImage must have images and views set before execute");
#endif
        }
        else if (type == typeid(AmaltheaPipelineImage2D))
        {
            if (!resource->isExternal)
            {
                shared_ptr<AmaltheaPipelineImage2D> image = std::static_pointer_cast<AmaltheaPipelineImage2D>(resource);

                // FIXME
            }
#ifdef _DEBUG
            else
            {
                shared_ptr<AmaltheaPipelineImage> image = std::static_pointer_cast<AmaltheaPipelineImage>(resource);
                if (!image->m_image || !image->m_view)
                    throw std::runtime_error("Abstract PipelineImage must have images and views set before execute");
            }
#endif
        }
    }

    Execute(targetHandle, ctx, time);
}

AmaltheaPipeline::AmaltheaPipeline(Amalthea& amalthea)
    : m_amalthea(amalthea)
{
}
