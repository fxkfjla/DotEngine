#include "DotEngine.h"

#include <iostream>

int main()
{
    Window wnd;
    DotEngine engine(wnd);

    try
    {
        engine.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(...)
    {
        std::cerr << "Unknown error occurred!" << '\n';
    }

    return 0;
}