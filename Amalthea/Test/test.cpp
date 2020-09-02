#include "Io/Source/Io.h"
#include "Io/Source/IoEntryFunc.h"
#include "Europa/Source/EuropaVk.h"
#include "Amalthea/Source/Amalthea.h"
#include "Ganymede/Source/Ganymede.h"
#include "Himalia/Source/Himalia.h"

#include "unlit.frag.h"
#include "unlit.vert.h"

#include <thread>
#include <chrono>

#include <glm/gtx/transform.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
};

struct ShaderConstants {
	glm::mat4 modelMtx;
	glm::mat4 projViewMtx;
};

std::vector<Vertex> vertices;
std::vector<uint16> indices;

class TestApp : public Amalthea
{
public:
	EuropaBuffer::Ref m_vertexBuffer;
	EuropaBuffer::Ref m_indexBuffer;

	EuropaImage::Ref m_depthImage;
	EuropaImageView::Ref m_depthView;

	EuropaRenderPass::Ref m_mainRenderPass;

	EuropaDescriptorPool::Ref m_descPool;
	EuropaGraphicsPipeline::Ref m_pipeline;
	EuropaPipelineLayout::Ref m_pipelineLayout;

	std::vector<EuropaDescriptorSet::Ref> m_descSets;
	std::vector<EuropaFramebuffer::Ref> m_frameBuffers;

	uint32 m_constantsSize;

	void OnDeviceCreated()
	{
		// Load Model
		HimaliaPlyModel plyModel;
		plyModel.LoadFile("bun_zipper_res2.ply");

		HimaliaVertexProperty vertexFormat[] = {
			HimaliaVertexProperty::Position,
			HimaliaVertexProperty::Color,
			HimaliaVertexProperty::Normal
		};
		plyModel.mesh.BuildVertices<Vertex>(vertices, 3, vertexFormat);
		plyModel.mesh.BuildIndices<uint16>(indices);

		// Create & Upload geometry buffers
		EuropaBufferInfo vertexBufferInfo;
		vertexBufferInfo.exclusive = true;
		vertexBufferInfo.size = uint32(vertices.size() * sizeof(Vertex));
		vertexBufferInfo.usage = EuropaBufferUsage(EuropaBufferUsageVertex | EuropaBufferUsageTransferDst);
		vertexBufferInfo.memoryUsage = EuropaMemoryUsage::GpuOnly;
		m_vertexBuffer = m_device->CreateBuffer(vertexBufferInfo);

		m_transferUtil->UploadToBufferEx(m_vertexBuffer, vertices.data(), uint32(vertices.size()));

		EuropaBufferInfo indexBufferInfo;
		indexBufferInfo.exclusive = true;
		indexBufferInfo.size = uint32(indices.size() * sizeof(uint32));
		indexBufferInfo.usage = EuropaBufferUsage(EuropaBufferUsageIndex | EuropaBufferUsageTransferDst);
		indexBufferInfo.memoryUsage = EuropaMemoryUsage::GpuOnly;
		m_indexBuffer = m_device->CreateBuffer(indexBufferInfo);

		m_transferUtil->UploadToBufferEx(m_indexBuffer, indices.data(), uint32(indices.size()));
	}

	void OnDeviceDestroy()
	{
	}

