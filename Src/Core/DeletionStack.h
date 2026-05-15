#pragma once

#include <functional>
#include <stack>

class DeletionStack
{
private:
    std::stack<std::function<void()>> _stack;
    
public:
    DeletionStack() = default;

    void Push(std::function<void()> destroyFunction);

    void Flush();
};
