#include "App.h"

#include <iostream>

int main()
{
    Window wnd;
    App app(wnd);

    try
    {
        app.run();
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