#include "DotEngine.h"
#include "DotException.h"

#include <iostream>

int main()
{
    Window wnd;
    Dot::Engine engine(wnd);

    try
    {
        engine.run();
    }
    catch(const Dot::RuntimeError& e)
    {
        std::cerr << e.what() << '\n';
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