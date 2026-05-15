#include "DeletionStack.h"

void DeletionStack::Push(std::function<void()> destroyFunction)
{
    _stack.push(destroyFunction);
}

void DeletionStack::Flush()
{
    while (!_stack.empty())
    {
        _stack.top()();
        _stack.pop();
    }
}
