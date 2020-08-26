#pragma once

#include "Amalthea.h"

class AmaltheaForwardRP : public AmaltheaBehaviors
{
private:
    Amalthea& m_amalthea;

    EuropaRenderPass* m_renderPass;

    EuropaGraphicsPipeline* m_pipeline;
    EuropaGraphicsPipeline* m_pipelineLayout;

    std::vector<EuropaDescriptorSet*> m_descSets;
    std::vector<EuropaFramebuffer*> m_frameBuffers;

public:
    AmaltheaForwardRP(Amalthea& amalthea);
};