	void OnSwapChainCreated()
	{
		// Create Depth buffer
		EuropaImageInfo depthInfo;
		depthInfo.width = m_windowSize.x;
		depthInfo.height = m_windowSize.y;
		depthInfo.initialLayout = EuropaImageLayout::Undefined;
		depthInfo.type = EuropaImageType::Image2D;
		depthInfo.format = EuropaImageFormat::D16Unorm;
		depthInfo.usage = EuropaImageUsageDepthStencilAttachment;
		depthInfo.memoryUsage = EuropaMemoryUsage::GpuOnly;

		m_depthImage = m_device->CreateImage(depthInfo);

		EuropaImageViewCreateInfo depthViewInfo;
		depthViewInfo.format = EuropaImageFormat::D16Unorm;
		depthViewInfo.image = m_depthImage;
		depthViewInfo.type = EuropaImageViewType::View2D;
		depthViewInfo.minArrayLayer = 0;
		depthViewInfo.minMipLevel = 0;
		depthViewInfo.numArrayLayers = 1;
		depthViewInfo.numMipLevels = 1;

		m_depthView = m_device->CreateImageView(depthViewInfo);

		// Create Renderpass
		m_mainRenderPass = m_device->CreateRenderPassBuilder();
		uint32 presentTarget = m_mainRenderPass->AddAttachment(EuropaAttachmentInfo{
			EuropaImageFormat::BGRA8sRGB,
			EuropaAttachmentLoadOp::Clear,
			EuropaAttachmentStoreOp::Store,
			EuropaAttachmentLoadOp::DontCare,
			EuropaAttachmentStoreOp::DontCare,
			EuropaImageLayout::Undefined,
			EuropaImageLayout::Present
			});
		uint32 depthTarget = m_mainRenderPass->AddAttachment(EuropaAttachmentInfo{
			EuropaImageFormat::D16Unorm,
			EuropaAttachmentLoadOp::Clear,
			EuropaAttachmentStoreOp::Store,
			EuropaAttachmentLoadOp::DontCare,
			EuropaAttachmentStoreOp::DontCare,
			EuropaImageLayout::Undefined,
			EuropaImageLayout::DepthStencilAttachment
			});
		std::vector<EuropaAttachmentReference> attachments = { { presentTarget, EuropaImageLayout::ColorAttachment } };
		EuropaAttachmentReference depthAttachment = { depthTarget, EuropaImageLayout::DepthStencilAttachment };
		uint32 forwardPass = m_mainRenderPass->AddSubpass(EuropaPipelineBindPoint::Graphics, attachments, &depthAttachment);
		m_mainRenderPass->AddDependency(EuropaRenderPass::SubpassExternal, forwardPass, EuropaPipelineStageColorAttachmentOutput, EuropaAccessNone, EuropaPipelineStageColorAttachmentOutput, EuropaAccessColorAttachmentWrite);
		m_mainRenderPass->CreateRenderpass();

		// Create Pipeline
		EuropaVertexInputBindingInfo binding;
		binding.binding = 0;
		binding.stride = sizeof(Vertex);
		binding.perInstance = false;

		EuropaVertexAttributeBindingInfo attributes[3];
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].offset = offsetof(Vertex, Vertex::pos);
		attributes[0].format = EuropaImageFormat::RGB32F;

		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].offset = offsetof(Vertex, Vertex::color);
		attributes[1].format = EuropaImageFormat::RGB32F;

		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].offset = offsetof(Vertex, Vertex::normal);
		attributes[2].format = EuropaImageFormat::RGB32F;

		EuropaShaderModule::Ref shaderFragment = m_device->CreateShaderModule(shader_spv_unlit_frag, sizeof(shader_spv_unlit_frag));
		EuropaShaderModule::Ref shaderVertex = m_device->CreateShaderModule(shader_spv_unlit_vert, sizeof(shader_spv_unlit_vert));

		EuropaDescriptorSetLayout::Ref descLayout = m_device->CreateDescriptorSetLayout();
		descLayout->UniformBuffer(0, 1, EuropaShaderStageAllGraphics);
		descLayout->Build();

		m_pipelineLayout = m_device->CreatePipelineLayout(EuropaPipelineLayoutInfo{ 1, 0, &descLayout });

		EuropaGraphicsPipelineCreateInfo pipelineDesc{};

		EuropaShaderStageInfo stages[2] = {
			EuropaShaderStageInfo{ EuropaShaderStageFragment, shaderFragment, "main" },
			EuropaShaderStageInfo{ EuropaShaderStageVertex, shaderVertex, "main"}
		};

		pipelineDesc.shaderStageCount = 2;
		pipelineDesc.stages = stages;
		pipelineDesc.vertexInput.vertexBindingCount = 1;
		pipelineDesc.vertexInput.vertexBindings = &binding;
		pipelineDesc.vertexInput.attributeBindingCount = 3;
		pipelineDesc.vertexInput.attributeBindings = attributes;
		pipelineDesc.viewport.position = glm::vec2(0.0);
		pipelineDesc.viewport.size = m_windowSize;
		pipelineDesc.viewport.minDepth = 0.0f;
		pipelineDesc.viewport.maxDepth = 1.0f;
		pipelineDesc.scissor.position = glm::vec2(0.0);
		pipelineDesc.scissor.size = m_windowSize;
		pipelineDesc.depthStencil.enableDepthTest = true;
		pipelineDesc.depthStencil.enableDepthWrite = true;
		pipelineDesc.layout = m_pipelineLayout;
		pipelineDesc.renderpass = m_mainRenderPass;
		pipelineDesc.targetSubpass = 0;

		m_pipeline = m_device->CreateGraphicsPipeline(pipelineDesc);

		// Create Framebuffers
		for (AmaltheaFrame& ctx : m_frames)
		{
			EuropaFramebufferCreateInfo desc;
			desc.attachments = { ctx.imageView, m_depthView };
			desc.width = m_windowSize.x;
			desc.height = m_windowSize.y;
			desc.layers = 1;
			desc.renderpass = m_mainRenderPass;

			EuropaFramebuffer::Ref framebuffer = m_device->CreateFramebuffer(desc);

			m_frameBuffers.push_back(framebuffer);
		}

		// Constants & Descriptor Pools / Sets
		EuropaDescriptorPoolSizes descPoolSizes;
		descPoolSizes.Uniform = uint32(m_frames.size());

		m_descPool = m_device->CreateDescriptorPool(descPoolSizes, uint32(m_frames.size()));

		for (uint32 i = 0; i < m_frames.size(); i++)
		{
			m_descSets.push_back(m_descPool->AllocateDescriptorSet(descLayout));
		}

		m_constantsSize = alignUp(uint32(sizeof(ShaderConstants)), m_device->GetMinUniformBufferOffsetAlignment());
	}

	void OnSwapChainDestroy()
	{
		m_frameBuffers.clear();
		m_descSets.clear();
	}

	void RenderFrame(AmaltheaFrame& ctx, float time)
	{
		auto constantsHandle = m_streamingBuffer->AllocateTransient(m_constantsSize);
		ShaderConstants* constants = constantsHandle.Map<ShaderConstants>();
		
		constants->modelMtx = glm::scale(glm::rotate(time, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(3.0, 3.0, 3.0));

		glm::mat4 viewMtx = glm::lookAt(glm::vec3(0.0, sin(time * 0.3), -1.0), glm::vec3(0.0, 0.3, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 projMtx = glm::perspective(glm::radians(60.0f), float(m_windowSize.x) / (m_windowSize.y), 0.01f, 256.0f);

		projMtx[1].y = -projMtx[1].y;

		constants->projViewMtx = projMtx * viewMtx;

		constantsHandle.Unmap();

		m_descSets[ctx.frameIndex]->SetUniformBuffer(constantsHandle.buffer, constantsHandle.offset, m_constantsSize, 0, 0);

		EuropaClearValue clearValue[2];
		clearValue[0].color = glm::vec4(0.0, 0.0, 0.0, 1.0);
		clearValue[1].depthStencil = glm::vec2(1.0, 0.0);

		ctx.cmdlist->Begin();
		ctx.cmdlist->BeginRenderpass(m_mainRenderPass, m_frameBuffers[ctx.frameIndex], glm::ivec2(0), glm::uvec2(m_windowSize), 2, clearValue);
		ctx.cmdlist->BindPipeline(m_pipeline);
		ctx.cmdlist->BindVertexBuffer(m_vertexBuffer, 0, 0);
		ctx.cmdlist->BindIndexBuffer(m_indexBuffer, 0, EuropaImageFormat::R16UI);
		ctx.cmdlist->BindDescriptorSets(EuropaPipelineBindPoint::Graphics, m_pipelineLayout, m_descSets[ctx.frameIndex]);
		ctx.cmdlist->DrawIndexed(indices.size(), 1, 0, 0, 0);
		ctx.cmdlist->EndRenderpass();
		ctx.cmdlist->End();
	}

	~TestApp()
	{
	}

	TestApp(Europa& e, IoSurface::Ref s) : Amalthea(e, s) {};
};

int AppMain(IoSurface::Ref s)
{
	// Create Europa Instance
	Europa& europa = EuropaVk();

	TestApp app(europa, s);

	app.Run();

	return 0;
}