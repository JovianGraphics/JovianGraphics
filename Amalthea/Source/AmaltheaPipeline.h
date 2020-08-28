#pragma once

#include "Amalthea.h"

#include <functional>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>

using std::type_index;
using std::shared_ptr;
using std::function;

class AmaltheaPipelineResource
{
public:
    bool isExternal = false;

    virtual type_index GetType() = 0;
    ~AmaltheaPipelineResource() {};
};

class AmaltheaPipelineImage : public AmaltheaPipelineResource
{
public:
    EuropaImage* m_image = nullptr;
    EuropaImageView* m_view = nullptr;

    EuropaImageFormat format;
    EuropaImageLayout layout;

    type_index GetType() { return typeid(AmaltheaPipelineImage); }
    ~AmaltheaPipelineImage() {};
};

class AmaltheaPipelineImage2D : public AmaltheaPipelineImage
{
public:
    glm::uvec2 m_size;
    uint32 m_numLayers;

    type_index GetType() { return typeid(AmaltheaPipelineImage2D); }
    ~AmaltheaPipelineImage2D() {};
};

class AmaltheaPipeline
{
public:
    // Graph building API
    class Module : public std::enable_shared_from_this<Module>
    {
    private:
        void* m_parameters;
        AmaltheaPipeline& m_pipeline;
        Amalthea& m_amalthea;

        EuropaRenderPass* m_renderPass = nullptr;
        glm::uvec2 m_frameSize;
        std::vector<uint32> m_renderTargets;

        // Transient
        EuropaFramebuffer* m_frameBuffer = nullptr;

    public:
        function<void(Module&, AmaltheaFrame&, float)> m_execute;
        std::vector<uint32> dependencies;

        void Read(uint32 resourceHandle);
        void Write(uint32 resourceHandle);
        void UseRenderTarget(uint32 resourceHandle);
        template <class T> void SetExecuteCallback(function<void(Module&, AmaltheaFrame&, float)> callback) {
            if (!m_parameters) m_parameters = (void*)(new T);
            m_execute = callback;
        }

        void Compile();
        EuropaRenderPass* GetRenderPass();
        void Execute(AmaltheaFrame& ctx, float time);

        Module(AmaltheaPipeline& pipeline, Amalthea& amalthea);
    };

    std::unordered_multimap<uint32, std::weak_ptr<Module>> resourceDependencies;

private:
    std::unordered_map<std::string, shared_ptr<AmaltheaPipelineResource>> resources;
    std::unordered_map<uint32, std::weak_ptr<AmaltheaPipelineResource>> resourceHandles;
    std::unordered_map<std::string, shared_ptr<Module>> modules;

    Amalthea& m_amalthea;

    uint32 maxHandle = 0;
    uint32 targetHandle = 0;
    uint32 GetFreeHandle();

    void Execute(uint32 target, AmaltheaFrame& ctx, float time);

public:
    template <class T> shared_ptr<Module> CreateModule(std::string name)
    {
        auto m = std::make_shared<Module>(*this, m_amalthea);
        modules.emplace(name, m);
        return m;
    }

    uint32 ResourceImage2D(std::string name, glm::uvec2 size, uint32 numLayers, EuropaImageFormat format, EuropaImageLayout initialLayout, bool isExternal = false);
    void AssignResourceImage2D(uint32 handle, EuropaImage* image, EuropaImageView* view);
    uint32 ResourceFrameBuffer();

    std::weak_ptr<AmaltheaPipelineResource> GetResource(uint32 handle);

    // Graph
    void SetTarget(uint32 handle);
    void Compile();
    void Execute(AmaltheaFrame& ctx, float time);

    AmaltheaPipeline(Amalthea& amalthea);
};