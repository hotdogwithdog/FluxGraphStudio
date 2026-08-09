#pragma once
#include <unordered_map>

#include "CompiledNode.h"
#include "RGNodeInfo.h"


class DescriptionGraph
{
public:
    
    
public:
    DescriptionGraph() = default;

    DescriptionNodeInstanceHandle AddNodeInstance(DescriptionNodeInstance& descriptionNodeInstance);

    void ConnectNodes(DescriptionNodeInstanceHandle origin, int originIndex, DescriptionNodeInstanceHandle destination, int destinationIndex);
    void MarkNodeAsStart(DescriptionNodeInstanceHandle nodeToMark, int destinationIndexToMark);
    void MarkNodeAsEnd(DescriptionNodeInstanceHandle nodeToMark, int originIndexToMark);

    inline bool IsDirty() const { return _bIsDirty; }

    /// Return the cached compiled graph if valid and if not just do a topological sort of it and return it
    CompiledGraph CompileGraph();

    
private:
    bool _bIsDirty = true;

    CompiledGraph _compiledGraph;
    std::unordered_map<DescriptionNodeInstanceHandle, std::vector<Graph::ConnectionInfo>> _originConnections;
    std::unordered_map<DescriptionNodeInstanceHandle, std::vector<Graph::ConnectionInfo>> _destinationConnections;
    

    std::vector<DescriptionNodeInstanceHandle> _descriptionNodesInstanceHandles;
};
