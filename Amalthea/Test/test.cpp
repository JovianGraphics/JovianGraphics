#include "Io/Source/Io.h"
#include "Io/Source/IoEntryFunc.h"
#include "Europa/Source/EuropaVk.h"
#include "Amalthea/Source/Amalthea.h"
#include "Amalthea/Source/AmaltheaPipeline.h"
#include "Ganymede/Source/Ganymede.h"

#include "triangle.frag.h"
#include "triangle.vert.h"

#include <thread>
#include <chrono>

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
};

struct ShaderConstants {
	glm::vec4 color;
};

const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
	{{0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16> indices = {
	0, 1, 2, 2, 3, 0
};

class TestApp : public Amalthea
{
public:
	EuropaBuffer* m_uniformBuffer;
	EuropaBuffer* m_vertexBuffer;
	EuropaBuffer* m_indexBuffer;

	EuropaGraphicsPipeline* m_pipeline;
	EuropaPipelineLayout* m_pipelineLayout;

	std::vector<EuropaDescriptorSet*> m_descSets;
	std::vector<EuropaFramebuffer*> m_frameBuffers;

	uint32 m_constantsSize;

	AmaltheaPipeline m_renderGraph;
	uint32 swapChainTarget;

	void OnDeviceCreated()
	{
		// Create Render Graph
		swapChainTarget = m_renderGraph.ResourceImage2D("Swap Chain target", m_swapChainCaps.surfaceCaps.currentExtent, 1, EuropaImageFormat::BGRA8sRGB, EuropaImageLayout::Present, true);

		struct ForwardModuleData
		{
			uint32 frameBuffer;
		};

		auto forwardModule = m_renderGraph.CreateModule<ForwardModuleData>("Forward Geometry");
		forwardModule->UseRenderTarget(swapChainTarget);
		forwardModule->SetExecuteCallback<ForwardModuleData>(
			[&](AmaltheaPipeline::Module m, AmaltheaFrame& ctx, float time)
			{
				auto constantsHandle = m_streamingBuffer->AllocateTransient(m_constantsSize);
				ShaderConstants* constants = constantsHandle.Map<ShaderConstants>();
				constants->color = glm::vec4(sin(time) * 0.5 + 0.5, cos(time) * 0.5 + 0.5, cos(time * 2.0) * 0.5 + 0.5, 1.0);
				constantsHandle.Unmap();

				m_descSets[ctx.frameIndex]->SetUniformBuffer(constantsHandle.buffer, constantsHandle.offset, m_constantsSize, 0, 0);

				ctx.cmdlist->BindPipeline(m_pipeline);
				ctx.cmdlist->BindVertexBuffer(m_vertexBuffer, 0, 0);
				ctx.cmdlist->BindIndexBuffer(m_indexBuffer, 0, EuropaImageFormat::R16UI);
				ctx.cmdlist->BindDescriptorSets(EuropaPipelineBindPoint::Graphics, m_pipelineLayout, m_descSets[ctx.frameIndex]);
				ctx.cmdlist->DrawIndexed(6, 1, 0, 0, 0);
			}
		);

		m_renderGraph.SetTarget(swapChainTarget);
		m_renderGraph.Compile();

		// Create Pipeline
		EuropaVertexInputBindingInfo binding;
		binding.binding = 0;
		binding.stride = sizeof(Vertex);
		binding.perInstance = false;

		EuropaVertexAttributeBindingInfo attributes[2];
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].offset = offsetof(Vertex, Vertex::pos);
		attributes[0].format = EuropaImageFormat::RG32F;

		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].offset = offsetof(Vertex, Vertex::color);
		attributes[1].format = EuropaImageFormat::RGB32F;

		EuropaShaderModule* shaderFragment = m_device->CreateShaderModule(shader_spv_triangle_frag, sizeof(shader_spv_triangle_frag));
		EuropaShaderModule* shaderVertex = m_device->CreateShaderModule(shader_spv_triangle_vert, sizeof(shader_spv_triangle_vert));

		EuropaDescriptorSetLayout* descLayout = m_device->CreateDescriptorSetLayout();
		descLayout->UniformBuffer(0, 1, EuropaShaderStageAllGraphics);
		descLayout->Build();

		EuropaPipelineLayoutInfo layoutInfo;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.descSetLayouts = &descLayout;
		m_pipelineLayout = m_device->CreatePipelineLayout(layoutInfo);

		EuropaGraphicsPipelineCreateInfo pipelineDesc{};

		EuropaShaderStageInfo stages[2] = {
			EuropaShaderStageInfo{ EuropaShaderStageFragment, shaderFragment, "main" },
			EuropaShaderStageInfo{ EuropaShaderStageVertex, shaderVertex, "main"}
		};

		pipelineDesc.shaderStageCount = 2;
		pipelineDesc.stages = stages;
		pipelineDesc.vertexInput.vertexBindingCount = 1;
		pipelineDesc.vertexInput.vertexBindings = &binding;
		pipelineDesc.vertexInput.attributeBindingCount = 2;
		pipelineDesc.vertexInput.attributeBindings = attributes;
		pipelineDesc.viewport.position = glm::vec2(0.0);
		pipelineDesc.viewport.size = m_swapChainCaps.surfaceCaps.currentExtent;
		pipelineDesc.viewport.minDepth = 0.0f;
		pipelineDesc.viewport.maxDepth = 1.0f;
		pipelineDesc.scissor.position = glm::vec2(0.0);
		pipelineDesc.scissor.size = m_swapChainCaps.surfaceCaps.currentExtent;
		pipelineDesc.layout = m_pipelineLayout;
		pipelineDesc.renderpass = forwardModule->GetRenderPass();
		pipelineDesc.targetSubpass = 0;

		m_pipeline = m_device->CreateGraphicsPipeline(pipelineDesc);

		// Constants & Descriptor Pools / Sets
		EuropaDescriptorPoolSizes descPoolSizes;
		descPoolSizes.Uniform = uint32(m_frames.size());

		EuropaDescriptorPool* pool = m_device->CreateDescriptorPool(descPoolSizes, uint32(m_frames.size()));

		for (uint32 i = 0; i < m_frames.size(); i++)
		{
			m_descSets.push_back(pool->AllocateDescriptorSet(descLayout));
		}

		m_constantsSize = alignUp(uint32(sizeof(ShaderConstants)), m_device->GetMinUniformBufferOffsetAlignment());

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

	void RenderFrame(AmaltheaFrame& ctx, float time)
	{
		m_renderGraph.AssignResourceImage2D(swapChainTarget, ctx.image, ctx.imageView);

		ctx.cmdlist->Begin();
		m_renderGraph.Execute(ctx, time);
		ctx.cmdlist->End();
	}

	~TestApp()
	{
		//for (auto fb : m_frameBuffers) GanymedeDelete(fb);
		GanymedeDelete(m_vertexBuffer);
		GanymedeDelete(m_indexBuffer);
		GanymedeDelete(m_uniformBuffer);
		GanymedeDelete(m_pipeline);
		GanymedeDelete(m_pipelineLayout);
	}

	TestApp(Europa& e, IoSurface& s)
		: Amalthea(e, s)
		, m_renderGraph(*this)
	{
	}
};

int AppMain(IoSurface& s)
{
	// Create Europa Instance
	Europa& europa = EuropaVk();

	TestApp app(europa, s);

	app.Run();

	return 0;
}