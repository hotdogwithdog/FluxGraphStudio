#include "StringUtils.h"

std::string StringUtils::Trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    
    return str.substr(first, last - first + 1);
}

void StringUtils::RemoveUTF8BOM(std::string& str)
{
    if (str.size() >= 3 && (unsigned char)str[0] == 0xEF && (unsigned char)str[1] == 0xBB && (unsigned char)str[2] == 0xBF)
    {
        str.erase(0, 3);
    }
}
