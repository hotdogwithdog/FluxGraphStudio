#pragma once

#include <cstdint>

class RGNodeInfo;

using NodeInfoHandle = uint32_t;

namespace AssetsManager
{
    NodeInfoHandle RegisterNodeInfo(RGNodeInfo& nodeInfo);
    RGNodeInfo* GetNodeInfo(const NodeInfoHandle& nodeInfoHandle);
    void DestroyNodeInfo(const NodeInfoHandle& nodeInfoHandle);


    void ClearAll();
    void ClearNodesInfos();
};


