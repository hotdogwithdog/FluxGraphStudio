#pragma once
#include <string>

namespace StringUtils
{
    std::string Trim(const std::string& str);

    void RemoveUTF8BOM(std::string& str);
}
