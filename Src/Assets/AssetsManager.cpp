#include "AssetsManager.h"

#include <queue>

#include "Graph/RGNode.h"
#include "Graph/RGNodeInfo.h"
#include "Logger/Logger.h"
#include "Utils/AssetsUtils.h"

namespace AssetsManager
{
    NodeInfoHandle _nodeInfoNextID = 1;
    std::unordered_map<NodeInfoHandle, RGNodeInfo> _nodesInfos;
    std::queue<NodeInfoHandle> _nodesInfosFreeIDs;
}

NodeInfoHandle AssetsManager::RegisterNodeInfo(RGNodeInfo& nodeInfo)
{
    NodeInfoHandle nodeInfoHandle;
    if (!_nodesInfosFreeIDs.empty())
    {
        nodeInfoHandle = _nodesInfosFreeIDs.front();
        _nodesInfosFreeIDs.pop();
    }
    else
    {
        nodeInfoHandle = _nodeInfoNextID;
        _nodeInfoNextID++;
    }
    
    auto suceess = _nodesInfos.emplace(nodeInfoHandle, nodeInfo);

    if (!suceess.second)
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("AssetsManager::RegisterNodeInfo: The nodeInfo of name {}, failed to be inserted to the map", nodeInfo.GetName()));
        return 0;
    }
    
    return nodeInfoHandle;
}

RGNodeInfo* AssetsManager::GetNodeInfo(const NodeInfoHandle& nodeInfoHandle)
{
    if (!AssetsUtils::IsValid(nodeInfoHandle))
    {
        Logger::Log(Logger::LogLevel::Error, "AssetsManager::GetNodeInfo: Node info handle is invalid");
        return nullptr;
    }

    if (!_nodesInfos.contains(nodeInfoHandle))
    {
        Logger::Log(Logger::LogLevel::Error, std::format("AssetsManager::GetNodeInfo: Node info with handle = {}; not found", nodeInfoHandle));
        return nullptr;
    }

    return &_nodesInfos[nodeInfoHandle];
}

void AssetsManager::DestroyNodeInfo(const NodeInfoHandle& nodeInfoHandle)
{
    if (!AssetsUtils::IsValid(nodeInfoHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, "AssetsManager::DestroyNodeInfo: Node info Handle Not Valid");
        return;
    }
    
    if (!_nodesInfos.contains(nodeInfoHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("AssetsManager::DestroyNodeInfo: Key not found: {}", nodeInfoHandle));
        return;
    }
    
    _nodesInfos.erase(nodeInfoHandle);
    _nodesInfosFreeIDs.push(nodeInfoHandle);
}

void AssetsManager::ClearAll()
{
    ClearNodesInfos();
}

void AssetsManager::ClearNodesInfos()
{
    _nodesInfos.clear();
    size_t s = _nodesInfosFreeIDs.size();
    for (size_t i = 0; i < s; ++i)
    {
        _nodesInfosFreeIDs.pop();
    }
    _nodeInfoNextID = 1;
}
