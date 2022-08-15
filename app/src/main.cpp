#include "dot_Engine.h"
#include "dot_Exception.h"

#include <iostream>

int main()
{
    Window wnd;
    dot::Engine engine(wnd);

    try
    {
        engine.run();
    }
    catch(const dot::RuntimeError& e)
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