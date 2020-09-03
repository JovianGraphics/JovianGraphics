#pragma once

#include "Europa.h"
#include <Io/Source/Io.h>
#include <External/imgui/imgui.h>

class EuropaImGui
{
private:
	EuropaDevice::Ref m_device = nullptr;
	EuropaQueue::Ref m_queue = nullptr;
	EuropaDescriptorPool::Ref m_descPool = nullptr;
	EuropaRenderPass::Ref m_renderPass = nullptr;
	std::vector<EuropaFramebuffer::Ref> m_frameBuffers;

	glm::uvec2 m_viewportSize;

public:
	void Init();
	void Render(uint32 frameId, EuropaCmdlist::Ref cmdlist);
	void NewFrame();

	void RebuildSwapChain(uint32 imageCount, glm::uvec2 imageSize, EuropaImageView::Ref* views, EuropaCmdlist::Ref cmdlist);

	EuropaImGui(Europa& europa, EuropaImageFormat imageFormat, EuropaDevice::Ref device, EuropaQueue::Ref queue, EuropaSurface::Ref surface, IoSurface::Ref ioSurface);
	~EuropaImGui();
};