#include "NodesResourceManager.h"

#include <queue>
#include <unordered_map>

#include "DescriptionNodeInstance.h"
#include "Logger/Logger.h"

namespace NodesResourceManager
{
    DescriptionNodeInstanceHandle _descriptionNodeInstanceNextID = 1; 
    std::unordered_map<DescriptionNodeInstanceHandle, DescriptionNodeInstance> _descriptionNodesInstances;
    std::queue<DescriptionNodeInstanceHandle> _descriptionNodeInstancesFreeIDs;
}

DescriptionNodeInstanceHandle NodesResourceManager::RegisterDescriptionNodeInstanceHandle(DescriptionNodeInstance& descriptionNodeInstance)
{
    DescriptionNodeInstanceHandle descriptionNodeInstanceHandle;
    if (!_descriptionNodeInstancesFreeIDs.empty())
    {
        descriptionNodeInstanceHandle = _descriptionNodeInstancesFreeIDs.front();
        _descriptionNodeInstancesFreeIDs.pop();
    }
    else
    {
        descriptionNodeInstanceHandle = _descriptionNodeInstanceNextID;
        _descriptionNodeInstanceNextID++;
    }
    
    auto suceess = _descriptionNodesInstances.emplace(descriptionNodeInstanceHandle, descriptionNodeInstance);

    if (!suceess.second)
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("NodesResourceManager::RegisterDescriptionNodeInstance: The descriptionNode with nodeInfoHandle {}, failed to be inserted to the map", descriptionNodeInstance.nodeInfoHandle));
        return 0;
    }
    
    return descriptionNodeInstanceHandle;
}

DescriptionNodeInstance* NodesResourceManager::GetDescriptionNodeInstance(const DescriptionNodeInstanceHandle& descriptionNodeInstanceHandle)
{
    if (!AssetsUtils::IsValid(descriptionNodeInstanceHandle))
    {
        Logger::Log(Logger::LogLevel::Error, "NodesResourceManager::GetDescriptionNodeInstance: Description node instance handle is invalid");
        return nullptr;
    }

    if (!_descriptionNodesInstances.contains(descriptionNodeInstanceHandle))
    {
        Logger::Log(Logger::LogLevel::Error, std::format("NodesResourceManager::GetDescriptionNodeInstance: description node instance with handle = {}; not found", descriptionNodeInstanceHandle));
        return nullptr;
    }

    return &_descriptionNodesInstances[descriptionNodeInstanceHandle];
}

void NodesResourceManager::DestroyDescriptionNodeInstance(const DescriptionNodeInstanceHandle& descriptionNodeInstanceHandle)
{
    if (!AssetsUtils::IsValid(descriptionNodeInstanceHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, "NodesResourceManager::DestroyDescriptionNodeInstance: Description Node Instance Handle Not Valid");
        return;
    }
    
    if (!_descriptionNodesInstances.contains(descriptionNodeInstanceHandle))
    {
        Logger::Log(Logger::LogLevel::Warning, std::format("NodesResourceManager::DestroyDescriptionNodeInstance: Key not found: {}", descriptionNodeInstanceHandle));
        return;
    }
    
    _descriptionNodesInstances.erase(descriptionNodeInstanceHandle);
    _descriptionNodeInstancesFreeIDs.push(descriptionNodeInstanceHandle);
}
