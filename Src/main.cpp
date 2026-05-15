#include <iostream>

#include "Core/FGSEngine.h"

int main()
{
    FGSEngine app;
    
    app.Init();
    
    app.Run();
    
    app.CleanUp();

    return 0;
}
