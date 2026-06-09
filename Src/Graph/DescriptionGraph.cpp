#include "DescriptionGraph.h"

#include <queue>
#include <set>

#include "DescriptionNodeInstance.h"
#include "Logger/Logger.h"

DescriptionNodeInstanceHandle DescriptionGraph::AddNodeInstance(DescriptionNodeInstance& descriptionNodeInstance)
{
    _descriptionNodesInstanceHandles.push_back(NodesResourceManager::RegisterDescriptionNodeInstanceHandle(descriptionNodeInstance));
    _originConnections.emplace(_descriptionNodesInstanceHandles.back(), std::vector<Graph::ConnectionInfo>());
    _destinationConnections.emplace(_descriptionNodesInstanceHandles.back(), std::vector<Graph::ConnectionInfo>());
    return _descriptionNodesInstanceHandles.back();
}

void DescriptionGraph::ConnectNodes(DescriptionNodeInstanceHandle origin, int originIndex, DescriptionNodeInstanceHandle destination, int destinationIndex)
{
    DescriptionNodeInstance* originDescriptionNodeInstance = NodesResourceManager::GetDescriptionNodeInstance(origin);
    if (originDescriptionNodeInstance == nullptr) return;
    RGNodeInfo* originNodeInfo = AssetsManager::GetNodeInfo(originDescriptionNodeInstance->nodeInfoHandle);
    if (originNodeInfo == nullptr) return;
    Graph::EImageFormat originFormat = Graph::EImageFormat::None;
    if (originIndex < originNodeInfo->outputs.size() && originIndex >= 0)
    {
        originFormat = originNodeInfo->outputs[originIndex].format;
    }

    DescriptionNodeInstance* destinationDescriptionNodeInstance = NodesResourceManager::GetDescriptionNodeInstance(destination);
    if (destinationDescriptionNodeInstance == nullptr) return;
    RGNodeInfo* destinationNodeInfo = AssetsManager::GetNodeInfo(destinationDescriptionNodeInstance->nodeInfoHandle);
    if (destinationNodeInfo == nullptr) return;
    Graph::EImageFormat destinationFormat = Graph::EImageFormat::None;
    if (destinationIndex < destinationNodeInfo->inputs.size() && destinationIndex >= 0)
    {
        destinationFormat = destinationNodeInfo->inputs[destinationIndex].format;
    }

    if (originFormat == Graph::EImageFormat::None || destinationFormat == Graph::EImageFormat::None
        || originFormat != destinationFormat)
    {
        Logger::Log(Logger::LogLevel::Error, std::format("DescriptionGraph::ConnectNodes: The Formats are invalid: origin -> {}; destination -> {}",
            Graph::CastImageFormatToString(originFormat), Graph::CastImageFormatToString(destinationFormat)));
        return;
    }

    Graph::ConnectionInfo connectionInfo;
    connectionInfo.originNodeInstance = origin;
    connectionInfo.originIndex = originIndex;
    connectionInfo.destinationNodeInstance = destination;
    connectionInfo.destinationIndex = destinationIndex;

    _originConnections[origin].push_back(connectionInfo);
    _destinationConnections[destination].push_back(connectionInfo);
    _bIsDirty = true;
}

void DescriptionGraph::MarkNodeAsStart(DescriptionNodeInstanceHandle nodeToMark, int destinationIndexToMark)
{
    DescriptionNodeInstance* nodeToMarkInstance = NodesResourceManager::GetDescriptionNodeInstance(nodeToMark);
    if (nodeToMarkInstance == nullptr) return;
    RGNodeInfo* nodeToMarkInfo = AssetsManager::GetNodeInfo(nodeToMarkInstance->nodeInfoHandle);
    if (nodeToMarkInfo == nullptr) return;

    if (destinationIndexToMark >= nodeToMarkInfo->inputs.size() && destinationIndexToMark < 0)
    {
        Logger::Log(Logger::LogLevel::Error, std::format("DescriptionGraph::MarkNodeAsStart: The destination index is invalid: {}", destinationIndexToMark));
        return;
    }

    Graph::ConnectionInfo connectionInfo;
    connectionInfo.bIsStart = true;
    connectionInfo.destinationNodeInstance = nodeToMark;
    connectionInfo.destinationIndex = destinationIndexToMark;

    _destinationConnections[nodeToMark].push_back(connectionInfo);
    _bIsDirty = true;
}

std::vector<DescriptionNodeInstanceHandle> DescriptionGraph::CompileGraph()
{
    if (!_bIsDirty) // Must construct this class with the dirty flag on
    {
        return _compiledGraph;
    }

    std::unordered_map<DescriptionNodeInstanceHandle, int> numberOfInputConnections;
    std::queue<DescriptionNodeInstanceHandle> starterNodes;
    std::set<DescriptionNodeInstanceHandle> visitedNodes;

    _compiledGraph.clear();
    _compiledGraph.reserve(_descriptionNodesInstanceHandles.size()); // Not necessary the fills the full capacity but is more efficient anyway

    for (DescriptionNodeInstanceHandle nodeHandle : _descriptionNodesInstanceHandles)
    {
        int nConnections = 0;
        for (const Graph::ConnectionInfo& connectionInfo : _destinationConnections[nodeHandle])
        {
            if (!connectionInfo.bIsStart) nConnections++;
        }
        numberOfInputConnections[nodeHandle] = nConnections;
        if (nConnections == 0 && !visitedNodes.contains(nodeHandle))
        {
            visitedNodes.insert(nodeHandle);
            starterNodes.push(nodeHandle);
        }
    }

    while (starterNodes.size() > 0)
    {
        DescriptionNodeInstanceHandle currentNode = starterNodes.front();
        starterNodes.pop();
        _compiledGraph.push_back(currentNode);

        for (const Graph::ConnectionInfo& connectionInfo : _originConnections[currentNode])
        {
            if (!connectionInfo.bIsEnd) numberOfInputConnections[connectionInfo.destinationNodeInstance]--;
            if (numberOfInputConnections[connectionInfo.destinationNodeInstance] == 0 &&
                !visitedNodes.contains(connectionInfo.destinationNodeInstance))
            {
                visitedNodes.insert(connectionInfo.destinationNodeInstance);
                starterNodes.push(connectionInfo.destinationNodeInstance);
            }
        }
    }

    _bIsDirty = false;
    return _compiledGraph;
}
