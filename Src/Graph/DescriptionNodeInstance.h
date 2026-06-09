#pragma once

#include <string>
#include <vector>
#include "Graph/RGNodeInfo.h"

struct ShaderParameterValue
{
    Graph::ShaderParameter description;
    void* data;
};

struct DescriptionNodeInstance
{
    NodeInfoHandle nodeInfoHandle;

    std::vector<std::string> imagesPaths;
    std::vector<ShaderParameterValue> uniformsValues;

    DescriptionNodeInstance() = default; // just for the initialization on compile time of the unordered_map on the NodeResourceManager not used never

    DescriptionNodeInstance(const DescriptionNodeInstance& other)
    {
        nodeInfoHandle = other.nodeInfoHandle;
        imagesPaths.reserve(other.imagesPaths.size());
        for (std::string path : other.imagesPaths)
        {
            imagesPaths.push_back(path);
        }
        uniformsValues.reserve(other.uniformsValues.size());
        for (ShaderParameterValue value : other.uniformsValues)
        {
            uniformsValues.push_back(value);
        }
    }
};
