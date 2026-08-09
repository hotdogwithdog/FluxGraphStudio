#pragma once
#include "NodesResourceManager.h"
#include "RGNodeInfo.h"


struct CompiledNode
{
    DescriptionNodeInstanceHandle nodeInstanceHandle;

    std::vector<Graph::ConnectionInfo> inputs;
    std::vector<Graph::ConnectionInfo> outputs;
};

using CompiledGraph = std::vector<CompiledNode>;