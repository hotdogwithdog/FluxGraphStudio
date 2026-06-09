#include "RenderGraph.h"

#include "GPU/GPUResourceManager.h"

RenderGraph::RenderGraph()
{
    Logger::Log(Logger::LogLevel::Error, "Render Graph created without resourceManager");
    _resourceManager = nullptr;
}

RenderGraph::RenderGraph(GPUResourceManager* resourceManager)
{
    _resourceManager = resourceManager;
    _texturePool = TexturePool(_resourceManager);
}
